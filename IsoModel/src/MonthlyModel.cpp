/**********************************************************************
 * Copyright (c) 2008-2013, Alliance for Sustainable Energy.
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 **********************************************************************/
#include "MonthlyModel.hpp"
// to run main
#include "UserModel.hpp"
// [Refactor] Include the helpers for vector/matrix math
#include "Constants.hpp"
#include "MathHelpers.hpp"
#include "Profiler.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace openstudio::isomodel {

MonthlyModel::MonthlyModel() = default;
MonthlyModel::~MonthlyModel() = default;

void MonthlyModel::calculateSunHours(const Matrix &m_mhEgh,
                                     Vector &v_hrs_sun_down_mo) const {
  // Find what time the sun comes up and goes down and the fraction of hours sun
  // is up and down.
  for (int i = 0; i < monthsInYear; i++) {
    int sun_up_time = 0;
    int sun_down_time = 0;

    // Searching forwards, the first hour with non-zero Egh is the first
    // daylight hour (sunrise).
    for (int j = 0; j < 24; j++) {
      if (m_mhEgh(i, j) != 0) {
        sun_up_time = j;
        break;
      }
    }

    // Searching backwards, the first hour with non-zero Egh is the last
    // daylight hour (sunset is at the *end* of this hour).
    for (int j = 23; j >= 0; j--) {
      if (m_mhEgh(i, j) != 0) {
        sun_down_time = j;
        break;
      }
    }

    double frac_hrs_sun_up = (sun_down_time - sun_up_time + 1) / 24.0;
    v_hrs_sun_down_mo[i] = (1.0 - frac_hrs_sun_up) * hoursInMonth[i];
  }
}

Vector MonthlyModel::calculateUtilizationFactor(const Vector &gamma_H,
                                                double a_H) const {
  PROFILE_FUNCTION();
  Vector eta_g(monthsInYear);
  for (unsigned int i = 0; i < eta_g.size(); i++) {
    if (gamma_H[i] > 0) {
      double num = std::pow(gamma_H[i], a_H);
      eta_g[i] = (UNITY_FRACTION - num) / (UNITY_FRACTION - num * gamma_H[i]);
    } else {
      eta_g[i] =
          UNITY_FRACTION / (gamma_H[i] + std::numeric_limits<double>::epsilon());
    }
  }
  return eta_g;
}

std::pair<Vector, Vector> MonthlyModel::calculateAirVolumes(
    const Vector &Qneed_ht, const Vector &Qneed_cl, const Vector &Th_avg,
    const Vector &Tc_avg) const {
  PROFILE_FUNCTION();
  // Hot air supply temperature (C).
  double T_sup_ht =
      heating.temperatureSetPointOccupied() + heating.dT_supp_ht();
  // Cool air supply temperature (C).
  double T_sup_cl =
      cooling.temperatureSetPointOccupied() - cooling.dT_supp_cl();

  // Volume of air moved for heating (m3).
  Vector v_Vair_ht =
      div(Qneed_ht, sum(mult(dif(T_sup_ht, Th_avg), rhoCpAir),
                          std::numeric_limits<double>::epsilon()));
  // Volume of air moved for cooling (m3).
  Vector v_Vair_cl =
      div(Qneed_cl, sum(mult(dif(Tc_avg, T_sup_cl), rhoCpAir),
                          std::numeric_limits<double>::epsilon()));

  return {v_Vair_ht, v_Vair_cl};
}

Vector MonthlyModel::calculateTotalAirFlow(const Vector &v_Vair_ht,
                                           const Vector &v_Vair_cl,
                                           double frac_hrs_wk_day) const {
  PROFILE_FUNCTION();
  // Total air flow (m3).
  // Multiply by MEGASECONDS_TO_SECONDS to convert megaseconds to seconds.
  // Divide by LITERS_TO_M3 to convert liters to m3.
  Vector v_Vair_tot = maximum(
      sum(v_Vair_ht, v_Vair_cl),
      div(mult(megasecondsInMonth,
               ventilation.supplyRate() * frac_hrs_wk_day *
                   (MEGASECONDS_TO_SECONDS / LITERS_TO_M3),
               monthsInYear),
          UNITY_FRACTION)); // UNITY_FRACTION is 1.0, just for consistency
  return v_Vair_tot;
}

Vector MonthlyModel::calculateFanEnergy(const Vector &Vair_tot) const {
  PROFILE_FUNCTION();
  // Fan power (MJ)
  // ventilation.fanPower is in W/(L/s) which is J/L, also kJ/m3. Divide by
  // KJ_TO_MJ for MJ/m3 to get fanEnergy in MJ.
  Vector fanEnergy = mult(Vair_tot, ventilation.fanPower() / KJ_TO_MJ);
  return fanEnergy;
}

Matrix MonthlyModel::buildSolarIrradianceMatrix() const {
  PROFILE_FUNCTION();
  // Combine vertical surface radiation (msolar) and horizontal radiation
  // (mEgh) into one matrix (W/m2).
  Matrix m_I_sol(monthsInYear, numTotalSurfaces);

  // Access weather data via reference to avoid copy
  const Matrix &m_solar = location.weather()->msolarRef();
  const Vector &v_mEgh = location.weather()->mEghRef();

  for (unsigned int r = 0; r < m_I_sol.size1(); r++) {
    for (unsigned int c = 0; c < numVerticalSurfaces; c++) { // Vertical surfaces
      m_I_sol(r, c) = m_solar(r, c);
    }
    m_I_sol(r, numVerticalSurfaces) = v_mEgh[r]; // Roof/Horizontal surface
  }
  return m_I_sol;
}

Vector MonthlyModel::calculateGlazingSolarHeatGain(
    const Matrix &m_I_sol, const Vector &v_win_A_sol) const {
  PROFILE_FUNCTION();
  Vector v_win_phi_sol(monthsInYear);
  Vector v_win_SCF_frac(numTotalSurfaces);
  v_win_SCF_frac.assign(numTotalSurfaces, UNITY_FRACTION);

  // Compute the total solar heat gain for the glazing area.
  for (unsigned int i = 0; i < monthsInYear; i++) {
    double monthlySum = 0.0;
    for (unsigned int j = 0; j < numTotalSurfaces; j++) {
      monthlySum += structure.windowShadingCorrectionFactor()[j] *
                    v_win_SCF_frac[j] * v_win_A_sol[j] * m_I_sol(i, j);
    }
    v_win_phi_sol[i] = monthlySum;
  }
  return v_win_phi_sol;
}

Vector MonthlyModel::calculateOpaqueSolarHeatGain(
    const Matrix &m_I_sol, const Vector &v_wall_A_sol,
    const Vector &v_wall_phi_r) const {
  PROFILE_FUNCTION();
  Vector v_wall_phi_sol(monthsInYear);

  // Compute the total solar heat gain for the opaque area.
  for (unsigned int i = 0; i < monthsInYear; i++) {
    double monthlySum = 0.0;
    // Using numTotalSurfaces for iteration as envFormFactors is also sized for it
    for (unsigned int j = 0; j < numTotalSurfaces; j++) {
      monthlySum +=
          v_wall_A_sol[j] * m_I_sol(i, j) - v_wall_phi_r[j] * envFormFactors[j];
    }
    v_wall_phi_sol[i] = monthlySum;
  }
  return v_wall_phi_sol;
}

double MonthlyModel::calculatePeopleHeatGain(bool occupied) const {
  PROFILE_FUNCTION();
  if (occupied) {
    return pop.heatGainPerPerson() / pop.densityOccupied();
  } else {
    return pop.heatGainPerPerson() / pop.densityUnoccupied();
  }
}

double MonthlyModel::calculateApplianceHeatGain(bool occupied) const {
  PROFILE_FUNCTION();
  if (occupied) {
    return building.electricApplianceHeatGainOccupied() +
           building.gasApplianceHeatGainOccupied();
  } else {
    return building.electricApplianceHeatGainUnoccupied() +
           building.gasApplianceHeatGainUnoccupied();
  }
}

double MonthlyModel::calculateIlluminationHeatGain(double Q_illum_val,
                                                  double hours_fraction) const {
  PROFILE_FUNCTION();
  // Q_illum_val is in kWh, structure.floorArea() in m2, hoursInYear in hours.
  // Result should be in W/m2.
  return Q_illum_val / structure.floorArea() / hoursInYear / hours_fraction *
         KWATTS_TO_WATTS;
}

double MonthlyModel::calculateAverageIlluminationHeatGain(
    double Q_illum_tot_yr) const {
  PROFILE_FUNCTION();
  return Q_illum_tot_yr / structure.floorArea() / hoursInYear *
         KWATTS_TO_WATTS;
}

Vector MonthlyModel::calculatePeriodHeatGain(
    double phi_int_period, const Vector &megaseconds_period,
    const Vector &frac_Pgh_period, const Vector &v_E_sol) const {
  PROFILE_FUNCTION();
  // Internal heat gain for the period (MJ).
  Vector v_W_int_period =
      mult(megaseconds_period, phi_int_period * structure.floorArea());
  // Solar heat gain for the period (MJ).
  Vector v_W_sol_period = mult(v_E_sol, frac_Pgh_period);
  // Total heat gain for the period (W).
  return div(sum(v_W_int_period, v_W_sol_period), megaseconds_period);
}

MonthlyModel::PlugLoads
MonthlyModel::calculatePlugLoads(double frac_hrs_wk_day) const {
  PROFILE_FUNCTION();
  PlugLoads result;

  // Average electric plug loads (W/m2).
  double E_plug_elec_avg =
      building.electricApplianceHeatGainOccupied() * frac_hrs_wk_day +
      building.electricApplianceHeatGainUnoccupied() *
          (UNITY_FRACTION - frac_hrs_wk_day);
  // Average gas plug loads (W/m2).
  double E_plug_gas_avg =
      building.gasApplianceHeatGainOccupied() * frac_hrs_wk_day +
      building.gasApplianceHeatGainUnoccupied() *
          (UNITY_FRACTION - frac_hrs_wk_day);

  // Electric plug load (kWh/m2).
  result.v_Q_plug_elec =
      mult(hoursInMonth, E_plug_elec_avg * W2kW, monthsInYear);
  // Gas plug load (kWh/m2).
  result.v_Q_plug_gas =
      mult(hoursInMonth, E_plug_gas_avg * W2kW, monthsInYear);

  return result;
}

Vector MonthlyModel::convertEnergyToKWhPerSqM(const Vector &energy_MJ,
                                              double floor_area) const {
  PROFILE_FUNCTION();
  if (floor_area == 0.0) {
    return Vector(energy_MJ.size(), 0.0); // Avoid division by zero
  }
  // Convert MJ to kWh, then divide by floor area
  return div(div(energy_MJ, floor_area), kWh2MJ);
}


MonthlyModel::WindowShadingComponents
MonthlyModel::calculateWindowShadingComponents() const {
  PROFILE_FUNCTION();
  WindowShadingComponents result;
  result.v_win_ff.resize(numTotalSurfaces);
  Vector v_win_SDF(numTotalSurfaces);
  Vector v_win_SDF_frac(numTotalSurfaces);

  for (int i = 0; i < numTotalSurfaces; i++) {
    result.v_win_ff[i] = UNITY_FRACTION - structure.win_ff();
    // Assign SDF based on pulldown value of 1, 2 or 3.
    v_win_SDF[i] = winSDFTable[((int)structure.windowShadingDevice()[i]) - 1];
    // Set the SDF fractions which include heat transfer - set at 100% for now.
    v_win_SDF_frac[i] = UNITY_FRACTION;
  }
  result.v_win_F_shgl = mult(v_win_SDF, v_win_SDF_frac);
  return result;
}

MonthlyModel::AnnualLightingHours
MonthlyModel::calculateAnnualLightingOperationalHours() const {
  PROFILE_FUNCTION();

  AnnualLightingHours result;

  // Lighting operational hours during the daytime.
  double hoursOccupied = std::min(lights.n_day_end(), pop.hoursEnd()) -
                         std::max(pop.hoursStart(), lights.n_day_start());
  if (hoursOccupied < 0) {
    hoursOccupied += hoursInDay; // Use constant
  }
  double daysOccupied = pop.daysEnd() - pop.daysStart() + 1;
  if (daysOccupied < 0) {
    daysOccupied += daysInWeek; // Use constant
  }
  result.t_lt_D = hoursOccupied * daysOccupied * lights.n_weeks();

  // Lighting operational hours during the nighttime.
  hoursOccupied = std::max(lights.n_day_start() - pop.hoursStart(), 0.0) +
                  std::max(pop.hoursEnd() - lights.n_day_end(), 0.0);
  result.t_lt_N = hoursOccupied * daysOccupied * lights.n_weeks();

  // Unoccupied hours.
  result.t_unocc = hoursInYear - result.t_lt_D - result.t_lt_N;

  return result;
}

/**
 * Breaks down the solar radiation and temperature data into day, night,
 * weekday and weekend vectors, as appropriate.
 */
void MonthlyModel::solarRadiationBreakdown(MonthlySimulationData &simData) const {
  PROFILE_FUNCTION();
  // Copy to a new variables so matrix nature is clear.
  // Optimization: Use references to avoid copying matrices
  const Matrix &m_mhEgh = location.weather()->mhEghRef();
  const Matrix &m_mhdbt = location.weather()->mhdbtRef();

  const auto &scheduleData = simData.scheduleData;

  // Note, these are matrix multiplies (matrix*vector) resulting in a vector.

  // monthly average dry bulb temp (dbt) during the occupied hours of days
  simData.v_Tdbt_day = prod(m_mhdbt, scheduleData.clockHourOccupied);
  simData.v_Tdbt_day = div(simData.v_Tdbt_day, sum(scheduleData.clockHourOccupied));

  // monthly avg dbt during the unoccupied hours of days
  simData.v_Tdbt_nt = prod(m_mhdbt, scheduleData.clockHourUnoccupied);
  simData.v_Tdbt_nt = div(simData.v_Tdbt_nt, sum(scheduleData.clockHourUnoccupied));

  // monthly avg global horiz rad power (Egh)  during the "day" hours
  Vector v_Egh_day = prod(m_mhEgh, scheduleData.clockHourOccupied);
  v_Egh_day = div(v_Egh_day, sum(scheduleData.clockHourOccupied));

  // monthly avg Egh during the "night" hours
  Vector v_Egh_nt = prod(m_mhEgh, scheduleData.clockHourUnoccupied);
  v_Egh_nt = div(v_Egh_nt, sum(scheduleData.clockHourUnoccupied));

  // Monthly avg Egh energy (Wgh) during the week days.
  Vector v_Wgh_wk_day = mult(v_Egh_day, scheduleData.weekdayOccupiedMegaseconds);
  // Monthly avg Wgh during week nights.
  Vector v_Wgh_wk_nt = mult(v_Egh_nt, scheduleData.weekdayUnoccupiedMegaseconds);
  // Monthly avg Wgh during weekend days.
  Vector v_Wgh_wke_day = mult(v_Egh_day, scheduleData.weekendOccupiedMegaseconds);
  // Monthly avg Wgh during weekend nights.
  Vector v_Wgh_wke_nt = mult(v_Egh_nt, scheduleData.weekendUnoccupiedMegaseconds);
  // Egh_avg_total MJ/m2.
  Vector v_Wgh_tot =
      sum(sum(v_Wgh_wk_day, v_Wgh_wk_nt), sum(v_Wgh_wke_day, v_Wgh_wke_nt));

  // frac_Egh_unocc_weekday_night
  simData.frac_Pgh_wk_nt = div(v_Wgh_wk_nt, v_Wgh_tot);
  // frac_Egh_unocc_weekend_day
  simData.frac_Pgh_wke_day = div(v_Wgh_wke_day, v_Wgh_tot);
  // frac_Egh_unocc_weekend_night
  simData.frac_Pgh_wke_nt = div(v_Wgh_wke_nt, v_Wgh_tot);

  calculateSunHours(m_mhEgh, simData.v_hrs_sun_down_mo);
}

/**
 * Compute lighting energy use as per prEN 15193:2006.
 */
void MonthlyModel::lightingEnergyUse(MonthlySimulationData &simData) const {
  PROFILE_FUNCTION();
  double lpd_occ = lights.powerDensityOccupied();
  double lpd_unocc = lights.powerDensityUnoccupied();

  // Daylight sensor dimming fraction.
  double F_D = lights.dimmingFraction();
  // Occupancy sensor control fraction.
  double F_O = building.lightingOccupancySensor();
  // Constant illimance control fraction.
  double F_C = building.constantIllumination();

  // Calculate annual lighting operational hours using the new helper
  AnnualLightingHours annualHours = calculateAnnualLightingOperationalHours();

  // Total lighting energy for occupied times (kWh).
  simData.Q_illum_occ = structure.floorArea() * lpd_occ * F_C * F_O *
                (annualHours.t_lt_D * F_D + annualHours.t_lt_N) *
                W2kW;
  // Total annual lighting energy for unnocupied times (kWh).
  simData.Q_illum_unocc =
      structure.floorArea() * lpd_unocc * annualHours.t_unocc * W2kW;
  // Total annual lighting energy (kWh).
  simData.Q_illum_tot_yr = simData.Q_illum_occ + simData.Q_illum_unocc;

  // Split annual lighting energy into monthly lighting energy via the month
  // fraction of the year (kWh).
  simData.v_Q_illum_tot = mult(monthFractionOfYear, simData.Q_illum_tot_yr, monthsInYear);
  // Total exterior lighting (kWh).
  simData.v_Q_illum_ext_tot =
      mult(simData.v_hrs_sun_down_mo, lights.exteriorEnergy() * W2kW);
}

/**
 * Compute envelope parameters as per ISO 13790 8.3.
 */
void MonthlyModel::envelopCalculations(MonthlySimulationData &simData) const {
  PROFILE_FUNCTION();
  // TODO: Copying the various structure values to new variables (e.g. v_wall_A)
  // is not necessary. BAA@2015-07-13.
  simData.v_wall_A = structure.wallAreaRef();
  simData.v_win_A = structure.windowAreaRef();
  simData.v_wall_U = structure.wallUniformRef();
  const Vector &v_win_U = structure.windowUniformRef();

  // Compute total envelope U*A.
  const Vector v_env_UA = sum(mult(simData.v_wall_A, simData.v_wall_U), mult(simData.v_win_A, v_win_U));

  // Compute direct transmission heat transfer coefficient to exterior in as per
  // ISO 13790 8.3.1 (W/K). Ignore linear and point thermal bridges for now.
  // TODO: Implement thermal bridges. BAA@2015-07-13.
  double H_D = sum(v_env_UA);

  // For now, also ignore heat transfer to ground (minimal in large buildings),
  // unconditioned spaces, and adjacent buildings.
  // TODO: Implement ground, unconditioned, and adjacent above heat transfer
  // coefficients. BAA@2015-07-13.
  double H_g = 0;
  double H_U = 0;
  double H_A = 0;

  // Total transmission heat transfer coefficient. ISO 13790 8.3.1 eq. 17.
  simData.H_tr = H_D + H_g + H_U + H_A;

  simData.v_wall_emiss = structure.wallThermalEmissivityRef();
  simData.v_wall_alpha_sc = structure.wallSolarAbsorptionRef();
}

/*
 * Compute window solar gain per ISO 13790 11.3.
 */
void MonthlyModel::windowSolarGain(MonthlySimulationData &simData) const {
  PROFILE_FUNCTION();
  // TODO: The solar heat gain could be improved
  // better understand SCF and SDF and how they map to F_sh
  // calculate effective sky temp so we can better estimate theta_er and
  // theta_ss, and hr.

  // From ISO 13790 11.3.3 Effective solar collecting area of glazed elements,
  // eqn 44 A_sol = F_sh,gl* g_gl*(1 ? F_f)*A_w,p A_sol = effective solar
  // collecting area of window in m2 F_sh,gl = shading reduction factor for
  // movable shades as per 11.4.3 (v_win_SDF *v_win_SDF_frac) g_gl = total solar
  // energy transmittance of transparent element as per 11.4.2 F_f = Frame area
  // fraction (ratio of projected frame area to overall glazed element area) as
  // per 11.4.5 (v_wind_ff) A_w,p = ovaral projected area of glazed element in
  // m2 (v_wind_A)

  // // Frame factor.
  // Vector v_win_ff = Vector(numTotalSurfaces);

  // Vector v_win_SDF = Vector(numTotalSurfaces);
  // Vector v_win_SDF_frac = Vector(numTotalSurfaces);

  // for (int i = 0; i < numTotalSurfaces; i++) {
  //   v_win_ff[i] = 1.0 - structure.win_ff();
  //   // Assign SDF based on pulldown value of 1, 2 or 3.
  //   // TODO: This needs to be clarified in the .ism file as it's not obvious
  //   // that the window SDF is a magic number rather than the actual value.
  //   // BAA@2015-07-13 BAA@2015-07-143
  //   v_win_SDF[i] = winSDFTable[((int)structure.windowShadingDevice()[i]) - 1];
  //   // Set the SDF fractions which include heat transfer - set at 100% for now.
  //   v_win_SDF_frac[i] = 1.0;
  // }

  // Vector v_win_F_shgl = mult(v_win_SDF, v_win_SDF_frac);


  // Calculate window shading components
  WindowShadingComponents shadingComponents = calculateWindowShadingComponents();
  const Vector &v_win_ff = shadingComponents.v_win_ff;
  const Vector &v_win_F_shgl = shadingComponents.v_win_F_shgl;


  // Normal incidence solar energy transmittance which is SHGC in america.
  // Vector v_g_gln = structure.windowNormalIncidenceSolarEnergyTransmittance();
  const Vector &v_g_gln = structure.windowNormalIncidenceSolarEnergyTransmittance();
  // Solar energy transmittance of glazing as per ISO 13790 11.4.2.
  Vector v_g_gl = mult(v_g_gln, structure.win_F_W());

  simData.v_win_A_sol = mult(mult(mult(v_win_F_shgl, v_g_gl), v_win_ff), simData.v_win_A);

  // // Form factors given in ISO 13790, 11.4.6 as 0.5 for wall, 1.0 for
  // unshaded roof double envFormFactors[] = { 0.5, 0.5, 0.5, 0.5, 0.5, 0.5,
  // 0.5, 0.5, 1 };

  // Vertical wall external convective surface heat resistances (simplified).
  simData.v_wall_R_sc.assign(numTotalSurfaces, structure.R_sc_ext());

  // Window external radiative heat xfer coeff.
  // ISO 13790 11.4.6 says use hr=5 as a first approx.
  simData.v_win_hr = mult(simData.v_wall_emiss, ISO_WIN_EXT_RAD_COEFF);

  simData.v_wall_A_sol =
      mult(mult(mult(simData.v_wall_alpha_sc, simData.v_wall_R_sc), simData.v_wall_U), simData.v_wall_A);
}

/**
 * Calculate solar heat gain. ISO 13790 11.3.2.
 */
void MonthlyModel::solarHeatGain(MonthlySimulationData &simData) const {
  PROFILE_FUNCTION();
  // EN ISO 13790 11.3.2 eq. 43.
  // \Phi_sol,k = F_sh,ob,k * A_sol,k * I_sol,k - F_r,k * \Phi_r,k

  // \Phi_sol,k = solar heat flow gains through building element k
  // F_sh,ob,k = shading reduction factor for external obstacles calculated
  // via 11.4.4 A_sol,k = effective collecting area of surface calculated
  // via 11.3.3 (glazing ) 11.3.4 (opaque) I_sol,k = solar irradiance, mean
  // energy of solar irradiation per square meter calculated using Annex F F_r,k
  // form factor between building and sky determined using 11.4.6 \Phi_r,k extra
  // heat flow from thermal radiation to sky determined using 11.3.5

  // TODO: The solar heat gain could be improved
  // better understand SCF and SDF and how they map to F_SH
  // calculate effective sky temp so we can better estimate theta_er and
  // theta_ss.

  // Build the combined solar irradiance matrix
  const Matrix m_I_sol = buildSolarIrradianceMatrix();

  // Combine vertical surface radiation (mosolar) and horizontal radiation
  // (mEgh) into one matrix (W/m2).
  printMatrix("m_I_sol", m_I_sol);

  // Compute the total solar heat gain for the glazing area.
  Vector v_win_phi_sol = calculateGlazingSolarHeatGain(m_I_sol, simData.v_win_A_sol);

  // Compute opaque area thermal radiation to the sky from EN ISO 13790 11.3.5
  // \Phi_r,k = R_se * U_c  * A_c * h_h * \delta\theta_er (46)
  // \Phi_r,k = thermal radiation to sky in W
  // R_se = external heat resistance as defined above m2K/W
  // U_c = U value of element as defined above W/m2K
  // A_c = area of element  defined above m2
  // \delta\theta_er = is the average difference between the external air
  // temperature and the apparent sky temperature, determined in accordance
  // with 11.4.6, expressed in degrees centigrade.
  Vector theta_er(numTotalSurfaces);
  theta_er.assign(numTotalSurfaces, ISO_SKY_TEMP_DIFF);
  /* for (unsigned int i = 0; i < theta_er.size(); i++) {
    // Average difference between air temperature and sky temperature.
    // ISO 13790 11.4.6 says take \Theta_er=9k in sub polar zones, 13 K in
    // tropical or 11 K in intermediate
    // TODO: Does the .epw file contain the sky temperature? If not, use the
    // weather file's lat/lon to determine which default value to use for
    // theta_er. BAA@2015-07-13.
    theta_er[i] = ISO_SKY_TEMP_DIFF;
  } */

  Vector v_wall_phi_r = mult(
      mult(mult(mult(simData.v_wall_R_sc, simData.v_wall_U), simData.v_wall_A), simData.v_win_hr), theta_er);

  // Total solar heat gain for opaque area.
  Vector v_wall_phi_sol = calculateOpaqueSolarHeatGain(m_I_sol, simData.v_wall_A_sol, v_wall_phi_r);

  printVector("v_wall_phi_r", v_wall_phi_r);
  printVector("v_win_phi_sol", v_win_phi_sol);
  printVector("v_wall_phi_sol", v_wall_phi_sol);

  // Total envelope solar heat gain (W).
  Vector v_phi_sol = sum(v_win_phi_sol, v_wall_phi_sol);
  printVector("v_phi_sol", v_phi_sol);

  // Total envelope solar heat gain (MJ).
  simData.v_E_sol = mult(v_phi_sol, megasecondsInMonth);
}

/**
 * Compute internal heat gains and losses.
 */
void MonthlyModel::heatGainsAndLosses(MonthlySimulationData &simData) const {
  PROFILE_FUNCTION();
  // Internal heat gains from people (W/m2).
  double phi_int_occ = calculatePeopleHeatGain(true);
  double phi_int_unocc = calculatePeopleHeatGain(false);
  simData.phi_int_avg = std::lerp(phi_int_unocc, phi_int_occ, simData.scheduleData.frac_hrs_wk_day);

  // Internal heat gain from appliances (W/m2).
  double phi_plug_occ = calculateApplianceHeatGain(true);
  double phi_plug_unocc = calculateApplianceHeatGain(false);
  simData.phi_plug_avg = std::lerp(phi_plug_unocc, phi_plug_occ, simData.scheduleData.frac_hrs_wk_day);

  // Internal heat gain from illumination (W/m2).
  double phi_illum_occ = calculateIlluminationHeatGain(simData.Q_illum_occ, simData.scheduleData.frac_hrs_wk_day);
  double phi_illum_unocc = calculateIlluminationHeatGain(simData.Q_illum_unocc, (UNITY_FRACTION - simData.scheduleData.frac_hrs_wk_day));
  simData.phi_illum_avg = calculateAverageIlluminationHeatGain(simData.Q_illum_tot_yr);

  // Original spreadsheet computed the approximate internal heat gain for week
  // nights, weekend days, and weekend nights assuming they scale as the occ.
  // fractions.  These are used for finding temp and not for directly
  // calculating energy use total so approximations are more acceptable.
  //
  // The following is a more accuate internal heat gain for week nights,
  // weekend days and weekend nights as it uses the unoccupied values rather
  // than just scaling occupied versions with the occupancy fraction
  // RTM 13-Nov-2012
  double phi_unoccupied_total = (phi_int_unocc + phi_plug_unocc + phi_illum_unocc);
  simData.phi_int_wk_nt = phi_unoccupied_total;
  simData.phi_int_wke_day = phi_unoccupied_total;
  simData.phi_int_wke_nt = phi_unoccupied_total;
}

/**
 * Compute total internal heat gain in W.
 */
void MonthlyModel::internalHeatGain(MonthlySimulationData &simData) const {
  PROFILE_FUNCTION();
  // Total internal heat gain (W).
  simData.phi_I_tot =
      (simData.phi_int_avg + simData.phi_plug_avg + simData.phi_illum_avg) * structure.floorArea();
}

/**
 * Compute unoccupied heat gain.
 */
void MonthlyModel::unoccupiedHeatGain(MonthlySimulationData &simData) const {
  PROFILE_FUNCTION();

  simData.v_P_tot_wk_nt = calculatePeriodHeatGain(simData.phi_int_wk_nt,
                                          simData.scheduleData.weekdayUnoccupiedMegaseconds,
                                          simData.frac_Pgh_wk_nt, simData.v_E_sol);
  simData.v_P_tot_wke_day = calculatePeriodHeatGain(simData.phi_int_wke_day,
                                            simData.scheduleData.weekendOccupiedMegaseconds,
                                            simData.frac_Pgh_wke_day, simData.v_E_sol);
  simData.v_P_tot_wke_nt = calculatePeriodHeatGain(simData.phi_int_wke_nt,
                                           simData.scheduleData.weekendUnoccupiedMegaseconds,
                                           simData.frac_Pgh_wke_nt, simData.v_E_sol);

  if (DEBUG_ISO_MODEL_SIMULATION) {
    printVector("v_P_tot_wk_nt", simData.v_P_tot_wk_nt);
    printVector("v_P_tot_wke_day", simData.v_P_tot_wke_day);
    printVector("v_P_tot_wke_nt", simData.v_P_tot_wke_nt);
  }
}

double MonthlyModel::calculateBEMAdjustment() const {
  switch ((int)building.buildingEnergyManagement()) {
  case 1:
    return 0.0;
  case 2:
    return BEM_SIMPLE_ADJUSTMENT;
  case 3:
    return BEM_ADVANCED_ADJUSTMENT;
  default:
    return 0.0;
  }
}

void MonthlyModel::calculateWeekendTemperatures(
    const Vector &v_decay_start_base, const Vector &v_limit_start_col0,
    double tset_unocc, double tau, const Vector &v_ti, const Matrix &M_dT,
    const Matrix &M_Te, Vector &v_wke_avg, Vector &v_wk_nt) const {

  // 1. Calculate exponential decay (floating temperature)
  // M_Decay corresponds to M_Ta (heating) or M_Tc (cooling)
  Matrix M_Decay(monthsInYear, 4);
  Vector v_Tstart(v_decay_start_base);

  for (unsigned int i = 0; i < M_Decay.size2(); i++) {
    for (unsigned int j = 0; j < M_Decay.size1(); j++) {
      v_Tstart[j] = M_Decay(j, i) =
          (v_Tstart[j] - M_Te(j, i) - M_dT(j, i)) * exp(-1 * v_ti[i] / tau) +
          M_Te(j, i) + M_dT(j, i);
    }
  }

  // 2. Apply setpoint limits
  // M_Limit corresponds to M_Taa (heating) or M_Tcc (cooling)
  Matrix M_Limit(monthsInYear, 5);

  // Initialize column 0
  for (unsigned int j = 0; j < M_Limit.size1(); j++) {
    M_Limit(j, 0) = v_limit_start_col0[j];
  }

  // Apply limits for subsequent columns
  for (unsigned int i = 1; i < M_Limit.size2(); i++) {
    for (unsigned int j = 0; j < M_Limit.size1(); j++) {
      M_Limit(j, i) = std::max(M_Decay(j, i - 1), tset_unocc);
    }
  }

  // 3. Calculate average temperatures
  // M_Avg corresponds to M_Tb (heating) or M_Td (cooling)
  Matrix M_Avg(monthsInYear, 5);

  for (unsigned int i = 0; i < M_Avg.size2(); i++) {
    for (unsigned int j = 0; j < M_Avg.size1(); j++) {
      double v_T_avg = tau / v_ti[i] *
                           (M_Limit(j, i) - M_Te(j, i) - M_dT(j, i)) *
                           (1 - exp(-1 * v_ti[i] / tau)) +
                       M_Te(j, i) + M_dT(j, i);
      M_Avg(j, i) = std::max(v_T_avg, tset_unocc);
    }
  }

  // 4. Aggregate results
  for (unsigned int i = 0; i < v_wke_avg.size(); i++) {
    double rowSum = 0;
    for (unsigned int j = 0; j < M_Avg.size2(); j++) {
      rowSum += M_Avg(i, j);
    }
    v_wke_avg[i] = rowSum / M_Avg.size2();
  }
  for (unsigned int j = 0; j < M_Avg.size1(); j++) {
    v_wk_nt[j] = M_Avg(j, 1);
  }
}

/*
 * Calculate interior temp.
 */
void MonthlyModel::interiorTemp(MonthlySimulationData &simData) const {
  PROFILE_FUNCTION();
  // Set the temp differential from the interior heating/cooling setpoint
  // based on the BEM type. An advanced BEM has the effect of reducing the
  // effective heating temp and raising the effective cooling temp during
  // times of control (i.e. during occupancy).
  double T_adj = calculateBEMAdjustment();

  if (DEBUG_ISO_MODEL_SIMULATION) {
    std::cout << "BEM: " << building.buildingEnergyManagement() << ", "
              << ((int)building.buildingEnergyManagement()) << std::endl;
    std::cout << "T_adj: " << T_adj << std::endl;
  }

  // Adjust the heating set points.
  double ht_tset_ctrl = heating.temperatureSetPointOccupied() - T_adj;
  double cl_tset_ctrl = cooling.temperatureSetPointOccupied() + T_adj;

  // During unoccupied times, we use a setback temp and even if we have a BEM
  // it has no effect.
  double ht_tset_unocc = heating.temperatureSetPointUnoccupied();
  double cl_tset_unocc = cooling.temperatureSetPointUnoccupied();

  Vector v_ht_tset_ctrl(monthsInYear, ht_tset_ctrl);
  Vector v_cl_tset_ctrl(monthsInYear, cl_tset_ctrl);

  if (DEBUG_ISO_MODEL_SIMULATION) {
    printVector("v_cl_tset_ctrl", v_cl_tset_ctrl);
    printVector("v_ht_tset_ctrl", v_ht_tset_ctrl);
  }

  // Interior heat capacity (J/k).
  double Cm_int = structure.interiorHeatCapacity() * structure.floorArea();

  // Envelope heat capacity (J/k).
  double Cm_env = structure.wallHeatCapacity() * sum(simData.v_wall_A);

  // Total heat capacity (J/k).
  double Cm = Cm_int + Cm_env;

  // Total heat transfer coefficient.
  double H_tot = simData.H_tr + ventilation.H_ve();

  // Building time constant in hours as pwer ISO 13790 12.2.1.3 eq. 62.
  simData.tau = Cm / H_tot / 3600.0;

  // The following code computes the average weekend room temp using exponential
  // rise and decays as we switch between day and night temp settings.  It
  // assumes that the weekend is two days (we'll call them sat and sun)
  //
  // we do this wierd breakdown breakdown because want to separate day with
  // solar loading from night without.  We can then use the average temp
  // in each time frame rather than the overall monthly average.  right now
  // wk_nt stuff is the same as wke_nt, but wke_day is much different because
  // the solar gain increases the heat gain considerably, even on the weekend
  // when occupant, lighting, and plugload gains are small

  // Create a vector of lengths of the periods of times between possible
  // temperature resets during the weekend.
  Vector v_ti(5);
  v_ti[0] = v_ti[2] = v_ti[4] = simData.scheduleData.hoursUnoccupiedPerDay;
  v_ti[1] = v_ti[3] = simData.scheduleData.hoursOccupiedPerDay;

  // Generate an effective delta T matrix from ratio of total interior gains to
  // heat transfer coefficient for each time period.
  //
  // This is a matrix where the columns are the vectors v_P_tot_wk_nt/H_tot, and
  // so on this is for a week night, weekend day, weekend night, weekend day,
  // weekend night sequence
  Matrix M_dT(simData.v_P_tot_wk_nt.size(), 5);
  Matrix M_Te(simData.v_Tdbt_nt.size(), 5);

  for (unsigned int i = 0; i < simData.v_P_tot_wk_nt.size(); ++i) {
    M_dT(i, 0) = simData.v_P_tot_wk_nt[i] / H_tot;
    M_dT(i, 1) = M_dT(i, 3) = simData.v_P_tot_wke_day[i] / H_tot;
    M_dT(i, 2) = M_dT(i, 4) = simData.v_P_tot_wke_nt[i] / H_tot;
  }

  for (unsigned int i = 0; i < simData.v_Tdbt_nt.size(); ++i) {
    M_Te(i, 0) = M_Te(i, 2) = M_Te(i, 4) = simData.v_Tdbt_nt[i];
    M_Te(i, 1) = M_Te(i, 3) = simData.v_Tdbt_day[i];
  }

  if (DEBUG_ISO_MODEL_SIMULATION) {
    printMatrix("M_dT", M_dT);
    printMatrix("M_Te", M_Te);
    printVector("v_ti", v_ti);
  }

  Vector v_Th_wke_avg(v_ht_tset_ctrl);
  Vector v_Th_wk_day(v_ht_tset_ctrl);
  Vector v_Th_wk_nt(v_ht_tset_ctrl);

  if (DEBUG_ISO_MODEL_SIMULATION) {
    printVector("v_Th_wke_avg", v_Th_wke_avg);
    printVector("v_Th_wk_day", v_Th_wk_day);
    printVector("v_Th_wk_nt", v_Th_wk_nt);
  }

  // Compute the change in temp from setback to another heating temp in
  // unoccupied times
  if (heating.T_ht_ctrl_flag() ==
      1) { // If the HVAC heating controls are turned on.
    calculateWeekendTemperatures(v_ht_tset_ctrl, v_ht_tset_ctrl, ht_tset_unocc,
                                 simData.tau, v_ti, M_dT, M_Te, v_Th_wke_avg,
                                 v_Th_wk_nt);
  }

  // Default for if cooling is turned off.
  Vector v_Tc_wk_day(v_cl_tset_ctrl);
  Vector v_Tc_wk_nt(v_cl_tset_ctrl);
  Vector v_Tc_wke_avg(v_cl_tset_ctrl);

  // If cooling is on, find the temp decay after any changes in cooling temp
  // setpoint.
  if (cooling.T_cl_ctrl_flag() == 1) {
    Vector v_limit_start = minimum(v_ht_tset_ctrl, cl_tset_unocc);
    calculateWeekendTemperatures(v_cl_tset_ctrl, v_limit_start, cl_tset_unocc,
                                 simData.tau, v_ti, M_dT, M_Te, v_Tc_wke_avg,
                                 v_Tc_wk_nt);
  }

  if (DEBUG_ISO_MODEL_SIMULATION) {
    printVector("v_Tc_wk_day", v_Tc_wk_day);
    printVector("v_Tc_wk_nt", v_Tc_wk_nt);
    printVector("v_Tc_wke_avg", v_Tc_wke_avg);
  }

  // Find the average temp for the whole week from the fractions of each period.
  Vector v_Th_wk_avg = sum(
      sum(mult(v_Th_wk_day, simData.scheduleData.frac_hrs_wk_day), mult(v_Th_wk_nt, simData.scheduleData.frac_hrs_wk_nt)),
      mult(v_Th_wke_avg, simData.scheduleData.frac_hrs_wke_tot));
  Vector v_Tc_wk_avg = sum(
      sum(mult(v_Tc_wk_day, simData.scheduleData.frac_hrs_wk_day), mult(v_Tc_wk_nt, simData.scheduleData.frac_hrs_wk_nt)),
      mult(v_Tc_wke_avg, simData.scheduleData.frac_hrs_wke_tot));

  if (DEBUG_ISO_MODEL_SIMULATION) {
    printVector("v_Tc_wk_avg", v_Tc_wk_avg);
    printVector("v_Th_wk_avg", v_Th_wk_avg);
  }

  // The final avg for monthly energy computations is the lesser of the avg
  // computed above and the heating set control.
  for (unsigned int i = 0; i < v_Tc_wk_avg.size(); i++) {
    simData.v_Th_avg[i] = std::min(v_Th_wk_avg[i], ht_tset_ctrl);
    simData.v_Tc_avg[i] = std::min(v_Tc_wk_avg[i], cl_tset_ctrl);
  }
}

/**
 * Calculate required energy for mechanical ventilation based on source EN ISO
 * 13789 C.3, C.5 and EN 15242:2007 6.7 and EN ISO 13790 Sec 9.2.
 */
void MonthlyModel::ventilationCalc(MonthlySimulationData &simData) const {
  PROFILE_FUNCTION();
  // Optimization: Cache weather references
  const Vector &v_mdbt = location.weather()->mdbtRef();
  const Vector &v_mwind = location.weather()->mwindRef();

  // Ventilation Zone Height (m) with a minimum of 0.1 m.
  double vent_zone_height =
      std::max(openstudio::isomodel::MIN_VENT_ZONE_HEIGHT, structure.buildingHeight());

  // Vent supply rate m3/h/m2 (input is in in L/s).
  double qv_supp =
      ventilation.supplyRate() / structure.floorArea() / LPS_TO_M3H;

  // Vent exhaust rate m3/h/m2, negative indicates out of building.
  double qv_ext = -(qv_supp - ventilation.supplyDifference() /
                                  structure.floorArea() / LPS_TO_M3H);

  // Combustion appliance ventilation rate - not implemented yet but will be
  // impt for restaurants.
  double qv_comb = 0;

  // Difference between air intake and air exhaust including combustion exhaust.
  double qv_diff = qv_supp + qv_ext + qv_comb;

  double vent_ht_recov = ventilation.heatRecoveryEfficiency();

  double vent_outdoor_frac = 1 - ventilation.exhaustAirRecirculated();

  // Infilatration source EN 15242:2007 Sec 6.7 direct method
  double tot_env_A = sum(structure.wallArea()) + sum(structure.windowArea());

  // Infiltration data from:
  // Tamura, (1976), Studies on exterior wall air tightness and air infiltration
  // of tall buildings, ASHRAE Transactions, 82(1), 122-134. Orm (1998), AIVC
  // TN44: Numerical data for air infiltration and natural ventilation
  // calculations, Air Infiltration and Ventilation Centre. Emmerich, (2005),
  // Investigation of the Impact of Commercial Building Envelope Airtightness on
  // HVAC Energy Use.

  // Infiltration rate in m3/h/m2 @ 75 Pa based on wall area.
  double v_Q75pa = structure.infiltrationRate();

  // Convert infiltration to Q@4Pa in m3/h /m2 based on floor area.
  // double v_Q4pa = v_Q75pa * tot_env_A / structure.floorArea() *
  // (std::pow((4.0 / 75.0), ventilation.p_exp()));
  double v_Q4pa = v_Q75pa;

  // Effective stack height.
  double h_stack = ventilation.zone_frac() * vent_zone_height;

  Vector dbtDiff = dif(v_mdbt, simData.v_Th_avg);
  printVector("dbtDiff", dbtDiff);
  Vector dbtDiffAbs = abs(dbtDiff);
  printVector("dbtDiffAbs", dbtDiffAbs);
  Vector dbtHStack = mult(dbtDiffAbs, h_stack);
  printVector("dbtHstack", dbtHStack);
  Vector dbtPowered = pow(dbtHStack, ventilation.stack_exp());
  printVector("dbtPowered", dbtPowered);
  Vector dbtMultQ4 = mult(dbtPowered, ventilation.stack_coeff() * v_Q4pa);
  printVector("dbtMultQ4", dbtMultQ4);

  // Calculate the infiltration from stack effect pressure difference for
  // heating from EN 15242: sec 6.7.1 (m3/h/m2).
  Vector v_qv_stack_ht = maximum(dbtMultQ4, MIN_INFILTRATION_FLOW);

  // Recalculate for cooling.
  dbtDiff = dif(v_mdbt, simData.v_Tc_avg);
  printVector("dbtDiff", dbtDiff);
  dbtDiffAbs = abs(dbtDiff);
  printVector("dbtDiffAbs", dbtDiffAbs);
  dbtHStack = mult(dbtDiffAbs, h_stack);
  printVector("dbtHstack", dbtHStack);
  dbtPowered = pow(dbtHStack, ventilation.stack_exp());
  printVector("dbtPowered", dbtPowered);
  dbtMultQ4 = mult(dbtPowered, ventilation.stack_coeff() * v_Q4pa);
  printVector("dbtMultQ4", dbtMultQ4);

  // Calculate the infiltration from stack effect pressure difference for
  // cooling from EN 15242: sec 6.7.1 (m3/h/m2).
  Vector v_qv_stack_cl = maximum(dbtMultQ4, MIN_INFILTRATION_FLOW);
  printVector("v_qv_stack_ht", v_qv_stack_ht);
  printVector("v_qv_stack_cl", v_qv_stack_cl);

  Vector v_qv_wind_ht =
      mult(mult(pow(mult(mult(v_mwind, v_mwind),
                         ventilation.dCp() * location.terrain()),
                    ventilation.wind_exp()),
                v_Q4pa),
           ventilation.wind_coeff());
  Vector v_qv_wind_cl =
      mult(mult(pow(mult(mult(v_mwind, v_mwind),
                         ventilation.dCp() * location.terrain()),
                    ventilation.wind_exp()),
                v_Q4pa),
           ventilation.wind_coeff());
  printVector("v_qv_wind_ht", v_qv_wind_ht);
  printVector("v_qv_wind_cl", v_qv_wind_cl);

  Vector v_qv_ht_max = maximum(v_qv_stack_ht, v_qv_wind_ht);
  Vector v_qv_cl_max = maximum(v_qv_stack_cl, v_qv_wind_cl);
  printVector("v_qv_ht_max", v_qv_ht_max);
  printVector("v_qv_cl_max", v_qv_cl_max);

  Vector v_qv_sw_ht =
      sum(v_qv_ht_max, div(mult(mult(v_qv_stack_ht, v_qv_wind_ht), n_sw_coeff),
                           v_Q4pa)); // m3/h/m2
  Vector v_qv_sw_cl =
      sum(v_qv_cl_max, div(mult(mult(v_qv_stack_cl, v_qv_wind_cl), n_sw_coeff),
                           v_Q4pa)); // m3/h/m2
  printVector("v_qv_sw_ht", v_qv_sw_ht);
  printVector("v_qv_sw_cl", v_qv_sw_cl);

  Vector v_qv_inf_ht = sum(v_qv_sw_ht, std::max(0.0, -qv_diff)); // m3/h/m2
  Vector v_qv_inf_cl = sum(v_qv_sw_cl, std::max(0.0, -qv_diff)); // m3/h/m2
  printVector("v_qv_inf_ht", v_qv_inf_ht);
  printVector("v_qv_inf_cl", v_qv_inf_cl);

  // TODO: Figure out what the comment below is refering to. I don't want to
  // delete it just yet because connecting the code to the sources of the
  // equations is important. BAA@2015-07-14.
  //
  // source EN ISO 13789 C.5  There they use Vdot instead of Q
  // Vdot = Vdot_f (1-eta_v) +Vdot_x
  // Vdot_f is the design airflow rate due to mechanical ventilation;
  // Vdot_x is the additional airflow rate with fans on, due to wind effects;
  // ?_v is the global heat recovery efficiency, taking account of the
  // differences between supply and extract airflow rates. Heat in air leaving
  // the building through leakage cannot be recovered.

  // Set vent_rate_flag=0 if ventilation rate is constant, 1 if we assume vent
  // off in unoccopied times or 2 if we assume ventilation rate is dropped
  // proportionally to population set to 1 to mimic the behavior of the original
  // spreadsheet.
  double vent_op_frac;
  switch (ventilation.vent_rate_flag()) {
  case 0:
    vent_op_frac = UNITY_FRACTION;
    break;
  case 1:
    vent_op_frac = simData.scheduleData.frac_hrs_wk_day;
    break;
  default:
    vent_op_frac = std::lerp(pop.densityOccupied() / pop.densityUnoccupied(),
                             1.0, simData.scheduleData.frac_hrs_wk_day);
    break;
  }

  double initVal =
      ventilation.ventType() == 3
          ? 0
          : (vent_op_frac * qv_supp * vent_outdoor_frac * (1 - vent_ht_recov));
  Vector v_qv_mve_ht(monthsInYear, initVal);
  Vector v_qv_mve_cl(monthsInYear, initVal);

  // Total air flow in m3/s when heating.
  Vector v_qve_ht = sum(v_qv_inf_ht, v_qv_mve_ht);
  // Total air flow in m3/s when cooling.
  Vector v_qve_cl = sum(v_qv_inf_cl, v_qv_mve_cl);
  printVector("v_qve_ht", v_qve_ht);
  printVector("v_qve_cl", v_qve_cl);

  // Hve heating (W/K).
  simData.v_Hve_ht = mult(v_qve_ht, rhoCpAirWh);
  // Hve cooling (W/K).
  simData.v_Hve_cl = mult(v_qve_cl, rhoCpAirWh);
}

/**
 * Compute monthly heating and cooling demand.
 */
void MonthlyModel::heatingAndCooling(MonthlySimulationData &simData) const {
  PROFILE_FUNCTION();
  // Optimization: Cache weather reference
  const Vector &v_mdbt = location.weather()->mdbtRef();

  // Convert internal heat gains from W to MJ.
  Vector temp = mult(megasecondsInMonth, simData.phi_I_tot, monthsInYear);

  // Total internal + solar heat gains (MJ).
  Vector v_tot_mo_ht_gain = sum(temp, simData.v_E_sol);

  // Building heating dimensionless constant.
  double a_H = heating.a_H0() + simData.tau / heating.tau_H0();

  // Heat transfer (loss) by transmission, heating (MJ).
  Vector v_QT_ht = mult(mult(dif(simData.v_Th_avg, v_mdbt), megasecondsInMonth), simData.H_tr);
  // Heat transfer (loss) by ventilation, heating (MJ).
  Vector v_QV_ht =
      mult(mult(mult(simData.v_Hve_ht, structure.floorArea()), dif(simData.v_Th_avg, v_mdbt)),
           megasecondsInMonth);
  // Total heat transfer (loss) (MJ). ISO 13790 7.2.1.3 eq. 7.
  Vector v_Qtot_ht = sum(v_QT_ht, v_QV_ht);

  // Compute the ratio of heat gain to heat loss.
  Vector v_gamma_H_ht = div(
      v_tot_mo_ht_gain,
      sum(v_Qtot_ht,
          std::numeric_limits<
              double>::epsilon())); // Add
                                    // std::numeric_limits<double>::epsilon()
                                    // to avoid divide by zero.

  // Heating utilization factor.
  // Vector v_eta_g_H(monthsInYear);

  // // For each month, set the check the heat gain ratio and set the heating
  // // utlization factor accordingly.
  // for (unsigned int i = 0; i < v_eta_g_H.size(); i++) {
  //   if (v_gamma_H_ht[i] > 0) {
  //     // Optimization: x^(a+1) = x^a * x
  //     double num = std::pow(v_gamma_H_ht[i], a_H);
  //     v_eta_g_H[i] = (1.0 - num) / (1.0 - num * v_gamma_H_ht[i]);
  //   } else {
  //     v_eta_g_H[i] =
  //         1.0 / (v_gamma_H_ht[i] + std::numeric_limits<double>::epsilon());
  //   }
  // }


  Vector v_eta_g_H = calculateUtilizationFactor(v_gamma_H_ht, a_H);

  // Ensure v_Qneed_ht is initialized to the correct size before use
  simData.v_Qneed_ht.resize(monthsInYear);
  // Total heating need (MJ).
  simData.v_Qneed_ht = dif(v_Qtot_ht, mult(v_eta_g_H, v_tot_mo_ht_gain));
  simData.Qneed_ht_yr = sum(simData.v_Qneed_ht);


  // Total heating need (MJ).
  simData.v_Qneed_ht = dif(v_Qtot_ht, mult(v_eta_g_H, v_tot_mo_ht_gain));
  simData.Qneed_ht_yr = sum(simData.v_Qneed_ht);

  // Heat transfer (loss) by transmission, cooling (MJ).
  Vector v_QT_cl = mult(mult(dif(simData.v_Tc_avg, v_mdbt), simData.H_tr), megasecondsInMonth);
  // Heat transfer (loss) by ventilation, cooling (MJ).
  Vector v_QV_cl =
      mult(mult(mult(simData.v_Hve_cl, structure.floorArea()), dif(simData.v_Tc_avg, v_mdbt)),
           megasecondsInMonth);
  // Total heat transfer (loss), cooling (MJ). ISO 13790 7.2.1.3 eq. 7.
  Vector v_Qtot_cl = sum(v_QT_cl, v_QV_cl);

  // Heat transfer (loss) to heat gain ratio, cooling.
  Vector v_gamma_H_cl = div(
      v_Qtot_cl, sum(v_tot_mo_ht_gain, std::numeric_limits<double>::epsilon()));

  // Compute the cooling gain utilization factor eta_g_cl
  Vector v_eta_g_CL = calculateUtilizationFactor(v_gamma_H_cl, a_H);

  // Ensure v_Qneed_cl is initialized to the correct size before use
  simData.v_Qneed_cl.resize(monthsInYear);
  // Total cooling need (MJ).
  simData.v_Qneed_cl = dif(v_tot_mo_ht_gain, mult(v_eta_g_CL, v_Qtot_cl));
  simData.Qneed_cl_yr = sum(simData.v_Qneed_cl);

  // Hot air supply temperature (C).
  double T_sup_ht =
      heating.temperatureSetPointOccupied() + heating.dT_supp_ht();
  // Cool air supply temperature (C).
  double T_sup_cl =
      cooling.temperatureSetPointOccupied() - cooling.dT_supp_cl();

  // Calculate air volumes for heating and cooling
  auto [v_Vair_ht, v_Vair_cl] =
      calculateAirVolumes(simData.v_Qneed_ht, simData.v_Qneed_cl, simData.v_Th_avg, simData.v_Tc_avg);
  printVector("v_Vair_ht", v_Vair_ht);
  printVector("v_Vair_cl", v_Vair_cl);

  // Calculate total air flow
  Vector v_Vair_tot =
      calculateTotalAirFlow(v_Vair_ht, v_Vair_cl, simData.scheduleData.frac_hrs_wk_day);
  printVector("v_Vair_tot", v_Vair_tot);

  // Calculate fan energy
  Vector fanEnergy = calculateFanEnergy(v_Vair_tot);
  printVector("fanEnergy", fanEnergy);

  if (DEBUG_ISO_MODEL_SIMULATION) {
    std::cout << "ventilation.fanPower() = " << ventilation.fanPower()
              << std::endl;
    std::cout << "ventilation.fanControlFactor() = "
              << ventilation.fanControlFactor() << std::endl;
    std::cout << "structure.floorArea() = " << structure.floorArea()
              << std::endl;
  }

  // Calculate fan EUI (kWh/m2).
  simData.v_Qfan_tot = div(div(fanEnergy, structure.floorArea()), kWh2MJ);
}

void MonthlyModel::calculateHeatingSystemLoads(const Vector &v_Qneed_ht,
                                               const Vector &v_Qloss_ht_dist,
                                               Vector &v_Qht_sys,
                                               Vector &v_Qht_DH) const {
  PROFILE_FUNCTION();
  if (heating.DH_YesNo() == 1) {
    // If District Heating is enabled, the district heating system handles the load
    v_Qht_DH = sum(v_Qneed_ht, v_Qloss_ht_dist);
    v_Qht_sys.assign(monthsInYear, 0.0); // No local system load
  } else {
    // Otherwise, the local system handles the load
    v_Qht_sys =
        div(sum(v_Qloss_ht_dist, v_Qneed_ht),
            heating.efficiency() + std::numeric_limits<double>::epsilon());
    v_Qht_DH.assign(monthsInYear, 0.0); // No district heating load
  }
}

void MonthlyModel::calculateCoolingSystemLoads(const Vector &v_Qneed_cl,
                                               const Vector &v_Qloss_cl_dist,
                                               double IEER, Vector &v_Qcl_sys,
                                               Vector &v_Qcool_DC) const {
  PROFILE_FUNCTION();
  if (cooling.DC_YesNo() == 1) {
    // If District Cooling is enabled, the district cooling system handles the load
    v_Qcool_DC = sum(v_Qneed_cl, v_Qloss_cl_dist);
    v_Qcl_sys.assign(monthsInYear, 0.0); // No local system load
  } else {
    // Otherwise, the local system handles the load
    v_Qcl_sys = div(sum(v_Qloss_cl_dist, v_Qneed_cl),
                    IEER + std::numeric_limits<double>::epsilon());
    v_Qcool_DC.assign(monthsInYear, 0.0); // No district cooling load
  }
}

/**
 * HVAC systems calculations.
 */
void MonthlyModel::hvac(const Vector &v_Qneed_ht, const Vector &v_Qneed_cl,
                        double Qneed_ht_yr, double Qneed_cl_yr,
                        Vector &v_Qelec_ht, Vector &v_Qgas_ht,
                        Vector &v_Qcl_elec_tot, Vector &v_Qcl_gas_tot) const {
  PROFILE_FUNCTION();
  // TODO: Implement (or remove) all the district heating/cooling stuff that is
  // currently commented out. BAA@2015-07-15.

  // From original matlab code. Preserved for future implementation of district
  // heating/cooling. BAA@2015-07-15.
  /*
      %% District H/C info

      DH_YesNo =0;  % building connected to DH (0=no, 1=yes.  Assume DH is
     powered by natural gas) n_eta_DH_network = 0.9; % efficiency of DH network.
     Typical value 0l75-0l9 EN 15316-4-5 n_eta_DH_sys = 0.87; % efficiency of DH
     heating system n_frac_DH_free = 0.000; % fraction of free heat source to DH
     (0 to 1)

      DC_YesNo = 0;  % building connected to DC (0=no, 1=yes)
      n_eta_DC_network = 0.9;  % efficiency of DC network.
      n_eta_DC_COP = 5.5;  % COP of DC elec Chillers
      n_eta_DC_frac_abs = 0;  % fraction of DC chillers that are absorption
      n_eta_DC_COP_abs = 1;  % COP of DC absorption chillers
      n_frac_DC_free = 0;  % fraction of free heat source to absorption DC
     chillers (0 to 1)
      */

  // From EN 15243-2007 Annex E.
  // HVAC system info table from EN 15243:2007 Table E1.
  // The integrated energy efficiency ratio (IEER) is the effective average COP
  // for the system.
  double IEER = cooling.cop() * cooling.partialLoadValue();

  // Copy over the HVAC loss/waste factors into local variables with names
  // that match the equations better
  double f_waste = heating.hotcoldWasteFactor();
  double a_ht_loss = heating.hvacLossFactor();
  double a_cl_loss = cooling.hvacLossFactor();

  // Fraction of yearly heating demand with regard to total heating + cooling
  // demand.
  double f_dem_ht = std::max(Qneed_ht_yr / (Qneed_cl_yr + Qneed_ht_yr), MIN_DEMAND_FRACTION);
  // Fraction of yearly cooling demand.
  double f_dem_cl = std::max((1.0 - f_dem_ht), MIN_DEMAND_FRACTION);

  // Overall distribution efficiency for heating.
  double eta_dist_ht = 1.0 / (1.0 + a_ht_loss + f_waste / f_dem_ht);
  // Overall distrubtion efficiency for cooling.
  double eta_dist_cl = 1.0 / (1.0 + a_cl_loss + f_waste / f_dem_cl);

  // Losses from HVAC distributuion, heating.
  Vector v_Qloss_ht_dist =
      div(mult(v_Qneed_ht, (1 - eta_dist_ht)), eta_dist_ht);
  // Losses from HVAC distributuion, cooling.
  Vector v_Qloss_cl_dist =
      div(mult(v_Qneed_cl, (1 - eta_dist_cl)), eta_dist_cl);
  printVector("v_Qloss_ht_dist", v_Qloss_ht_dist);
  printVector("v_Qloss_cl_dist", v_Qloss_cl_dist);

  Vector v_Qht_sys(monthsInYear, 0.0);
  Vector v_Qht_DH(monthsInYear, 0.0);
  Vector v_Qcl_sys(monthsInYear, 0.0);
  Vector v_Qcool_DC(monthsInYear, 0.0);

  calculateHeatingSystemLoads(v_Qneed_ht, v_Qloss_ht_dist, v_Qht_sys, v_Qht_DH);
  calculateCoolingSystemLoads(v_Qneed_cl, v_Qloss_cl_dist, IEER, v_Qcl_sys, v_Qcool_DC);

  printVector("v_Qht_sys", v_Qht_sys);
  printVector("v_Qht_DH", v_Qht_DH);
  printVector("v_Qcl_sys", v_Qcl_sys);
  printVector("v_Qcool_DC", v_Qcool_DC);
  Vector v_Qcl_DC_elec = div(mult(v_Qcool_DC, 1 - cooling.eta_DC_frac_abs()),
                             cooling.eta_DC_COP() * cooling.eta_DC_network());
  Vector v_Qcl_DC_abs = div(mult(v_Qcool_DC, 1 - cooling.frac_DC_free()),
                            cooling.eta_DC_COP_abs());
  printVector("v_Qcl_DC_elec", v_Qcl_DC_elec);
  printVector("v_Qcl_DC_abs", v_Qcl_DC_abs);

  Vector v_Qht_DH_total = div(mult(v_Qht_DH, 1 - heating.frac_DH_free()),
                              heating.eta_DH_sys() * heating.eta_DH_network());
  v_Qcl_elec_tot = sum(v_Qcl_sys, v_Qcl_DC_elec);
  v_Qcl_gas_tot = v_Qcl_DC_abs;
  printVector("v_Qht_DH_total", v_Qht_DH_total);
  printVector("v_Qcl_elec_tot", v_Qcl_elec_tot);
  printVector("v_Qcl_gas_tot", v_Qcl_gas_tot);

  // Vector v_Qelec_ht,v_Qgas_ht;

  if (heating.energyType() == 1) {
    v_Qelec_ht = v_Qht_sys;
    v_Qgas_ht = v_Qht_DH_total;
  } else {
    v_Qelec_ht.assign(monthsInYear, 0.0);
    v_Qgas_ht = sum(v_Qht_sys, v_Qht_DH_total);
  }
  printVector("v_Qelec_ht", v_Qelec_ht);
  printVector("v_Qgas_ht", v_Qgas_ht);
}

Vector MonthlyModel::calculatePumpEnergyForMode(
    const Vector &v_Qneed_mode, const Vector &v_Qneed_total,
    double E_pumps_w_per_m2, double pump_control_reduction) const {
  PROFILE_FUNCTION();

  // Total annual pump energy for the mode if pumps run continuously (MJ/m2).
  double Q_pumps_yr_mode_per_m2 =
      sum(mult(megasecondsInMonth, E_pumps_w_per_m2, monthsInYear));

  // Fraction of time the system is in this mode each month.
  Vector v_frac_mode = div(v_Qneed_mode, v_Qneed_total);

  // Total energy fraction for this mode over the year.
  double frac_total = sum(v_frac_mode);

  // Total yearly pump energy, adjusted by control factor and floor area (MJ).
  double Q_pumps_mode =
      Q_pumps_yr_mode_per_m2 * pump_control_reduction * structure.floorArea();

  // Distribute the total annual pump energy for this mode across the months.
  return div(mult(v_frac_mode, Q_pumps_mode),
             frac_total + std::numeric_limits<double>::epsilon());
}

/**
 * Calculate energy for pumps used in the heating/cooling systems.
 * References: EPA NR 6.9.7.1 and 6.9.7.2, EN 15243.
 */
void MonthlyModel::pump(const Vector &v_Qneed_ht, const Vector &v_Qneed_cl,
                        double Qneed_ht_yr, double Qneed_cl_yr,
                        Vector &v_Q_pump_tot) const {
  PROFILE_FUNCTION();
  // TODO: The current implementation is wrong. It either needs to be revised to
  // be more like the hourly implementation where the pump energy is multiplied
  // by the amount of time the pumps are actually on or
  // heating.E_pumps()/cooling.E_pumps() needs to be expressed in terms of the
  // heating/cooling delivered so that the pump energy can be determined by
  // multiplying it by the heating/cooling delivered. Both methods have
  // challenges, which is why they are not yet implements. Until then, consider
  // the monthly pump values unreliable. BAA@2015-07-15.

  // Total monthly heating and cooling need (MJ).
  Vector v_Qneed_total = sum(v_Qneed_ht, v_Qneed_cl);

  // Calculate monthly pump energy for heating mode.
  Vector v_Q_pumps_ht = calculatePumpEnergyForMode(
      v_Qneed_ht, v_Qneed_total, heating.E_pumps(), heating.pumpControlReduction());

  // Calculate monthly pump energy for cooling mode.
  Vector v_Q_pumps_cl = calculatePumpEnergyForMode(
      v_Qneed_cl, v_Qneed_total, cooling.E_pumps(), cooling.pumpControlReduction());

  // Total pump operational factor.
  Vector v_frac_tot =
      div(sum(v_Qneed_ht, v_Qneed_cl), Qneed_ht_yr + Qneed_cl_yr);
  double frac_total = sum(v_frac_tot);
  double Q_pumps_tot = sum(v_Q_pumps_ht) + sum(v_Q_pumps_cl);

  if (sum(v_Q_pumps_ht) == 0.0 || sum(v_Q_pumps_cl) == 0.0) {
    // If there is just heating or just cooling, use the individual heating or
    // cooling pump energy vector.
    v_Q_pump_tot = sum(v_Q_pumps_ht, v_Q_pumps_cl);
  } else {
    // Otherwise, distribut the combined pump energy proportional to the
    // combined heating/cooling load.
    v_Q_pump_tot = div(mult(v_frac_tot, Q_pumps_tot),
                       frac_total + std::numeric_limits<double>::epsilon());
  }
}

/**
 * Energy Generation
 * NOT INCLUDED YET
 */
void MonthlyModel::energyGeneration() const { PROFILE_FUNCTION(); }

/**
 * Calculate domestic hot water (DHW).
 * References: NEN 2916 12.2
 */
void MonthlyModel::heatedWater(Vector &v_Q_dhw_elec,
                               Vector &v_Q_dhw_gas) const {
  PROFILE_FUNCTION();
  // Energy from solar energy hot water collectors - not included yet
  Vector v_Q_dhw_solar(monthsInYear, 0.0);

  // Total annual energy demand required for heating DHW (MJ/yr).
  double Q_dhw_yr = heating.hotWaterDemand() *
                    (heating.dhw_tset() - heating.dhw_tsupply()) * rhoCpWater;

  Vector v_MonthlyDemand = mult(daysInMonth, Q_dhw_yr, monthsInYear);
  Vector v_frac_MonthlyDemand_yr = div(v_MonthlyDemand, daysInYear);
  Vector v_Qe_demand =
      div(v_frac_MonthlyDemand_yr, heating.hotWaterDistributionEfficiency());

  // Monthly DHW energy demand including distribution efficiency.
  Vector v_Q_dhw_demand = div(v_Qe_demand, kWh2MJ);
  // Total monthly supply need is (demand - solar)/system efficiency.
  Vector v_Q_dhw_need = maximum(div(dif(v_Q_dhw_demand, v_Q_dhw_solar),
                                    heating.hotWaterSystemEfficiency()),
                                0);

  if (DEBUG_ISO_MODEL_SIMULATION) {
    printVector("v_MonthlyDemand", v_MonthlyDemand);
    printVector("v_frac_MonthlyDemand_yr", v_frac_MonthlyDemand_yr);
    printVector("v_Qe_demand", v_Qe_demand);
    printVector("v_Q_dhw_demand", v_Q_dhw_demand);
    printVector("v_Q_dhw_need", v_Q_dhw_need);
  }

  if (heating.hotWaterEnergyType() == 1) {
    v_Q_dhw_elec = v_Q_dhw_need;
    v_Q_dhw_gas.assign(v_Q_dhw_need.size(), 0.0);
  } else {
    v_Q_dhw_gas = v_Q_dhw_need;
    v_Q_dhw_elec.assign(v_Q_dhw_need.size(), 0.0);
  }

  if (DEBUG_ISO_MODEL_SIMULATION) {
    printVector("v_Q_dhw_gas", v_Q_dhw_gas);
    printVector("v_Q_dhw_elec", v_Q_dhw_elec);
  }
}

std::vector<EndUses> MonthlyModel::simulate() const {
  PROFILE_FUNCTION();

  Vector v_Qelec_ht, v_Qcl_elec_tot,
      v_Qfan_tot, v_Q_pump_tot, v_Q_dhw_elec, v_Qgas_ht, v_Qcl_gas_tot,
      v_Q_dhw_gas;

  // openstudio::isomodel::loadDefaults(monthlyModel);

  if (DEBUG_ISO_MODEL_SIMULATION) {
    std::cout << std::endl << "scheduleAndOccupancy: " << std::endl;
  }
  MonthlySimulationData simData; // Declare the new struct
  simData.scheduleData = schedules::getMonthlySchedules(pop);

  if (DEBUG_ISO_MODEL_SIMULATION) {
    std::cout << "frac_hrs_wk_day: " << simData.scheduleData.frac_hrs_wk_day << std::endl;
    std::cout << "hoursUnoccupiedPerDay: " << simData.scheduleData.hoursUnoccupiedPerDay
              << std::endl;
    std::cout << "hoursOccupiedPerDay: " << simData.scheduleData.hoursOccupiedPerDay << std::endl;
    std::cout << "frac_hrs_wk_nt: " << simData.scheduleData.frac_hrs_wk_nt << std::endl;
    std::cout << "frac_hrs_wke_tot: " << simData.scheduleData.frac_hrs_wke_tot << std::endl;

    printVector("weekdayOccupiedMegaseconds", simData.scheduleData.weekdayOccupiedMegaseconds);
    printVector("weekdayUnoccupiedMegaseconds", simData.scheduleData.weekdayUnoccupiedMegaseconds);
    printVector("weekendOccupiedMegaseconds", simData.scheduleData.weekendOccupiedMegaseconds);
    printVector("weekendUnoccupiedMegaseconds", simData.scheduleData.weekendUnoccupiedMegaseconds);
    printVector("clockHourOccupied", simData.scheduleData.clockHourOccupied);
    printVector("clockHourUnoccupied", simData.scheduleData.clockHourUnoccupied);

    std::cout << std::endl << "solarRadiationBreakdown: " << std::endl;
  }
  solarRadiationBreakdown(simData);

  // Bridge variables for downstream functions
  const Vector& v_hrs_sun_down_mo = simData.v_hrs_sun_down_mo;
  const Vector& frac_Pgh_wk_nt = simData.frac_Pgh_wk_nt;
  const Vector& frac_Pgh_wke_day = simData.frac_Pgh_wke_day;
  const Vector& frac_Pgh_wke_nt = simData.frac_Pgh_wke_nt;
  const Vector& v_Tdbt_nt = simData.v_Tdbt_nt;
  const Vector& v_Tdbt_day = simData.v_Tdbt_day;

  if (DEBUG_ISO_MODEL_SIMULATION) {
    printVector("v_hrs_sun_down_mo", v_hrs_sun_down_mo);
    printVector("frac_Pgh_wk_nt", frac_Pgh_wk_nt);
    printVector("frac_Pgh_wke_day", frac_Pgh_wke_day);
    printVector("frac_Pgh_wke_nt", frac_Pgh_wke_nt);
    printVector("v_Tdbt_nt", v_Tdbt_nt);
    printVector("v_Tdbt_day", v_Tdbt_day);

    std::cout << std::endl << "lightingEnergyUse: " << std::endl;
  }
  lightingEnergyUse(simData);
  // Bridge variables for downstream functions
  double Q_illum_occ = simData.Q_illum_occ;
  double Q_illum_unocc = simData.Q_illum_unocc;
  double Q_illum_tot_yr = simData.Q_illum_tot_yr;
  const Vector& v_Q_illum_tot = simData.v_Q_illum_tot;
  const Vector& v_Q_illum_ext_tot = simData.v_Q_illum_ext_tot;

  if (DEBUG_ISO_MODEL_SIMULATION) {
    std::cout << "Q_illum_occ: " << Q_illum_occ << std::endl;
    std::cout << "Q_illum_unocc: " << Q_illum_unocc << std::endl;
    std::cout << "Q_illum_unocc: " << Q_illum_unocc << std::endl;
    printVector("v_Q_illum_tot", v_Q_illum_tot);
    printVector("v_Q_illum_ext_tot", v_Q_illum_ext_tot);

    std::cout << std::endl
              << "envelopCalculations: " << std::endl; /*
v_wall_A = structure.wallArea();
v_win_A = structure.windowArea();
v_wall_U = structure.wallUniform();
Vector v_win_U = structure.windowUniform();*/
    printVector("structure.wallArea()", structure.wallArea());
    printVector("structure.windowArea()", structure.windowArea());
    printVector("structure.wallUniform()", structure.wallUniform());
    printVector("structure.windowUniform()", structure.windowUniform());
  }
  envelopCalculations(simData);
  // Bridge variables for downstream functions
  const Vector& v_wall_U = simData.v_wall_U;
  const Vector& v_wall_A = simData.v_wall_A;
  double H_tr = simData.H_tr;

  if (DEBUG_ISO_MODEL_SIMULATION) {
    std::cout << "H_tr: " << H_tr << std::endl;
    printVector("v_win_A", simData.v_win_A);
    printVector("v_wall_emiss", simData.v_wall_emiss);
    printVector("v_wall_alpha_sc", simData.v_wall_alpha_sc);
    printVector("v_wall_U", v_wall_U);
    printVector("v_wall_A", v_wall_A);

    std::cout << std::endl << "windowSolarGain: " << std::endl;
  }
  windowSolarGain(simData);

  if (DEBUG_ISO_MODEL_SIMULATION) {
    printVector("v_wall_A_sol", simData.v_wall_A_sol);
    printVector("v_win_hr", simData.v_win_hr);
    printVector("v_wall_R_sc", simData.v_wall_R_sc);
    printVector("v_win_A_sol", simData.v_win_A_sol);

    std::cout << std::endl << "solarHeatGain: " << std::endl;
  }
  solarHeatGain(simData);
  const Vector& v_E_sol = simData.v_E_sol;

  if (DEBUG_ISO_MODEL_SIMULATION) {
    printVector("v_E_sol", v_E_sol);

    std::cout << std::endl << "heatGainsAndLosses: " << std::endl;
  }
  heatGainsAndLosses(simData);
  if (DEBUG_ISO_MODEL_SIMULATION) {
    std::cout << "phi_int_avg: " << simData.phi_int_avg << std::endl;
    std::cout << "phi_plug_avg: " << simData.phi_plug_avg << std::endl;
    std::cout << "phi_illum_avg: " << simData.phi_illum_avg << std::endl;
    std::cout << "phi_int_wke_nt: " << simData.phi_int_wke_nt << std::endl;
    std::cout << "phi_int_wke_day: " << simData.phi_int_wke_day << std::endl;
    std::cout << "phi_int_wk_nt: " << simData.phi_int_wk_nt << std::endl;

    std::cout << std::endl << "internalHeatGain: " << std::endl;
  }
  internalHeatGain(simData);
  // Bridge variables for downstream functions
  double phi_I_tot = simData.phi_I_tot;

  if (DEBUG_ISO_MODEL_SIMULATION) {
    std::cout << "phi_I_tot: " << phi_I_tot << std::endl;

    std::cout << std::endl << "unoccupiedHeatGain: " << std::endl;
  }
  unoccupiedHeatGain(simData);
  if (DEBUG_ISO_MODEL_SIMULATION) {
    std::cout << std::endl << "interiorTemp: " << std::endl;
  }
  interiorTemp(simData);

  if (DEBUG_ISO_MODEL_SIMULATION) {
    std::cout << "tau: " << simData.tau << std::endl;
    printVector("v_Th_avg", simData.v_Th_avg);
    printVector("v_Tc_avg", simData.v_Tc_avg);

    std::cout << std::endl << "ventilationCalc: " << std::endl;
  }
  ventilationCalc(simData);
  if (DEBUG_ISO_MODEL_SIMULATION) {
    printVector("v_Hve_ht", simData.v_Hve_ht);
    printVector("v_Hve_cl", simData.v_Hve_cl);

    std::cout << std::endl << "heatingAndCooling: " << std::endl;
  }
  heatingAndCooling(simData);
  if (DEBUG_ISO_MODEL_SIMULATION) {
    std::cout << "Qneed_ht_yr: " << simData.Qneed_ht_yr << std::endl;
    std::cout << "Qneed_cl_yr: " << simData.Qneed_cl_yr << std::endl;
    printVector("v_Qfan_tot", simData.v_Qfan_tot);

    std::cout << std::endl << "hvac: " << std::endl;
  }
  hvac(simData.v_Qneed_ht, simData.v_Qneed_cl, simData.Qneed_ht_yr, simData.Qneed_cl_yr, v_Qelec_ht, v_Qgas_ht,
       v_Qcl_elec_tot, v_Qcl_gas_tot);
  if (DEBUG_ISO_MODEL_SIMULATION) {
    printVector("v_Qelec_ht", v_Qelec_ht);
    printVector("v_Qgas_ht", v_Qgas_ht);
    printVector("v_Qcl_elec_tot", v_Qcl_elec_tot);
    printVector("v_Qcl_gas_tot", v_Qcl_gas_tot);

    std::cout << std::endl << "pump: " << std::endl;
  }
  pump(simData.v_Qneed_ht, simData.v_Qneed_cl, simData.Qneed_ht_yr, simData.Qneed_cl_yr, v_Q_pump_tot);
  if (DEBUG_ISO_MODEL_SIMULATION) {
    printVector("v_Q_pump_tot", v_Q_pump_tot);

    std::cout << std::endl << "energyGeneration: " << std::endl;
  }
  energyGeneration();
  if (DEBUG_ISO_MODEL_SIMULATION) {
    std::cout << std::endl << "heatedWater: " << std::endl;
  }
  heatedWater(v_Q_dhw_elec, v_Q_dhw_gas);
  if (DEBUG_ISO_MODEL_SIMULATION) {
    printVector("v_Q_dhw_elec", v_Q_dhw_elec);
    printVector("v_Q_dhw_gas", v_Q_dhw_gas);
  }

  return outputGeneration(v_Qelec_ht, v_Qcl_elec_tot, v_Q_illum_tot,
                          v_Q_illum_ext_tot, simData.v_Qfan_tot, v_Q_pump_tot,
                          v_Q_dhw_elec, v_Qgas_ht, v_Qcl_gas_tot,
                          v_Q_dhw_gas, simData.scheduleData.frac_hrs_wk_day); // Use simData.scheduleData
}
std::vector<EndUses> MonthlyModel::outputGeneration(
    const Vector &v_Qelec_ht, const Vector &v_Qcl_elec_tot,
    const Vector &v_Q_illum_tot, const Vector &v_Q_illum_ext_tot,
    const Vector &v_Qfan_tot, const Vector &v_Q_pump_tot,
    const Vector &v_Q_dhw_elec, const Vector &v_Qgas_ht,
    const Vector &v_Qcl_gas_tot, const Vector &v_Q_dhw_gas, double frac_hrs_wk_day)
    const {
  PROFILE_FUNCTION();
  std::vector<EndUses> allResults;

  // // TODO: Move the plug load calcs to a separate function. BAA@2015-07-15

  // // Average electric plug loads (W/m2).
  // double E_plug_elec =
  //     building.electricApplianceHeatGainOccupied() * frac_hrs_wk_day +
  //     building.electricApplianceHeatGainUnoccupied() * (1.0 - frac_hrs_wk_day);
  // // Average gas plug loads (W/m2).
  // double E_plug_gas =
  //     building.gasApplianceHeatGainOccupied() * frac_hrs_wk_day +
  //     building.gasApplianceHeatGainUnoccupied() * (1.0 - frac_hrs_wk_day);

  // // Electric plug load (kWh/m2).
  // Vector v_Q_plug_elec =
  //     div(mult(hoursInMonth, E_plug_elec, monthsInYear), 1000.0);
  // // Gas plug load (kWh/m2).
  // Vector v_Q_plug_gas =
  //     div(mult(hoursInMonth, E_plug_gas, monthsInYear), 1000.0);
  // printVector("v_Q_plug_elec", v_Q_plug_elec);
  // printVector("v_Q_plug_gas", v_Q_plug_gas);



  // Calculate plug loads
  PlugLoads plugLoads = calculatePlugLoads(frac_hrs_wk_day);
  const Vector &v_Q_plug_elec = plugLoads.v_Q_plug_elec;
  const Vector &v_Q_plug_gas = plugLoads.v_Q_plug_gas;

  if (DEBUG_ISO_MODEL_SIMULATION) {
    printVector("v_Q_plug_elec", v_Q_plug_elec);
    printVector("v_Q_plug_gas", v_Q_plug_gas);
  }

  // // Electric loads (kWh/m2).
  // Vector Eelec_ht = div(div(v_Qelec_ht, structure.floorArea()),
  //                       kWh2MJ); // Total monthly electric usage for heating.
  // Vector Eelec_cl = div(div(v_Qcl_elec_tot, structure.floorArea()),
  //                       kWh2MJ); // Total monthly electric usage for cooling.

  Vector Eelec_ht = convertEnergyToKWhPerSqM(v_Qelec_ht, structure.floorArea());
  Vector Eelec_cl = convertEnergyToKWhPerSqM(v_Qcl_elec_tot, structure.floorArea());
               
  // Total monthly electric usage for interior and exterior lights.
  Vector Eelec_int_lt = div(v_Q_illum_tot, structure.floorArea()); 
  Vector Eelec_ext_lt = div(v_Q_illum_ext_tot,structure.floorArea()); 
  
  // Vector Eelec_fan = v_Qfan_tot; // Total monthly elec usage for fans.
  // Vector Eelec_pump = div(div(v_Q_pump_tot, structure.floorArea()),
  //                         kWh2MJ); // Total monthly elec usage for pumps.
  // Vector Eelec_plug =
  //     v_Q_plug_elec; // Total monthly elec usage for elec plugloads.
  
  Vector Eelec_fan = v_Qfan_tot;                               // Total monthly elec usage for fans.
  Vector Eelec_pump = convertEnergyToKWhPerSqM(v_Q_pump_tot, structure.floorArea()); // Total monthly elec usage for pumps.
  Vector Eelec_plug = v_Q_plug_elec;                           // Total monthly elec usage for elec plugloads.
  
  Vector Eelec_dhw = div(v_Q_dhw_elec, structure.floorArea());

  if (DEBUG_ISO_MODEL_SIMULATION) {
    printVector("v_Qcl_elec_tot", v_Qcl_elec_tot);
    printVector("v_Q_pump_tot", v_Q_pump_tot);
    printVector("Eelec_cl", Eelec_cl);
    printVector("Eelec_pump", Eelec_pump);
    std::cout << "floorArea: " << structure.floorArea() << std::endl;
  }

  // Gas loads (kWh/m2).
  // Vector Egas_ht = div(div(v_Qgas_ht, structure.floorArea()),
  //                      kWh2MJ); // Total monthly gas usage for heating.
  // Vector Egas_cl = div(div(v_Qcl_gas_tot, structure.floorArea()),
  //                      kWh2MJ);    // Total monthly gas usage for cooling.
  // Vector Egas_plug = v_Q_plug_gas; // Total monthly gas plugloads.
  // Vector Egas_dhw = div(
  //     v_Q_dhw_gas, structure.floorArea()); // Total monthly dhw gas plugloads.
  
  // Gas loads (kWh/m2).
  Vector Egas_ht = convertEnergyToKWhPerSqM(v_Qgas_ht, structure.floorArea());
  Vector Egas_cl = convertEnergyToKWhPerSqM(v_Qcl_gas_tot, structure.floorArea());
  Vector Egas_plug = v_Q_plug_gas;
  Vector Egas_dhw = div(v_Q_dhw_gas, structure.floorArea());

  allResults.reserve(monthsInYear);

  for (int i = 0; i < monthsInYear; i++) {

#ifdef ISOMODEL_STANDALONE
    EndUses eu;
    int euse = 0;
    eu.addEndUse(euse++, Eelec_ht[i]);
    eu.addEndUse(euse++, Eelec_cl[i]);
    eu.addEndUse(euse++, Eelec_int_lt[i]);
    eu.addEndUse(euse++, Eelec_ext_lt[i]);
    eu.addEndUse(euse++, Eelec_fan[i]);
    eu.addEndUse(euse++, Eelec_pump[i]);
    eu.addEndUse(euse++, Eelec_plug[i]);
    eu.addEndUse(euse++, 0); // Exterior Equipment
    eu.addEndUse(euse++, Eelec_dhw[i]);
    eu.addEndUse(euse++, Egas_ht[i]);
    eu.addEndUse(euse++, Egas_cl[i]);
    eu.addEndUse(euse++, Egas_plug[i]);
    eu.addEndUse(euse++, Egas_dhw[i]);
    allResults.push_back(eu);
#else
    EndUses eu;
    eu.addEndUse(Eelec_ht[i], EndUseFuelType::Electricity,
                 EndUseCategoryType::Heating);
    eu.addEndUse(Eelec_cl[i], EndUseFuelType::Electricity,
                 EndUseCategoryType::Cooling);
    eu.addEndUse(Eelec_int_lt[i], EndUseFuelType::Electricity,
                 EndUseCategoryType::InteriorLights);
    eu.addEndUse(Eelec_ext_lt[i], EndUseFuelType::Electricity,
                 EndUseCategoryType::ExteriorLights);
    eu.addEndUse(Eelec_fan[i], EndUseFuelType::Electricity,
                 EndUseCategoryType::Fans);
    eu.addEndUse(Eelec_pump[i], EndUseFuelType::Electricity,
                 EndUseCategoryType::Pumps);
    eu.addEndUse(Eelec_plug[i], EndUseFuelType::Electricity,
                 EndUseCategoryType::InteriorEquipment);
    eu.addEndUse(0, EndUseFuelType::Electricity,
                 EndUseCategoryType::ExteriorEquipment);
    eu.addEndUse(Eelec_dhw[i], EndUseFuelType::Electricity,
                 EndUseCategoryType::WaterSystems);

    eu.addEndUse(Egas_ht[i], EndUseFuelType::Gas, EndUseCategoryType::Heating);
    eu.addEndUse(Egas_cl[i], EndUseFuelType::Gas, EndUseCategoryType::Cooling);
    eu.addEndUse(Egas_plug[i], EndUseFuelType::Gas,
                 EndUseCategoryType::InteriorEquipment);
    eu.addEndUse(Egas_dhw[i], EndUseFuelType::Gas,
                 EndUseCategoryType::WaterSystems);
    allResults.push_back(eu);
#endif
  }
  return allResults;
}
} // namespace openstudio::isomodel
