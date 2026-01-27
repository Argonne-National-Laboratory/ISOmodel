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

void MonthlyModel::calculateSunHours(const Matrix &m_mhEgh, Vector &v_hrs_sun_down_mo) {
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

Matrix MonthlyModel::buildSolarIrradianceMatrix(const WeatherData& weather) {
  PROFILE_FUNCTION();
  // Combine vertical surface radiation (msolar) and horizontal radiation
  // (mEgh) into one matrix (W/m2).
  Matrix m_I_sol(monthsInYear, numTotalSurfaces);

  // Access weather data via reference to avoid copy
  const Matrix &m_solar = weather.msolarRef();
  const Vector &v_mEgh = weather.mEghRef();

  for (unsigned int r = 0; r < m_I_sol.size1(); r++) {
    for (unsigned int c = 0; c < numVerticalSurfaces; c++) { // Vertical surfaces
      m_I_sol(r, c) = m_solar(r, c);
    }
    m_I_sol(r, numVerticalSurfaces) = v_mEgh[r]; // Roof/Horizontal surface
  }
  return m_I_sol;
}

Vector MonthlyModel::calculateGlazingSolarHeatGain(const Matrix &m_I_sol,
                                                   const Vector &v_win_A_sol, const Structure& structure) {
  PROFILE_FUNCTION();
  Vector v_win_phi_sol(monthsInYear);
  Vector v_win_SCF_frac(numTotalSurfaces);
  v_win_SCF_frac.assign(numTotalSurfaces, UNITY_FRACTION);

  // Compute the total solar heat gain for the glazing area.
  for (unsigned int i = 0; i < monthsInYear; i++) {
    double monthlySum = 0.0;
    for (unsigned int j = 0; j < numTotalSurfaces; j++) {
      monthlySum += structure.windowShadingCorrectionFactorRef()[j] *
                    v_win_SCF_frac[j] * v_win_A_sol[j] * m_I_sol(i, j);
    }
    v_win_phi_sol[i] = monthlySum;
  }
  return v_win_phi_sol;
}

Vector MonthlyModel::calculateOpaqueSolarHeatGain(const Matrix &m_I_sol,
                                                  const Vector &v_wall_A_sol,
                                                  const Vector &v_wall_phi_r) {
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

double MonthlyModel::calculatePeopleHeatGain(const Population &pop, bool occupied) {
  PROFILE_FUNCTION();
  if (occupied) {
    return pop.heatGainPerPerson() / pop.densityOccupied();
  } else {
    return pop.heatGainPerPerson() / pop.densityUnoccupied();
  }
}

double MonthlyModel::calculateApplianceHeatGain(const Building &building, bool occupied) {
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
                                                  double hours_fraction, double floor_area) {
  PROFILE_FUNCTION();
  // Q_illum_val is in kWh, structure.floorArea() in m2, hoursInYear in hours.
  // Result should be in W/m2.
  return Q_illum_val / floor_area / hoursInYear / hours_fraction *
         KWATTS_TO_WATTS;
}

double MonthlyModel::calculateAverageIlluminationHeatGain(
    double Q_illum_tot_yr, double floor_area) {
  PROFILE_FUNCTION();
  return Q_illum_tot_yr / floor_area / hoursInYear *
         KWATTS_TO_WATTS;
}



MonthlyModel::AnnualLightingHours
MonthlyModel::calculateAnnualLightingOperationalHours(const Lighting& lights, const Population& pop) {
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

  // OPTIMIZATION: Fused loop for solar fractions
  // Replaces 4 mults, 1 sum, and 3 divs (8 vectors)
  for (int i = 0; i < monthsInYear; ++i) {
      double Wgh_wk_day = v_Egh_day[i] * scheduleData.weekdayOccupiedMegaseconds[i];
      double Wgh_wk_nt = v_Egh_nt[i] * scheduleData.weekdayUnoccupiedMegaseconds[i];
      double Wgh_wke_day = v_Egh_day[i] * scheduleData.weekendOccupiedMegaseconds[i];
      double Wgh_wke_nt = v_Egh_nt[i] * scheduleData.weekendUnoccupiedMegaseconds[i];
      
      double Wgh_tot = Wgh_wk_day + Wgh_wk_nt + Wgh_wke_day + Wgh_wke_nt;
      // Use 0.0 if total is 0 to avoid division by zero/infinity
      double inv_Wgh_tot = (Wgh_tot > SMALL_EPSILON) ? 1.0 / Wgh_tot : 0.0;

      // frac_Egh_unocc_weekday_night
      simData.frac_Pgh_wk_nt[i] = Wgh_wk_nt * inv_Wgh_tot;
      simData.frac_Pgh_wke_day[i] = Wgh_wke_day * inv_Wgh_tot;
      simData.frac_Pgh_wke_nt[i] = Wgh_wke_nt * inv_Wgh_tot;
  }

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
  AnnualLightingHours annualHours = calculateAnnualLightingOperationalHours(lights, pop);

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
void MonthlyModel::envelopeCalculations(MonthlySimulationData &simData) const {
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

  // OPTIMIZATION: Inlined calculation of shading components to avoid struct return
  Vector v_win_ff(numTotalSurfaces);
  Vector v_win_F_shgl(numTotalSurfaces);

  for (int i = 0; i < numTotalSurfaces; i++) {
    v_win_ff[i] = UNITY_FRACTION - structure.win_ff();
    // Assign SDF based on pulldown value of 1, 2 or 3.
    double SDF = winSDFTable[((int)structure.windowShadingDeviceRef()[i]) - 1];
    // SDF fractions which include heat transfer set at 100% (UNITY_FRACTION) for now.
    v_win_F_shgl[i] = SDF * UNITY_FRACTION;
  }

  // Normal incidence solar energy transmittance which is SHGC in america.
  // Vector v_g_gln = structure.windowNormalIncidenceSolarEnergyTransmittance();
  const Vector &v_g_gln = structure.windowNormalIncidenceSolarEnergyTransmittanceRef();
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
  const Matrix m_I_sol = buildSolarIrradianceMatrix(*location.weather());

  // Combine vertical surface radiation (mosolar) and horizontal radiation
  // (mEgh) into one matrix (W/m2).
  printMatrix("m_I_sol", m_I_sol);

  // Compute the total solar heat gain for the glazing area.
  Vector v_win_phi_sol = calculateGlazingSolarHeatGain(m_I_sol, simData.v_win_A_sol, structure);

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

  // OPTIMIZATION: Replaced chained vector math with a loop to avoid temporary allocations.
  Vector v_wall_phi_r(numTotalSurfaces);
  for (int i = 0; i < numTotalSurfaces; ++i) {
    v_wall_phi_r[i] = simData.v_wall_R_sc[i] * simData.v_wall_U[i] *
                      simData.v_wall_A[i] * simData.v_win_hr[i] * theta_er[i];
  }

  // Total solar heat gain for opaque area.
  Vector v_wall_phi_sol = calculateOpaqueSolarHeatGain(m_I_sol, simData.v_wall_A_sol, v_wall_phi_r);

  printVector("v_wall_phi_r", v_wall_phi_r);
  printVector("v_win_phi_sol", v_win_phi_sol);
  printVector("v_wall_phi_sol", v_wall_phi_sol);

  // Total envelope solar heat gain (W).
  // OPTIMIZATION: Replaced chained vector math with a loop to avoid temporary allocations.
  for (int i = 0; i < monthsInYear; ++i) {
    double phi_sol = v_win_phi_sol[i] + v_wall_phi_sol[i];
    simData.v_E_sol[i] = phi_sol * megasecondsInMonth[i];
    if (debugIsoModelSimulation) { std::cout << "v_phi_sol[" << i << "]=" << phi_sol << std::endl; }
  }
}

/**
 * Compute internal heat gains and losses.
 */
void MonthlyModel::calculateInternalGainComponents(MonthlySimulationData &simData) const {
  PROFILE_FUNCTION();
  // Internal heat gains from people (W/m2).
  double phi_int_occ = calculatePeopleHeatGain(pop, true);
  double phi_int_unocc = calculatePeopleHeatGain(pop, false);
  simData.phi_int_avg = std::lerp(phi_int_unocc, phi_int_occ, simData.scheduleData.frac_hrs_wk_day);

  // Internal heat gain from appliances (W/m2).
  double phi_plug_occ = calculateApplianceHeatGain(building, true);
  double phi_plug_unocc = calculateApplianceHeatGain(building, false);
  simData.phi_plug_avg = std::lerp(phi_plug_unocc, phi_plug_occ, simData.scheduleData.frac_hrs_wk_day);

  // Internal heat gain from illumination (W/m2).
  double phi_illum_occ = calculateIlluminationHeatGain(simData.Q_illum_occ, simData.scheduleData.frac_hrs_wk_day, structure.floorArea());
  double phi_illum_unocc = calculateIlluminationHeatGain(simData.Q_illum_unocc, (UNITY_FRACTION - simData.scheduleData.frac_hrs_wk_day), structure.floorArea());
  simData.phi_illum_avg = calculateAverageIlluminationHeatGain(simData.Q_illum_tot_yr, structure.floorArea());

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

  // Total internal heat gain (W).
  simData.phi_I_tot =
      (simData.phi_int_avg + simData.phi_plug_avg + simData.phi_illum_avg) * structure.floorArea();
}

/**
 * Compute unoccupied heat gain.
 */
void MonthlyModel::unoccupiedHeatGain(MonthlySimulationData &simData) const {
  PROFILE_FUNCTION();

  // Ensure vectors are sized
  simData.v_P_tot_wk_nt.resize(monthsInYear);
  simData.v_P_tot_wke_day.resize(monthsInYear);
  simData.v_P_tot_wke_nt.resize(monthsInYear);

  double floor_area = structure.floorArea();
  double phi_int_wk_nt = simData.phi_int_wk_nt;
  double phi_int_wke_day = simData.phi_int_wke_day;
  double phi_int_wke_nt = simData.phi_int_wke_nt;

  // OPTIMIZATION: Fused loop for unoccupied heat gain
  for (int i = 0; i < monthsInYear; ++i) {
      // Week Night
      double ms_wk_nt = simData.scheduleData.weekdayUnoccupiedMegaseconds[i];
      double W_int_wk_nt = ms_wk_nt * phi_int_wk_nt * floor_area;
      double W_sol_wk_nt = simData.v_E_sol[i] * simData.frac_Pgh_wk_nt[i];
      simData.v_P_tot_wk_nt[i] = (ms_wk_nt > 0) ? (W_int_wk_nt + W_sol_wk_nt) / ms_wk_nt : 0.0;

      // Weekend Day
      double ms_wke_day = simData.scheduleData.weekendOccupiedMegaseconds[i];
      double W_int_wke_day = ms_wke_day * phi_int_wke_day * floor_area;
      double W_sol_wke_day = simData.v_E_sol[i] * simData.frac_Pgh_wke_day[i];
      simData.v_P_tot_wke_day[i] = (ms_wke_day > 0) ? (W_int_wke_day + W_sol_wke_day) / ms_wke_day : 0.0;

      // Weekend Night
      double ms_wke_nt = simData.scheduleData.weekendUnoccupiedMegaseconds[i];
      double W_int_wke_nt = ms_wke_nt * phi_int_wke_nt * floor_area;
      double W_sol_wke_nt = simData.v_E_sol[i] * simData.frac_Pgh_wke_nt[i];
      simData.v_P_tot_wke_nt[i] = (ms_wke_nt > 0) ? (W_int_wke_nt + W_sol_wke_nt) / ms_wke_nt : 0.0;
  }

  if (DEBUG_ISO_MODEL_SIMULATION) {
    printVector("v_P_tot_wk_nt", simData.v_P_tot_wk_nt);
    printVector("v_P_tot_wke_day", simData.v_P_tot_wke_day);
    printVector("v_P_tot_wke_nt", simData.v_P_tot_wke_nt);
  }
}

double MonthlyModel::calculateBEMAdjustment(const Building& building) {
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
    const Vector &v_decay_start_base, const Vector &v_limit_start_col0, double tset_unocc,
    double tau, const Vector &v_ti, const Vector &v_P_tot_wk_nt, const Vector &v_P_tot_wke_day,
    const Vector &v_P_tot_wke_nt, const Vector &v_Tdbt_nt, const Vector &v_Tdbt_day, double H_tot,
    Vector &v_wke_avg, Vector &v_wk_nt) {

  // OPTIMIZATION: Removed matrix allocations.
  // The calculation iterates through 5 time steps (columns) for each month (rows).
  // We can accumulate the average and track the current temperature state per month.

  std::fill(v_wke_avg.begin(), v_wke_avg.end(), 0.0);
  Vector v_current_T = v_decay_start_base; // Tracks the temperature at the start of the decay phase

  // Steps: 0=wk_nt, 1=wke_day, 2=wke_nt, 3=wke_day, 4=wke_nt
  for (int step = 0; step < 5; ++step) {
    double ti = v_ti[step];

    // Handle zero duration steps (e.g. if occupied 24 hours)
    if (ti < 1e-6) {
        for (int m = 0; m < monthsInYear; ++m) {
             double limit;
             if (step == 0) limit = v_limit_start_col0[m];
             else limit = std::max(v_current_T[m], tset_unocc);
             
             // Temp stays at limit
             v_wke_avg[m] += limit;
             if (step == 1) v_wk_nt[m] = limit;
             if (step < 4) v_current_T[m] = limit;
        }
        continue;
    }

    double exp_val = std::exp(-ti / tau);
    double inv_ti_tau = tau / ti;
    double one_minus_exp = 1.0 - exp_val;

    for (int m = 0; m < monthsInYear; ++m) {
      // Determine inputs for this step
      double P_tot = (step == 0) ? v_P_tot_wk_nt[m]
                     : (step % 2 != 0) ? v_P_tot_wke_day[m]
                                       : v_P_tot_wke_nt[m];
      double Te = (step % 2 != 0) ? v_Tdbt_day[m] : v_Tdbt_nt[m];
      double dT = P_tot / H_tot;

      // 1. Apply Limits (M_Limit)
      double limit;
      if (step == 0) {
        limit = v_limit_start_col0[m];
      } else {
        // Limit is max of previous decay end temp and setpoint
        limit = std::max(v_current_T[m], tset_unocc);
      }

      // 2. Calculate Average (M_Avg)
      double term = (limit - Te - dT);
      double avg = inv_ti_tau * term * one_minus_exp + Te + dT;
      avg = std::max(avg, tset_unocc);

      v_wke_avg[m] += avg;

      if (step == 1) {
        v_wk_nt[m] = avg;
      }

      // 3. Calculate Decay for next step (M_Decay)
      if (step < 4) {
        v_current_T[m] = term * exp_val + Te + dT;
      }
    }
  }

  // Finalize average
  for (int m = 0; m < monthsInYear; ++m) {
    v_wke_avg[m] /= 5.0;
  }
}

/*
 * Calculate interior temp.
 */
void MonthlyModel::calculateInteriorTemperatures(MonthlySimulationData &simData) const {
  PROFILE_FUNCTION();
  // Set the temp differential from the interior heating/cooling setpoint
  // based on the BEM type. An advanced BEM has the effect of reducing the
  // effective heating temp and raising the effective cooling temp during
  // times of control (i.e. during occupancy).
  double T_adj = calculateBEMAdjustment(building);

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

  if (DEBUG_ISO_MODEL_SIMULATION) {
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
                                 simData.tau, v_ti, simData.v_P_tot_wk_nt, simData.v_P_tot_wke_day,
                                 simData.v_P_tot_wke_nt, simData.v_Tdbt_nt, simData.v_Tdbt_day,
                                 H_tot, v_Th_wke_avg, v_Th_wk_nt);
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
                                 simData.tau, v_ti, simData.v_P_tot_wk_nt, simData.v_P_tot_wke_day,
                                 simData.v_P_tot_wke_nt, simData.v_Tdbt_nt, simData.v_Tdbt_day,
                                 H_tot, v_Tc_wke_avg, v_Tc_wk_nt);
  }

  if (DEBUG_ISO_MODEL_SIMULATION) {
    printVector("v_Tc_wk_day", v_Tc_wk_day);
    printVector("v_Tc_wk_nt", v_Tc_wk_nt);
    printVector("v_Tc_wke_avg", v_Tc_wke_avg);
  }

  // OPTIMIZATION: Fused loop for weekly average temperatures
  // Replaces 10 vector operations (mults and sums)
  for (int i = 0; i < monthsInYear; i++) {
    double Th_wk_avg = v_Th_wk_day[i] * simData.scheduleData.frac_hrs_wk_day +
                       v_Th_wk_nt[i] * simData.scheduleData.frac_hrs_wk_nt +
                       v_Th_wke_avg[i] * simData.scheduleData.frac_hrs_wke_tot;
    
    double Tc_wk_avg = v_Tc_wk_day[i] * simData.scheduleData.frac_hrs_wk_day +
                       v_Tc_wk_nt[i] * simData.scheduleData.frac_hrs_wk_nt +
                       v_Tc_wke_avg[i] * simData.scheduleData.frac_hrs_wke_tot;

    simData.v_Th_avg[i] = std::min(Th_wk_avg, ht_tset_ctrl);
    simData.v_Tc_avg[i] = std::min(Tc_wk_avg, cl_tset_ctrl);
  }
}

/**
 * Calculate required energy for mechanical ventilation based on source EN ISO
 * 13789 C.3, C.5 and EN 15242:2007 6.7 and EN ISO 13790 Sec 9.2.
 */
void MonthlyModel::calculateVentilation(MonthlySimulationData &simData) const {
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

  double mve_init = ventilation.ventType() == 3 ? 0 : (vent_op_frac * qv_supp * vent_outdoor_frac * (1 - vent_ht_recov));

  // OPTIMIZATION: Fused vector operations into a single loop to avoid temporary allocations.
  double stack_exp = ventilation.stack_exp();
  double stack_coeff_Q4 = ventilation.stack_coeff() * v_Q4pa;
  double wind_exp = ventilation.wind_exp();
  double wind_coeff = ventilation.wind_coeff();
  double dCp_terrain = ventilation.dCp() * location.terrain();

  for (int i = 0; i < monthsInYear; ++i) {
      // Stack Effect Heating
      double dbtDiff_ht = std::abs(v_mdbt[i] - simData.v_Th_avg[i]);
      double qv_stack_ht = std::max(std::pow(dbtDiff_ht * h_stack, stack_exp) * stack_coeff_Q4, MIN_INFILTRATION_FLOW);

      // Stack Effect Cooling
      double dbtDiff_cl = std::abs(v_mdbt[i] - simData.v_Tc_avg[i]);
      double qv_stack_cl = std::max(std::pow(dbtDiff_cl * h_stack, stack_exp) * stack_coeff_Q4, MIN_INFILTRATION_FLOW);

      // Wind Effect
      double wind_sq = v_mwind[i] * v_mwind[i];
      double qv_wind = std::pow(wind_sq * dCp_terrain, wind_exp) * v_Q4pa * wind_coeff;

      // Combined (Superposition)
      double qv_ht_max = std::max(qv_stack_ht, qv_wind);
      double qv_cl_max = std::max(qv_stack_cl, qv_wind);

      double qv_sw_ht = qv_ht_max + (qv_stack_ht * qv_wind) * n_sw_coeff / v_Q4pa;
      double qv_sw_cl = qv_cl_max + (qv_stack_cl * qv_wind) * n_sw_coeff / v_Q4pa;

      // Infiltration
      double qv_inf_ht = qv_sw_ht + std::max(0.0, -qv_diff);
      double qv_inf_cl = qv_sw_cl + std::max(0.0, -qv_diff);

      // Total
      double qve_ht = qv_inf_ht + mve_init;
      double qve_cl = qv_inf_cl + mve_init;

      // Hve
      simData.v_Hve_ht[i] = qve_ht * rhoCpAirWh;
      simData.v_Hve_cl[i] = qve_cl * rhoCpAirWh;
  }
}

/**
 * Compute monthly heating and cooling demand.
 */
void MonthlyModel::calculateHeatingAndCoolingNeeds(MonthlySimulationData &simData) const {
  PROFILE_FUNCTION();
  // Optimization: Cache weather reference
  const Vector &v_mdbt = location.weather()->mdbtRef();

  // Building heating dimensionless constant.
  double a_H = heating.a_H0() + simData.tau / heating.tau_H0();

  // Pre-calculate constants for Air Volumes
  double T_sup_ht = heating.temperatureSetPointOccupied() + heating.dT_supp_ht();
  double T_sup_cl = cooling.temperatureSetPointOccupied() - cooling.dT_supp_cl();

  // Pre-calculate constants for Total Air Flow
  double min_flow_rate = ventilation.supplyRate() * simData.scheduleData.frac_hrs_wk_day * (MEGASECONDS_TO_SECONDS / LITERS_TO_M3);

  // Pre-calculate constants for Fan Energy
  double fan_power_factor = ventilation.fanPower() / KJ_TO_MJ;
  double area_kWh_factor = structure.floorArea() * kWh2MJ;

  // Initialize sums
  simData.Qneed_ht_yr = 0.0;
  simData.Qneed_cl_yr = 0.0;

  // Ensure vectors are sized
  simData.v_Qneed_ht.resize(monthsInYear);
  simData.v_Qneed_cl.resize(monthsInYear);
  simData.v_Vair_ht.resize(monthsInYear);
  simData.v_Vair_cl.resize(monthsInYear);
  simData.v_Vair_tot.resize(monthsInYear);
  simData.v_Qfan_tot.resize(monthsInYear);

  // OPTIMIZATION: Fused loop for Heating and Cooling Needs
  // Calculates Gains, Losses, Gamma, Eta, and Qneed in one pass
  for (int i = 0; i < monthsInYear; ++i) {
    // 1. Calculate Gains and Losses
    double tot_mo_ht_gain = (simData.phi_I_tot * megasecondsInMonth[i]) + simData.v_E_sol[i];

    double Th_avg_minus_mdbt = simData.v_Th_avg[i] - v_mdbt[i];
    double QT_ht = Th_avg_minus_mdbt * megasecondsInMonth[i] * simData.H_tr;
    double QV_ht = simData.v_Hve_ht[i] * structure.floorArea() * Th_avg_minus_mdbt * megasecondsInMonth[i];
    double Qtot_ht = QT_ht + QV_ht;

    double Tc_avg_minus_mdbt = simData.v_Tc_avg[i] - v_mdbt[i];
    double QT_cl = Tc_avg_minus_mdbt * simData.H_tr * megasecondsInMonth[i];
    double QV_cl = simData.v_Hve_cl[i] * structure.floorArea() * Tc_avg_minus_mdbt * megasecondsInMonth[i];
    double Qtot_cl = QT_cl + QV_cl;

    // 2. Heating Calculations
    double gamma_H_ht = tot_mo_ht_gain / (Qtot_ht + SMALL_EPSILON);
    double eta_g_H;
    if (gamma_H_ht > 0) {
        double num = std::pow(gamma_H_ht, a_H);
        eta_g_H = (UNITY_FRACTION - num) / (UNITY_FRACTION - num * gamma_H_ht);
    } else {
        eta_g_H = UNITY_FRACTION / (gamma_H_ht + SMALL_EPSILON);
    }
    
    simData.v_Qneed_ht[i] = Qtot_ht - eta_g_H * tot_mo_ht_gain;
    simData.Qneed_ht_yr += simData.v_Qneed_ht[i];

    // 3. Cooling Calculations
    double gamma_H_cl = Qtot_cl / (tot_mo_ht_gain + SMALL_EPSILON);
    double eta_g_CL;
    if (gamma_H_cl > 0) {
        double num = std::pow(gamma_H_cl, a_H);
        eta_g_CL = (UNITY_FRACTION - num) / (UNITY_FRACTION - num * gamma_H_cl);
    } else {
        eta_g_CL = UNITY_FRACTION / (gamma_H_cl + SMALL_EPSILON);
    }

    simData.v_Qneed_cl[i] = tot_mo_ht_gain - eta_g_CL * Qtot_cl;
    simData.Qneed_cl_yr += simData.v_Qneed_cl[i];

    // 4. Calculate Air Volumes
    double denominator_ht = ((T_sup_ht - simData.v_Th_avg[i]) * rhoCpAir) + SMALL_EPSILON;
    simData.v_Vair_ht[i] = simData.v_Qneed_ht[i] / denominator_ht;

    double denominator_cl = ((simData.v_Tc_avg[i] - T_sup_cl) * rhoCpAir) + SMALL_EPSILON;
    simData.v_Vair_cl[i] = simData.v_Qneed_cl[i] / denominator_cl;

    // 5. Calculate Total Air Flow
    double sum_Vair = simData.v_Vair_ht[i] + simData.v_Vair_cl[i];
    double min_flow_month = megasecondsInMonth[i] * min_flow_rate;
    simData.v_Vair_tot[i] = std::max(sum_Vair, min_flow_month);

    // 6. Calculate Fan Energy
    simData.v_Qfan_tot[i] = (simData.v_Vair_tot[i] * fan_power_factor) / area_kWh_factor;
  }

  printVector("v_Vair_ht", simData.v_Vair_ht);
  printVector("v_Vair_cl", simData.v_Vair_cl);
  printVector("v_Vair_tot", simData.v_Vair_tot);
  if (DEBUG_ISO_MODEL_SIMULATION) {
    // Note: fanEnergy is no longer available here, but we can print the inputs and output
    printVector("v_Vair_tot (input to fan calc)", simData.v_Vair_tot);
    std::cout << "ventilation.fanPower() = " << ventilation.fanPower()
              << std::endl;
    std::cout << "structure.floorArea() = " << structure.floorArea()
              << std::endl;
    printVector("v_Qfan_tot (output from fan calc)", simData.v_Qfan_tot);
  }
}

/**
 * HVAC systems calculations.
 */
void MonthlyModel::calculateHVACEnergyUse(MonthlySimulationData &simData) const {
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
  double f_dem_ht = std::max(simData.Qneed_ht_yr / (simData.Qneed_cl_yr + simData.Qneed_ht_yr), MIN_DEMAND_FRACTION);
  // Fraction of yearly cooling demand.
  double f_dem_cl = std::max((1.0 - f_dem_ht), MIN_DEMAND_FRACTION);

  // Overall distribution efficiency for heating.
  double eta_dist_ht = 1.0 / (1.0 + a_ht_loss + f_waste / f_dem_ht);
  // Overall distrubtion efficiency for cooling.
  double eta_dist_cl = 1.0 / (1.0 + a_cl_loss + f_waste / f_dem_cl);

  // Pre-calc factors for loop
  double loss_factor_ht = (1.0 - eta_dist_ht) / eta_dist_ht;
  double loss_factor_cl = (1.0 - eta_dist_cl) / eta_dist_cl;
  double inv_heat_eff = 1.0 / (heating.efficiency() + SMALL_EPSILON);
  double inv_IEER = 1.0 / (IEER + SMALL_EPSILON);
  double dc_elec_factor = 1.0 / (cooling.eta_DC_COP() * cooling.eta_DC_network());
  double dc_abs_factor = 1.0 / cooling.eta_DC_COP_abs();
  double one_minus_eta_DC_frac_abs = 1.0 - cooling.eta_DC_frac_abs();
  double one_minus_frac_DC_free = 1.0 - cooling.frac_DC_free();
  double dh_factor = 1.0 / (heating.eta_DH_sys() * heating.eta_DH_network());
  double one_minus_frac_DH_free = 1.0 - heating.frac_DH_free();

  bool dh_yes = (heating.DH_YesNo() == 1);
  bool dc_yes = (cooling.DC_YesNo() == 1);
  bool heat_is_elec = (heating.energyType() == 1);

  // OPTIMIZATION: Fused loop for HVAC energy use
  // Replaces multiple vector operations and helper function calls
  simData.v_Qht_sys.resize(monthsInYear);
  simData.v_Qht_DH.resize(monthsInYear);
  simData.v_Qcl_sys.resize(monthsInYear);
  simData.v_Qcool_DC.resize(monthsInYear);
  simData.v_Qcl_elec_tot.resize(monthsInYear);
  simData.v_Qcl_gas_tot.resize(monthsInYear);
  simData.v_Qelec_ht.resize(monthsInYear);
  simData.v_Qgas_ht.resize(monthsInYear);

  for (int i = 0; i < monthsInYear; ++i) {
      double Qloss_ht_dist = simData.v_Qneed_ht[i] * loss_factor_ht;
      double Qloss_cl_dist = simData.v_Qneed_cl[i] * loss_factor_cl;

      // Heating System Loads
      if (dh_yes) {
          simData.v_Qht_DH[i] = simData.v_Qneed_ht[i] + Qloss_ht_dist;
          simData.v_Qht_sys[i] = 0.0;
      } else {
          simData.v_Qht_sys[i] = (Qloss_ht_dist + simData.v_Qneed_ht[i]) * inv_heat_eff;
          simData.v_Qht_DH[i] = 0.0;
      }

      // Cooling System Loads
      if (dc_yes) {
          simData.v_Qcool_DC[i] = simData.v_Qneed_cl[i] + Qloss_cl_dist;
          simData.v_Qcl_sys[i] = 0.0;
      } else {
          simData.v_Qcl_sys[i] = (Qloss_cl_dist + simData.v_Qneed_cl[i]) * inv_IEER;
          simData.v_Qcool_DC[i] = 0.0;
      }

      double Qcl_DC_elec = simData.v_Qcool_DC[i] * one_minus_eta_DC_frac_abs * dc_elec_factor;
      double Qcl_DC_abs = simData.v_Qcool_DC[i] * one_minus_frac_DC_free * dc_abs_factor;
      double Qht_DH_total = simData.v_Qht_DH[i] * one_minus_frac_DH_free * dh_factor;

      simData.v_Qcl_elec_tot[i] = simData.v_Qcl_sys[i] + Qcl_DC_elec;
      simData.v_Qcl_gas_tot[i] = Qcl_DC_abs;

      if (heat_is_elec) {
          simData.v_Qelec_ht[i] = simData.v_Qht_sys[i];
          simData.v_Qgas_ht[i] = Qht_DH_total;
      } else {
          simData.v_Qelec_ht[i] = 0.0;
          simData.v_Qgas_ht[i] = simData.v_Qht_sys[i] + Qht_DH_total;
      }
  }

  printVector("v_Qelec_ht", simData.v_Qelec_ht);
  printVector("v_Qgas_ht", simData.v_Qgas_ht);
}

/**
 * Calculate energy for pumps used in the heating/cooling systems.
 * References: EPA NR 6.9.7.1 and 6.9.7.2, EN 15243.
 */
void MonthlyModel::calculatePumpEnergy(MonthlySimulationData &simData) const {
  PROFILE_FUNCTION();
  // TODO: The current implementation is wrong. It either needs to be revised to
  // be more like the hourly implementation where the pump energy is multiplied
  // by the amount of time the pumps are actually on or
  // heating.E_pumps()/cooling.E_pumps() needs to be expressed in terms of the
  // heating/cooling delivered so that the pump energy can be determined by
  // multiplying it by the heating/cooling delivered. Both methods have
  // challenges, which is why they are not yet implements. Until then, consider
  // the monthly pump values unreliable. BAA@2015-07-15.

  // OPTIMIZATION: Fused loop for Pump Energy
  // Replaces calculatePumpEnergyForMode and multiple vector ops
  double floor_area = structure.floorArea();
  double ht_pump_factor = heating.E_pumps() * heating.pumpControlReduction() * floor_area;
  double cl_pump_factor = cooling.E_pumps() * cooling.pumpControlReduction() * floor_area;

  // Calculate total annual pump energy potential (if running continuously)
  double Q_pumps_yr_ht_base = 0.0;
  double Q_pumps_yr_cl_base = 0.0;
  for(int i=0; i<monthsInYear; ++i) {
      double val = megasecondsInMonth[i];
      Q_pumps_yr_ht_base += val * ht_pump_factor;
      Q_pumps_yr_cl_base += val * cl_pump_factor;
  }

  // Calculate fractions
  double frac_ht_total = 0.0;
  double frac_cl_total = 0.0;
  Vector v_frac_ht(monthsInYear);
  Vector v_frac_cl(monthsInYear);

  for(int i=0; i<monthsInYear; ++i) {
      double total_need = simData.v_Qneed_ht[i] + simData.v_Qneed_cl[i];
      if (total_need > 0) {
          v_frac_ht[i] = simData.v_Qneed_ht[i] / total_need;
          v_frac_cl[i] = simData.v_Qneed_cl[i] / total_need;
      } else {
          v_frac_ht[i] = 0.0;
          v_frac_cl[i] = 0.0;
      }
      frac_ht_total += v_frac_ht[i];
      frac_cl_total += v_frac_cl[i];
  }

  simData.v_Q_pump_tot.resize(monthsInYear);
  for(int i=0; i<monthsInYear; ++i) {
      double Q_pumps_ht = (frac_ht_total > 0) ? (v_frac_ht[i] * Q_pumps_yr_ht_base / frac_ht_total) : 0.0;
      double Q_pumps_cl = (frac_cl_total > 0) ? (v_frac_cl[i] * Q_pumps_yr_cl_base / frac_cl_total) : 0.0;
      simData.v_Q_pump_tot[i] = Q_pumps_ht + Q_pumps_cl;
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
void MonthlyModel::calculateHeatedWaterEnergy(MonthlySimulationData &simData) const {
  PROFILE_FUNCTION();
  // Energy from solar energy hot water collectors - not included yet
  // Vector v_Q_dhw_solar(monthsInYear, 0.0);

  // Total annual energy demand required for heating DHW (MJ/yr).
  double Q_dhw_yr = heating.hotWaterDemand() *
                    (heating.dhw_tset() - heating.dhw_tsupply()) * rhoCpWater;

  // OPTIMIZATION: Fused loop for DHW
  double inv_daysInYear = 1.0 / daysInYear;
  double inv_dist_eff = 1.0 / heating.hotWaterDistributionEfficiency();
  double inv_sys_eff = 1.0 / heating.hotWaterSystemEfficiency();
  double inv_kWh2MJ = 1.0 / kWh2MJ;

  Vector v_Q_dhw_need(monthsInYear);
  for(int i=0; i<monthsInYear; ++i) {
      double monthlyDemand = daysInMonth[i] * Q_dhw_yr;
      double frac_MonthlyDemand_yr = monthlyDemand * inv_daysInYear;
      double Qe_demand = frac_MonthlyDemand_yr * inv_dist_eff;
      double Q_dhw_demand = Qe_demand * inv_kWh2MJ;
      v_Q_dhw_need[i] = std::max(0.0, Q_dhw_demand * inv_sys_eff);
  }

  if (DEBUG_ISO_MODEL_SIMULATION) {
    printVector("v_Q_dhw_need", v_Q_dhw_need);
  }

  if (heating.hotWaterEnergyType() == 1) {
    simData.v_Q_dhw_elec = v_Q_dhw_need;
    simData.v_Q_dhw_gas.assign(v_Q_dhw_need.size(), 0.0);
  } else {
    simData.v_Q_dhw_gas = v_Q_dhw_need;
    simData.v_Q_dhw_elec.assign(v_Q_dhw_need.size(), 0.0);
  }

  if (DEBUG_ISO_MODEL_SIMULATION) {
    printVector("v_Q_dhw_gas", simData.v_Q_dhw_gas);
    printVector("v_Q_dhw_elec", simData.v_Q_dhw_elec);
  }
}

std::vector<EndUses> MonthlyModel::simulate() const {
  PROFILE_FUNCTION();

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
  if (DEBUG_ISO_MODEL_SIMULATION) {
    printVector("v_hrs_sun_down_mo", simData.v_hrs_sun_down_mo);
    printVector("frac_Pgh_wk_nt", simData.frac_Pgh_wk_nt);
    printVector("frac_Pgh_wke_day", simData.frac_Pgh_wke_day);
    printVector("frac_Pgh_wke_nt", simData.frac_Pgh_wke_nt);
    printVector("v_Tdbt_nt", simData.v_Tdbt_nt);
    printVector("v_Tdbt_day", simData.v_Tdbt_day);

    std::cout << std::endl << "lightingEnergyUse: " << std::endl;
  }
  lightingEnergyUse(simData);

  if (DEBUG_ISO_MODEL_SIMULATION) {
    std::cout << "Q_illum_occ: " << simData.Q_illum_occ << std::endl;
    std::cout << "Q_illum_unocc: " << simData.Q_illum_unocc << std::endl;
    std::cout << "Q_illum_unocc: " << simData.Q_illum_unocc << std::endl;
    printVector("v_Q_illum_tot", simData.v_Q_illum_tot);
    printVector("v_Q_illum_ext_tot", simData.v_Q_illum_ext_tot);

    std::cout << std::endl
              << "envelopeCalculations: " << std::endl; /*
v_wall_A = structure.wallArea();
v_win_A = structure.windowArea();
v_wall_U = structure.wallUniform();
Vector v_win_U = structure.windowUniform();*/
    printVector("structure.wallArea()", structure.wallArea());
    printVector("structure.windowArea()", structure.windowArea());
    printVector("structure.wallUniform()", structure.wallUniform());
    printVector("structure.windowUniform()", structure.windowUniform());
  }
  envelopeCalculations(simData);
  if (DEBUG_ISO_MODEL_SIMULATION) {
    std::cout << "H_tr: " << simData.H_tr << std::endl;
    printVector("v_win_A", simData.v_win_A);
    printVector("v_wall_emiss", simData.v_wall_emiss);
    printVector("v_wall_alpha_sc", simData.v_wall_alpha_sc);
    printVector("v_wall_U", simData.v_wall_U);
    printVector("v_wall_A", simData.v_wall_A);

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
  if (DEBUG_ISO_MODEL_SIMULATION) {
    printVector("v_E_sol", simData.v_E_sol);

    std::cout << std::endl << "calculateInternalGainComponents: " << std::endl;
  }
  calculateInternalGainComponents(simData);
  if (DEBUG_ISO_MODEL_SIMULATION) {
    std::cout << "phi_int_avg: " << simData.phi_int_avg << std::endl;
    std::cout << "phi_plug_avg: " << simData.phi_plug_avg << std::endl;
    std::cout << "phi_illum_avg: " << simData.phi_illum_avg << std::endl;
    std::cout << "phi_int_wke_nt: " << simData.phi_int_wke_nt << std::endl;
    std::cout << "phi_int_wke_day: " << simData.phi_int_wke_day << std::endl;
    std::cout << "phi_int_wk_nt: " << simData.phi_int_wk_nt << std::endl;

    std::cout << std::endl << "calculateTotalInternalGain: " << std::endl;

    std::cout << std::endl << "unoccupiedHeatGain: " << std::endl;
  }
  unoccupiedHeatGain(simData);
  if (DEBUG_ISO_MODEL_SIMULATION) {
    std::cout << std::endl << "calculateInteriorTemperatures: " << std::endl;
  }
  calculateInteriorTemperatures(simData);

  if (DEBUG_ISO_MODEL_SIMULATION) {
    std::cout << "tau: " << simData.tau << std::endl;
    printVector("v_Th_avg", simData.v_Th_avg);
    printVector("v_Tc_avg", simData.v_Tc_avg);

    std::cout << std::endl << "calculateVentilation: " << std::endl;
  }
  calculateVentilation(simData);
  if (DEBUG_ISO_MODEL_SIMULATION) {
    printVector("v_Hve_ht", simData.v_Hve_ht);
    printVector("v_Hve_cl", simData.v_Hve_cl);

    std::cout << std::endl << "calculateHeatingAndCoolingNeeds: " << std::endl;
  }
  calculateHeatingAndCoolingNeeds(simData);
  if (DEBUG_ISO_MODEL_SIMULATION) {
    std::cout << "Qneed_ht_yr: " << simData.Qneed_ht_yr << std::endl;
    std::cout << "Qneed_cl_yr: " << simData.Qneed_cl_yr << std::endl;
    printVector("v_Qfan_tot", simData.v_Qfan_tot);

    std::cout << std::endl << "calculateHVACEnergyUse: " << std::endl;
  }
  calculateHVACEnergyUse(simData);
  if (DEBUG_ISO_MODEL_SIMULATION) {
    printVector("v_Qelec_ht", simData.v_Qelec_ht);
    printVector("v_Qgas_ht", simData.v_Qgas_ht);
    printVector("v_Qcl_elec_tot", simData.v_Qcl_elec_tot);
    printVector("v_Qcl_gas_tot", simData.v_Qcl_gas_tot);

    std::cout << std::endl << "calculatePumpEnergy: " << std::endl;
  }
  calculatePumpEnergy(simData);
  if (DEBUG_ISO_MODEL_SIMULATION) {
    printVector("v_Q_pump_tot", simData.v_Q_pump_tot);
    std::cout << std::endl << "energyGeneration: " << std::endl;
  }
  energyGeneration();
  if (DEBUG_ISO_MODEL_SIMULATION) {
    std::cout << std::endl << "calculateHeatedWaterEnergy: " << std::endl;
  }
  calculateHeatedWaterEnergy(simData);
  if (DEBUG_ISO_MODEL_SIMULATION) {
    printVector("v_Q_dhw_elec", simData.v_Q_dhw_elec);
    printVector("v_Q_dhw_gas", simData.v_Q_dhw_gas);
  }

  return outputGeneration(simData);
}
std::vector<EndUses>
MonthlyModel::outputGeneration(const MonthlySimulationData &simData) const {
  PROFILE_FUNCTION();
  std::vector<EndUses> allResults;

  // OPTIMIZATION: Inlined plug loads calculation
  double frac_hrs_wk_day = simData.scheduleData.frac_hrs_wk_day;
  double E_plug_elec_avg =
      building.electricApplianceHeatGainOccupied() * frac_hrs_wk_day +
      building.electricApplianceHeatGainUnoccupied() * (UNITY_FRACTION - frac_hrs_wk_day);
  double E_plug_gas_avg =
      building.gasApplianceHeatGainOccupied() * frac_hrs_wk_day +
      building.gasApplianceHeatGainUnoccupied() * (UNITY_FRACTION - frac_hrs_wk_day);

  Vector v_Q_plug_elec = mult(hoursInMonth, E_plug_elec_avg * W2kW, monthsInYear);
  Vector v_Q_plug_gas = mult(hoursInMonth, E_plug_gas_avg * W2kW, monthsInYear);

  if (DEBUG_ISO_MODEL_SIMULATION) {
    printVector("v_Q_plug_elec", v_Q_plug_elec);
    printVector("v_Q_plug_gas", v_Q_plug_gas);
  }

  // OPTIMIZATION: Pre-calculate factors for loop
  double floorArea = structure.floorArea();
  double invFloorArea = (floorArea > 0.0) ? 1.0 / floorArea : 0.0;
  double energy_factor = invFloorArea / kWh2MJ; // MJ -> kWh/m2

  if (DEBUG_ISO_MODEL_SIMULATION) {
    // Debug prints for intermediate values (reconstructing what would have been calculated)
    Vector Eelec_cl = mult(simData.v_Qcl_elec_tot, energy_factor);
    Vector Eelec_pump = mult(simData.v_Q_pump_tot, energy_factor);
    
    printVector("v_Qcl_elec_tot", simData.v_Qcl_elec_tot);
    printVector("v_Q_pump_tot", simData.v_Q_pump_tot);
    printVector("Eelec_cl", Eelec_cl);
    printVector("Eelec_pump", Eelec_pump);
    std::cout << "floorArea: " << structure.floorArea() << std::endl;
  }

  allResults.reserve(monthsInYear);

  for (int i = 0; i < monthsInYear; i++) {
    // OPTIMIZATION: Calculate final values directly in the loop
    // Electric (kWh/m2)
    double Eelec_ht = simData.v_Qelec_ht[i] * energy_factor;
    double Eelec_cl = simData.v_Qcl_elec_tot[i] * energy_factor;
    double Eelec_int_lt = simData.v_Q_illum_tot[i] * invFloorArea;
    double Eelec_ext_lt = simData.v_Q_illum_ext_tot[i] * invFloorArea;
    double Eelec_fan = simData.v_Qfan_tot[i]; // Already kWh/m2
    double Eelec_pump = simData.v_Q_pump_tot[i] * energy_factor;
    double Eelec_plug = v_Q_plug_elec[i]; // Already kWh/m2
    double Eelec_dhw = simData.v_Q_dhw_elec[i] * invFloorArea; // Already kWh

    // Gas (kWh/m2)
    double Egas_ht = simData.v_Qgas_ht[i] * energy_factor;
    double Egas_cl = simData.v_Qcl_gas_tot[i] * energy_factor;
    double Egas_plug = v_Q_plug_gas[i]; // Already kWh/m2
    double Egas_dhw = simData.v_Q_dhw_gas[i] * invFloorArea; // Already kWh

#ifdef ISOMODEL_STANDALONE
    EndUses eu;
    int euse = 0;
    eu.addEndUse(euse++, Eelec_ht);
    eu.addEndUse(euse++, Eelec_cl);
    eu.addEndUse(euse++, Eelec_int_lt);
    eu.addEndUse(euse++, Eelec_ext_lt);
    eu.addEndUse(euse++, Eelec_fan);
    eu.addEndUse(euse++, Eelec_pump);
    eu.addEndUse(euse++, Eelec_plug);
    eu.addEndUse(euse++, 0); // Exterior Equipment
    eu.addEndUse(euse++, Eelec_dhw);
    eu.addEndUse(euse++, Egas_ht);
    eu.addEndUse(euse++, Egas_cl);
    eu.addEndUse(euse++, Egas_plug);
    eu.addEndUse(euse++, Egas_dhw);
    allResults.push_back(eu);
#else
    EndUses eu;
    eu.addEndUse(Eelec_ht, EndUseFuelType::Electricity,
                 EndUseCategoryType::Heating);
    eu.addEndUse(Eelec_cl, EndUseFuelType::Electricity,
                 EndUseCategoryType::Cooling);
    eu.addEndUse(Eelec_int_lt, EndUseFuelType::Electricity,
                 EndUseCategoryType::InteriorLights);
    eu.addEndUse(Eelec_ext_lt, EndUseFuelType::Electricity,
                 EndUseCategoryType::ExteriorLights);
    eu.addEndUse(Eelec_fan, EndUseFuelType::Electricity,
                 EndUseCategoryType::Fans);
    eu.addEndUse(Eelec_pump, EndUseFuelType::Electricity,
                 EndUseCategoryType::Pumps);
    eu.addEndUse(Eelec_plug, EndUseFuelType::Electricity,
                 EndUseCategoryType::InteriorEquipment);
    eu.addEndUse(0, EndUseFuelType::Electricity,
                 EndUseCategoryType::ExteriorEquipment);
    eu.addEndUse(Eelec_dhw, EndUseFuelType::Electricity,
                 EndUseCategoryType::WaterSystems);

    eu.addEndUse(Egas_ht, EndUseFuelType::Gas, EndUseCategoryType::Heating);
    eu.addEndUse(Egas_cl, EndUseFuelType::Gas, EndUseCategoryType::Cooling);
    eu.addEndUse(Egas_plug, EndUseFuelType::Gas,
                 EndUseCategoryType::InteriorEquipment);
    eu.addEndUse(Egas_dhw, EndUseFuelType::Gas,
                 EndUseCategoryType::WaterSystems);
    allResults.push_back(eu);
#endif
  }
  return allResults;
}
} // namespace openstudio::isomodel
