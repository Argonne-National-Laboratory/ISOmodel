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

Matrix MonthlyModel::buildSolarIrradianceMatrix(const WeatherData &weather) {
  PROFILE_FUNCTION();
  // Combine vertical surface radiation (msolar) and horizontal radiation
  // (mEgh) into one matrix (W/m2).
  Matrix m_I_sol(MONTHS_IN_YEAR, NUM_TOTAL_SURFACES);

  // Access weather data via reference to avoid copy
  const Matrix &m_solar = weather.msolarRef();
  const Vector &v_mEgh = weather.mEghRef();

  for (unsigned int r = 0; r < m_I_sol.size1(); r++) {
    for (unsigned int c = 0; c < NUM_VERTICAL_SURFACES; c++) { // Vertical surfaces
      m_I_sol(r, c) = m_solar(r, c);
    }
    m_I_sol(r, NUM_VERTICAL_SURFACES) = v_mEgh[r]; // Roof/Horizontal surface
  }
  return m_I_sol;
}

MonthlyModel::AnnualLightingHours
MonthlyModel::calculateAnnualLightingOperationalHours(const Lighting &lights,
                                                      const Population &pop) {
  PROFILE_FUNCTION();

  AnnualLightingHours result;

  // Lighting operational hours during the daytime.
  double hoursOccupied = std::min(lights.n_day_end(), pop.hoursEnd()) -
                         std::max(pop.hoursStart(), lights.n_day_start());
  if (hoursOccupied < 0) {
    hoursOccupied += HOURS_IN_DAY; // Use constant
  }
  double daysOccupied = pop.daysEnd() - pop.daysStart() + 1;
  if (daysOccupied < 0) {
    daysOccupied += DAYS_IN_WEEK; // Use constant
  }
  result.t_lt_D = hoursOccupied * daysOccupied * lights.n_weeks();

  // Lighting operational hours during the nighttime.
  hoursOccupied = std::max(lights.n_day_start() - pop.hoursStart(), 0.0) +
                  std::max(pop.hoursEnd() - lights.n_day_end(), 0.0);
  result.t_lt_N = hoursOccupied * daysOccupied * lights.n_weeks();

  // Unoccupied hours.
  result.t_unocc = HOURS_IN_YEAR - result.t_lt_D - result.t_lt_N;

  return result;
}

/**
 * Breaks down the solar radiation and temperature data into day, night,
 * weekday and weekend vectors, as appropriate.
 */
void MonthlyModel::solarRadiationBreakdown(MonthlySimulationData &simData) const {
  PROFILE_FUNCTION();
  const Matrix &m_mhEgh = location.weather()->mhEghRef();
  const Matrix &m_mhdbt = location.weather()->mhdbtRef();
  const auto &sched = simData.scheduleData;

  double sum_occ = sum(sched.clockHourOccupied);
  double sum_unocc = sum(sched.clockHourUnoccupied);

  for (int i = 0; i < MONTHS_IN_YEAR; ++i) {
    double Tdbt_day_sum = 0.0;
    double Tdbt_nt_sum = 0.0;
    double Egh_day_sum = 0.0;
    double Egh_nt_sum = 0.0;
    int sun_up_time = 0;
    int sun_down_time = 0;
    bool sun_found = false;

    for (int h = 0; h < 24; ++h) {
      double val_dbt = m_mhdbt(i, h);
      double val_egh = m_mhEgh(i, h);
      double occ = sched.clockHourOccupied[h];
      double unocc = sched.clockHourUnoccupied[h];

      Tdbt_day_sum += val_dbt * occ;
      Tdbt_nt_sum += val_dbt * unocc;
      Egh_day_sum += val_egh * occ;
      Egh_nt_sum += val_egh * unocc;

      if (val_egh != 0) {
        if (!sun_found) {
          sun_up_time = h;
          sun_found = true;
        }
        sun_down_time = h;
      }
    }

    simData.v_Tdbt_day[i] = (sum_occ > 0) ? (Tdbt_day_sum / sum_occ) : 0.0;
    simData.v_Tdbt_nt[i] = (sum_unocc > 0) ? (Tdbt_nt_sum / sum_unocc) : 0.0;

    double Egh_day = (sum_occ > 0) ? (Egh_day_sum / sum_occ) : 0.0;
    double Egh_nt = (sum_unocc > 0) ? (Egh_nt_sum / sum_unocc) : 0.0;

    double Wgh_wk_day = Egh_day * sched.weekdayOccupiedMegaseconds[i];
    double Wgh_wk_nt = Egh_nt * sched.weekdayUnoccupiedMegaseconds[i];
    double Wgh_wke_day = Egh_day * sched.weekendOccupiedMegaseconds[i];
    double Wgh_wke_nt = Egh_nt * sched.weekendUnoccupiedMegaseconds[i];
    double Wgh_tot = Wgh_wk_day + Wgh_wk_nt + Wgh_wke_day + Wgh_wke_nt;

    if (Wgh_tot > 0) {
      simData.frac_Pgh_wk_nt[i] = Wgh_wk_nt / Wgh_tot;
      simData.frac_Pgh_wke_day[i] = Wgh_wke_day / Wgh_tot;
      simData.frac_Pgh_wke_nt[i] = Wgh_wke_nt / Wgh_tot;
    } else {
      simData.frac_Pgh_wk_nt[i] = 0.0;
      simData.frac_Pgh_wke_day[i] = 0.0;
      simData.frac_Pgh_wke_nt[i] = 0.0;
    }

    double frac_hrs_sun_up = (sun_down_time - sun_up_time + 1) / 24.0;
    simData.v_hrs_sun_down_mo[i] = (1.0 - frac_hrs_sun_up) * HOURS_IN_MONTH[i];
  }
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
                        (annualHours.t_lt_D * F_D + annualHours.t_lt_N) * WATTS_TO_KILOWATTS;
  // Total annual lighting energy for unnocupied times (kWh).
  simData.Q_illum_unocc =
      structure.floorArea() * lpd_unocc * annualHours.t_unocc * WATTS_TO_KILOWATTS;
  // Total annual lighting energy (kWh).
  simData.Q_illum_tot_yr = simData.Q_illum_occ + simData.Q_illum_unocc;

  double ext_energy_kW = lights.exteriorEnergy() * WATTS_TO_KILOWATTS;

  for (int i = 0; i < MONTHS_IN_YEAR; ++i) {
    simData.v_Q_illum_tot[i] = MONTH_FRACTION_OF_YEAR[i] * simData.Q_illum_tot_yr;
    simData.v_Q_illum_ext_tot[i] = simData.v_hrs_sun_down_mo[i] * ext_energy_kW;
  }
}

/**
 * Compute envelope parameters as per ISO 13790 8.3.
 */
void MonthlyModel::envelopeCalculations(MonthlySimulationData &simData) const {
  PROFILE_FUNCTION();
  const Vector &v_wall_A = structure.wallAreaRef();
  const Vector &v_win_A = structure.windowAreaRef();
  const Vector &v_wall_U = structure.wallUniformRef();
  const Vector &v_win_U = structure.windowUniformRef();

  // Compute direct transmission heat transfer coefficient (H_D)
  double H_D = 0.0;
  for (int i = 0; i < NUM_TOTAL_SURFACES; ++i) {
    H_D += (v_wall_A[i] * v_wall_U[i]) + (v_win_A[i] * v_win_U[i]);
  }

  // For now, also ignore heat transfer to ground (minimal in large buildings),
  // unconditioned spaces, and adjacent buildings.
  // TODO: Implement ground, unconditioned, and adjacent above heat transfer
  // coefficients. BAA@2015-07-13.
  double H_g = 0;
  double H_U = 0;
  double H_A = 0;

  // Total transmission heat transfer coefficient. ISO 13790 8.3.1 eq. 17.
  simData.H_tr = H_D + H_g + H_U + H_A;
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
  // fraction.

  const Vector &v_g_gln = structure.windowNormalIncidenceSolarEnergyTransmittanceRef();
  double win_F_W = structure.win_F_W();
  double win_ff_base = UNITY_FRACTION - structure.win_ff();
  double R_sc_ext = structure.R_sc_ext();
  double hr_factor = ISO_WIN_EXT_RAD_COEFF;

  const Vector &v_win_A = structure.windowAreaRef();
  const Vector &v_wall_emiss = structure.wallThermalEmissivityRef();
  const Vector &v_wall_alpha_sc = structure.wallSolarAbsorptionRef();
  const Vector &v_wall_U = structure.wallUniformRef();
  const Vector &v_wall_A = structure.wallAreaRef();

  for (int i = 0; i < NUM_TOTAL_SURFACES; ++i) {
    // Window Shading & Solar Area
    double SDF = WIN_SDF_TABLE[((int)structure.windowShadingDeviceRef()[i]) - 1];
    double F_shgl = SDF * UNITY_FRACTION;
    double g_gl = v_g_gln[i] * win_F_W;
    simData.v_win_A_sol[i] = F_shgl * g_gl * win_ff_base * v_win_A[i];

    // Wall R_sc
    simData.v_wall_R_sc[i] = R_sc_ext;

    // Window hr
    simData.v_win_hr[i] = v_wall_emiss[i] * hr_factor;

    // Wall A_sol
    simData.v_wall_A_sol[i] =
        v_wall_alpha_sc[i] * simData.v_wall_R_sc[i] * v_wall_U[i] * v_wall_A[i];
  }
}

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

  // Pre-calculate opaque thermal radiation to sky (constant over months)
  // \Phi_r,k = R_se * U_c  * A_c * h_h * \delta\theta_er
  // theta_er is constant ISO_SKY_TEMP_DIFF (11 K)
  double theta_er = ISO_SKY_TEMP_DIFF;

  const Vector &v_wall_U = structure.wallUniformRef();
  const Vector &v_wall_A = structure.wallAreaRef();

  Vector v_wall_phi_r(NUM_TOTAL_SURFACES);
  for (int j = 0; j < NUM_TOTAL_SURFACES; ++j) {
    v_wall_phi_r[j] =
        simData.v_wall_R_sc[j] * v_wall_U[j] * v_wall_A[j] * simData.v_win_hr[j] * theta_er;
  }

  // Pre-calculate window shading factors
  const Vector &v_win_SCF = structure.windowShadingCorrectionFactorRef();
  const Vector &v_win_A_sol = simData.v_win_A_sol;
  const Vector &v_wall_A_sol = simData.v_wall_A_sol;

  for (int i = 0; i < MONTHS_IN_YEAR; ++i) {
    double phi_sol = 0.0;
    for (int j = 0; j < NUM_TOTAL_SURFACES; ++j) {
      double I_sol = m_I_sol(i, j);
      // Glazing Gain: SCF * A_sol * I_sol (SCF_frac is 1.0)
      phi_sol += v_win_SCF[j] * v_win_A_sol[j] * I_sol;
      // Opaque Gain: A_sol * I_sol - phi_r * formFactor
      phi_sol += v_wall_A_sol[j] * I_sol - v_wall_phi_r[j] * ENV_FORM_FACTORS[j];
    }
    simData.v_E_sol[i] = phi_sol * MEGASECONDS_IN_MONTH[i];
    if (DEBUG_ISO_MODEL_SIMULATION) {
      std::cout << "v_phi_sol[" << i << "]=" << phi_sol << std::endl;
    }
  }
}

/**
 * Compute internal heat gains and losses.
 */
void MonthlyModel::calculateInternalGainComponents(MonthlySimulationData &simData) const {
  PROFILE_FUNCTION();
  // Internal heat gains from people (W/m2).
  double phi_int_occ = pop.heatGainPerPerson() / pop.densityOccupied();
  double phi_int_unocc = pop.heatGainPerPerson() / pop.densityUnoccupied();
  simData.phi_int_avg = std::lerp(phi_int_unocc, phi_int_occ, simData.scheduleData.frac_hrs_wk_day);

  // Internal heat gain from appliances (W/m2).
  double phi_plug_occ =
      building.electricApplianceHeatGainOccupied() + building.gasApplianceHeatGainOccupied();
  double phi_plug_unocc =
      building.electricApplianceHeatGainUnoccupied() + building.gasApplianceHeatGainUnoccupied();
  simData.phi_plug_avg =
      std::lerp(phi_plug_unocc, phi_plug_occ, simData.scheduleData.frac_hrs_wk_day);

  // Internal heat gain from illumination (W/m2).
  double floor_area = structure.floorArea();
  double inv_area_hours = KILOWATTS_TO_WATTS / (floor_area * HOURS_IN_YEAR);

  // phi_illum_occ is unused
  double phi_illum_unocc = (simData.Q_illum_unocc * inv_area_hours) /
                           (UNITY_FRACTION - simData.scheduleData.frac_hrs_wk_day);
  simData.phi_illum_avg = simData.Q_illum_tot_yr * inv_area_hours;

  double phi_unoccupied_total = (phi_int_unocc + phi_plug_unocc + phi_illum_unocc);
  simData.phi_int_wk_nt = phi_unoccupied_total;
  simData.phi_int_wke_day = phi_unoccupied_total;
  simData.phi_int_wke_nt = phi_unoccupied_total;

  // Total internal heat gain (W).
  simData.phi_I_tot =
      (simData.phi_int_avg + simData.phi_plug_avg + simData.phi_illum_avg) * floor_area;
}

/**
 * Compute unoccupied heat gain.
 */
void MonthlyModel::unoccupiedHeatGain(MonthlySimulationData &simData) const {
  PROFILE_FUNCTION();

  double floor_area = structure.floorArea();
  double phi_int_wk_nt = simData.phi_int_wk_nt;
  double phi_int_wke_day = simData.phi_int_wke_day;
  double phi_int_wke_nt = simData.phi_int_wke_nt;

  for (int i = 0; i < MONTHS_IN_YEAR; ++i) {
    // Week Night
    double ms_wk_nt = simData.scheduleData.weekdayUnoccupiedMegaseconds[i];
    double W_int_wk_nt = ms_wk_nt * phi_int_wk_nt * floor_area;
    double W_sol_wk_nt = simData.v_E_sol[i] * simData.frac_Pgh_wk_nt[i];
    simData.v_P_tot_wk_nt[i] = (ms_wk_nt > 0) ? (W_int_wk_nt + W_sol_wk_nt) / ms_wk_nt : 0.0;

    // Weekend Day
    double ms_wke_day = simData.scheduleData.weekendOccupiedMegaseconds[i];
    double W_int_wke_day = ms_wke_day * phi_int_wke_day * floor_area;
    double W_sol_wke_day = simData.v_E_sol[i] * simData.frac_Pgh_wke_day[i];
    simData.v_P_tot_wke_day[i] =
        (ms_wke_day > 0) ? (W_int_wke_day + W_sol_wke_day) / ms_wke_day : 0.0;

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

/*
 * Calculate interior temp.
 */
void MonthlyModel::calculateInteriorTemperatures(MonthlySimulationData &simData) const {
  PROFILE_FUNCTION();
  // Set the temp differential from the interior heating/cooling setpoint
  // based on the BEM type. An advanced BEM has the effect of reducing the
  // effective heating temp and raising the effective cooling temp during
  // times of control (i.e. during occupancy).

  double T_adj = building.buildingEnergyManagement();

  if (DEBUG_ISO_MODEL_SIMULATION) {
    std::cout << "BEM Adjustment: " << T_adj << std::endl;
  }

  // Adjust the heating set points.
  double ht_tset_ctrl = heating.temperatureSetPointOccupied() - T_adj;
  double cl_tset_ctrl = cooling.temperatureSetPointOccupied() + T_adj;

  // During unoccupied times, we use a setback temp and even if we have a BEM
  // it has no effect.
  double ht_tset_unocc = heating.temperatureSetPointUnoccupied();
  double cl_tset_unocc = cooling.temperatureSetPointUnoccupied();

  // Interior heat capacity (J/k).
  double Cm_int = structure.interiorHeatCapacity() * structure.floorArea();

  // Envelope heat capacity (J/k).
  double Cm_env = structure.wallHeatCapacity() * sum(structure.wallAreaRef());

  // Total heat capacity (J/k).
  double Cm = Cm_int + Cm_env;

  // Total heat transfer coefficient.
  double H_tot = simData.H_tr + ventilation.H_ve();

  // Building time constant in hours as pwer ISO 13790 12.2.1.3 eq. 62.
  simData.tau = Cm / H_tot / 3600.0;

  Vector v_ti(5);
  v_ti[0] = v_ti[2] = v_ti[4] = simData.scheduleData.hoursUnoccupiedPerDay;
  v_ti[1] = v_ti[3] = simData.scheduleData.hoursOccupiedPerDay;

  bool do_heating = (heating.T_ht_ctrl_flag() == 1);
  bool do_cooling = (cooling.T_cl_ctrl_flag() == 1);

  for (int m = 0; m < MONTHS_IN_YEAR; ++m) {
    double Th_wk_nt_val = ht_tset_ctrl;
    double Th_wke_avg_val = ht_tset_ctrl;
    double Tc_wk_nt_val = cl_tset_ctrl;
    double Tc_wke_avg_val = cl_tset_ctrl;

    // Heating
    if (do_heating) {
      double current_T = ht_tset_ctrl;
      double wke_sum = 0.0;
      for (int step = 0; step < 5; ++step) {
        double ti = v_ti[step];
        double exp_val = std::exp(-ti / simData.tau);
        double P_tot = (step == 0)       ? simData.v_P_tot_wk_nt[m]
                       : (step % 2 != 0) ? simData.v_P_tot_wke_day[m]
                                         : simData.v_P_tot_wke_nt[m];
        double Te = (step % 2 != 0) ? simData.v_Tdbt_day[m] : simData.v_Tdbt_nt[m];
        double dT = P_tot / H_tot;
        double limit = (step == 0) ? ht_tset_ctrl : std::max(current_T, ht_tset_unocc);
        double term = (limit - Te - dT);
        double avg = (simData.tau / ti) * term * (1.0 - exp_val) + Te + dT;
        avg = std::max(avg, ht_tset_unocc);
        wke_sum += avg;
        if (step == 1)
          Th_wk_nt_val = avg;
        if (step < 4)
          current_T = term * exp_val + Te + dT;
      }
      Th_wke_avg_val = wke_sum / 5.0;
    }

    // Cooling
    if (do_cooling) {
      double current_T = cl_tset_ctrl;
      double wke_sum = 0.0;
      double limit_start = std::min(ht_tset_ctrl, cl_tset_unocc);
      for (int step = 0; step < 5; ++step) {
        double ti = v_ti[step];
        double exp_val = std::exp(-ti / simData.tau);
        double P_tot = (step == 0)       ? simData.v_P_tot_wk_nt[m]
                       : (step % 2 != 0) ? simData.v_P_tot_wke_day[m]
                                         : simData.v_P_tot_wke_nt[m];
        double Te = (step % 2 != 0) ? simData.v_Tdbt_day[m] : simData.v_Tdbt_nt[m];
        double dT = P_tot / H_tot;
        double limit = (step == 0) ? limit_start : std::max(current_T, cl_tset_unocc);
        double term = (limit - Te - dT);
        double avg = (simData.tau / ti) * term * (1.0 - exp_val) + Te + dT;
        avg = std::max(avg, cl_tset_unocc);
        wke_sum += avg;
        if (step == 1)
          Tc_wk_nt_val = avg;
        if (step < 4)
          current_T = term * exp_val + Te + dT;
      }
      Tc_wke_avg_val = wke_sum / 5.0;
    }

    double Th_wk_avg = (ht_tset_ctrl * simData.scheduleData.frac_hrs_wk_day) +
                       (Th_wk_nt_val * simData.scheduleData.frac_hrs_wk_nt) +
                       (Th_wke_avg_val * simData.scheduleData.frac_hrs_wke_tot);
    double Tc_wk_avg = (cl_tset_ctrl * simData.scheduleData.frac_hrs_wk_day) +
                       (Tc_wk_nt_val * simData.scheduleData.frac_hrs_wk_nt) +
                       (Tc_wke_avg_val * simData.scheduleData.frac_hrs_wke_tot);

    simData.v_Th_avg[m] = std::min(Th_wk_avg, ht_tset_ctrl);
    simData.v_Tc_avg[m] = std::min(Tc_wk_avg, cl_tset_ctrl);
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
      ventilation.supplyRate() / structure.floorArea() / LITERS_PER_SECOND_TO_METERS3_PER_HOUR;

  // Vent exhaust rate m3/h/m2, negative indicates out of building.
  double qv_ext = -(qv_supp - ventilation.supplyDifference() / structure.floorArea() /
                                  LITERS_PER_SECOND_TO_METERS3_PER_HOUR);

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
    vent_op_frac = std::lerp(pop.densityOccupied() / pop.densityUnoccupied(), 1.0,
                             simData.scheduleData.frac_hrs_wk_day);
    break;
  }

  double mve_init = ventilation.ventType() == VentilationType::Natural
                        ? 0
                        : (vent_op_frac * qv_supp * vent_outdoor_frac * (1 - vent_ht_recov));

  // OPTIMIZATION: Fused vector operations into a single loop to avoid temporary allocations.
  double stack_exp = ventilation.stack_exp();
  double stack_coeff_Q4 = ventilation.stack_coeff() * v_Q4pa;
  double wind_exp = ventilation.wind_exp();
  double wind_coeff = ventilation.wind_coeff();
  double dCp_terrain = ventilation.dCp() * location.terrain();

  for (int i = 0; i < MONTHS_IN_YEAR; ++i) {
    // Stack Effect Heating
    double dbtDiff_ht = std::abs(v_mdbt[i] - simData.v_Th_avg[i]);
    double qv_stack_ht =
        std::max(std::pow(dbtDiff_ht * h_stack, stack_exp) * stack_coeff_Q4, MIN_INFILTRATION_FLOW);

    // Stack Effect Cooling
    double dbtDiff_cl = std::abs(v_mdbt[i] - simData.v_Tc_avg[i]);
    double qv_stack_cl =
        std::max(std::pow(dbtDiff_cl * h_stack, stack_exp) * stack_coeff_Q4, MIN_INFILTRATION_FLOW);

    // Wind Effect
    double wind_sq = v_mwind[i] * v_mwind[i];
    double qv_wind = std::pow(wind_sq * dCp_terrain, wind_exp) * v_Q4pa * wind_coeff;

    // Combined (Superposition)
    double qv_ht_max = std::max(qv_stack_ht, qv_wind);
    double qv_cl_max = std::max(qv_stack_cl, qv_wind);

    double qv_sw_ht = qv_ht_max + (qv_stack_ht * qv_wind) * N_SW_COEFF / v_Q4pa;
    double qv_sw_cl = qv_cl_max + (qv_stack_cl * qv_wind) * N_SW_COEFF / v_Q4pa;

    // Infiltration
    double qv_inf_ht = qv_sw_ht + std::max(0.0, -qv_diff);
    double qv_inf_cl = qv_sw_cl + std::max(0.0, -qv_diff);

    // Total
    double qve_ht = qv_inf_ht + mve_init;
    double qve_cl = qv_inf_cl + mve_init;

    // Hve
    simData.v_Hve_ht[i] = qve_ht * RHO_CP_AIR_IN_WATT_HOURS;
    simData.v_Hve_cl[i] = qve_cl * RHO_CP_AIR_IN_WATT_HOURS;
  }
}

/**
 * Compute monthly heating and cooling demand.
 */
void MonthlyModel::calculateHeatingAndCoolingNeeds(MonthlySimulationData &simData) const {
  PROFILE_FUNCTION();
  // Optimization: Cache weather reference
  const Vector &v_mdbt = location.weather()->mdbtRef();

  // Initialize yearly sums
  simData.Qneed_ht_yr = 0.0;
  simData.Qneed_cl_yr = 0.0;

  // Constants for loop
  double floor_area = structure.floorArea();
  double H_tr = simData.H_tr;
  double a_H = heating.a_H0() + simData.tau / heating.tau_H0();

  // Air Volume constants
  double T_sup_ht = heating.temperatureSetPointOccupied() + heating.dT_supp_ht();
  double T_sup_cl = cooling.temperatureSetPointOccupied() - cooling.dT_supp_cl();

  // Total Air Flow constants
  double min_flow_rate = ventilation.supplyRate() * simData.scheduleData.frac_hrs_wk_day *
                         (MEGASECONDS_TO_SECONDS / LITERS_TO_M3);

  // Fan Energy constants
  double fan_power_factor = ventilation.fanPower() / KILOJOULE_TO_MEGAJOULE;
  double area_kWh_factor = floor_area * KILOWATTHOURS_TO_MEGAJOULES;

  for (int i = 0; i < MONTHS_IN_YEAR; ++i) {
    // 1. Gains and Losses
    double tot_mo_ht_gain = (simData.phi_I_tot * MEGASECONDS_IN_MONTH[i]) + simData.v_E_sol[i];

    double Th_avg_minus_mdbt = simData.v_Th_avg[i] - v_mdbt[i];
    double Qtot_ht =
        (Th_avg_minus_mdbt * MEGASECONDS_IN_MONTH[i] * H_tr) +
        (simData.v_Hve_ht[i] * floor_area * Th_avg_minus_mdbt * MEGASECONDS_IN_MONTH[i]);

    double Tc_avg_minus_mdbt = simData.v_Tc_avg[i] - v_mdbt[i];
    double Qtot_cl =
        (Tc_avg_minus_mdbt * H_tr * MEGASECONDS_IN_MONTH[i]) +
        (simData.v_Hve_cl[i] * floor_area * Tc_avg_minus_mdbt * MEGASECONDS_IN_MONTH[i]);

    // 2. Heating Need
    double gamma_H_ht = tot_mo_ht_gain / (Qtot_ht + SAFE_EPSILON);
    double eta_g_H;
    if (gamma_H_ht > 0) {
      double num = std::pow(gamma_H_ht, a_H);
      eta_g_H = (UNITY_FRACTION - num) / (UNITY_FRACTION - num * gamma_H_ht);
    } else {
      eta_g_H = UNITY_FRACTION / (gamma_H_ht + SAFE_EPSILON);
    }

    simData.v_Qneed_ht[i] = Qtot_ht - (eta_g_H * tot_mo_ht_gain);
    simData.Qneed_ht_yr += simData.v_Qneed_ht[i];

    // 3. Cooling Need
    double gamma_H_cl = Qtot_cl / (tot_mo_ht_gain + SAFE_EPSILON);
    double eta_g_CL;
    if (gamma_H_cl > 0) {
      double num = std::pow(gamma_H_cl, a_H);
      eta_g_CL = (UNITY_FRACTION - num) / (UNITY_FRACTION - num * gamma_H_cl);
    } else {
      eta_g_CL = UNITY_FRACTION / (gamma_H_cl + SAFE_EPSILON);
    }

    simData.v_Qneed_cl[i] = tot_mo_ht_gain - (eta_g_CL * Qtot_cl);
    simData.Qneed_cl_yr += simData.v_Qneed_cl[i];

    // 4. Air Volumes
    double denominator_ht = ((T_sup_ht - simData.v_Th_avg[i]) * RHO_CP_AIR) + SAFE_EPSILON;
    simData.v_Vair_ht[i] = simData.v_Qneed_ht[i] / denominator_ht;

    double denominator_cl = ((simData.v_Tc_avg[i] - T_sup_cl) * RHO_CP_AIR) + SAFE_EPSILON;
    simData.v_Vair_cl[i] = simData.v_Qneed_cl[i] / denominator_cl;

    // 5. Total Air Flow
    double sum_Vair = simData.v_Vair_ht[i] + simData.v_Vair_cl[i];
    double min_flow_month = MEGASECONDS_IN_MONTH[i] * min_flow_rate;
    simData.v_Vair_tot[i] = std::max(sum_Vair, min_flow_month);

    // 6. Fan Energy
    simData.v_Qfan_tot[i] = (simData.v_Vair_tot[i] * fan_power_factor) / area_kWh_factor;
  }

  if (DEBUG_ISO_MODEL_SIMULATION) {
    // Note: fanEnergy is no longer available here, but we can print the inputs and output
    printVector("v_Vair_ht", simData.v_Vair_ht);
    printVector("v_Vair_cl", simData.v_Vair_cl);
    printVector("v_Vair_tot (input to fan calc)", simData.v_Vair_tot);
    std::cout << "ventilation.fanPower() = " << ventilation.fanPower() << std::endl;
    std::cout << "structure.floorArea() = " << structure.floorArea() << std::endl;
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
  double f_dem_ht = std::max(simData.Qneed_ht_yr / (simData.Qneed_cl_yr + simData.Qneed_ht_yr),
                             MIN_DEMAND_FRACTION);
  // Fraction of yearly cooling demand.
  double f_dem_cl = std::max((1.0 - f_dem_ht), MIN_DEMAND_FRACTION);

  // Overall distribution efficiency for heating.
  double eta_dist_ht = 1.0 / (1.0 + a_ht_loss + f_waste / f_dem_ht);
  // Overall distrubtion efficiency for cooling.
  double eta_dist_cl = 1.0 / (1.0 + a_cl_loss + f_waste / f_dem_cl);

  double ht_eff = heating.efficiency() + SAFE_EPSILON;
  double ht_dh_free = 1.0 - heating.frac_DH_free();
  double ht_dh_sys_net = heating.eta_DH_sys() * heating.eta_DH_network();
  double cl_dc_frac_abs = 1.0 - cooling.eta_DC_frac_abs();
  double cl_dc_elec_net = cooling.eta_DC_COP() * cooling.eta_DC_network();
  double cl_dc_free = 1.0 - cooling.frac_DC_free();
  double cl_dc_cop_abs = cooling.eta_DC_COP_abs();
  bool is_ht_elec = (heating.energyType() == FuelType::Electric);

  for (int i = 0; i < MONTHS_IN_YEAR; ++i) {
    double Qloss_ht_dist = simData.v_Qneed_ht[i] * (1.0 - eta_dist_ht) / eta_dist_ht;
    double Qloss_cl_dist = simData.v_Qneed_cl[i] * (1.0 - eta_dist_cl) / eta_dist_cl;

    double Qht_sys, Qht_DH, Qcl_sys, Qcool_DC;
    if (heating.DH_YesNo() == 1) {
      Qht_DH = simData.v_Qneed_ht[i] + Qloss_ht_dist;
      Qht_sys = 0.0;
    } else {
      Qht_sys = (Qloss_ht_dist + simData.v_Qneed_ht[i]) / ht_eff;
      Qht_DH = 0.0;
    }

    if (cooling.DC_YesNo() == 1) {
      Qcool_DC = simData.v_Qneed_cl[i] + Qloss_cl_dist;
      Qcl_sys = 0.0;
    } else {
      Qcl_sys = (Qloss_cl_dist + simData.v_Qneed_cl[i]) / (IEER + SAFE_EPSILON);
      Qcool_DC = 0.0;
    }

    double Qcl_DC_elec = (Qcool_DC * cl_dc_frac_abs) / cl_dc_elec_net;
    double Qcl_DC_abs = (Qcool_DC * cl_dc_free) / cl_dc_cop_abs;
    double Qht_DH_total = (Qht_DH * ht_dh_free) / ht_dh_sys_net;

    simData.v_Qht_sys[i] = Qht_sys;
    simData.v_Qht_DH[i] = Qht_DH;
    simData.v_Qcl_sys[i] = Qcl_sys;
    simData.v_Qcool_DC[i] = Qcool_DC;

    simData.v_Qcl_elec_tot[i] = Qcl_sys + Qcl_DC_elec;
    simData.v_Qcl_gas_tot[i] = Qcl_DC_abs;

    if (is_ht_elec) {
      simData.v_Qelec_ht[i] = Qht_sys;
      simData.v_Qgas_ht[i] = Qht_DH_total;
    } else {
      simData.v_Qelec_ht[i] = 0.0;
      simData.v_Qgas_ht[i] = Qht_sys + Qht_DH_total;
    }
  }

  if (DEBUG_ISO_MODEL_SIMULATION) {
    printVector("v_Qelec_ht", simData.v_Qelec_ht);
    printVector("v_Qgas_ht", simData.v_Qgas_ht);
  }
}

Vector MonthlyModel::calculatePumpEnergyForMode(const Vector &v_Qneed_mode,
                                                const Vector &v_Qneed_total,
                                                double E_pumps_w_per_m2,
                                                double pump_control_reduction, double floor_area) {
  PROFILE_FUNCTION();

  // Total annual pump energy for the mode if pumps run continuously (MJ/m2).
  double Q_pumps_yr_mode_per_m2 = sum(mult(MEGASECONDS_IN_MONTH, E_pumps_w_per_m2, MONTHS_IN_YEAR));

  // Fraction of time the system is in this mode each month.
  Vector v_frac_mode = div(v_Qneed_mode, v_Qneed_total);

  // Total energy fraction for this mode over the year.
  double frac_total = sum(v_frac_mode);

  // Total yearly pump energy, adjusted by control factor and floor area (MJ).
  double Q_pumps_mode = Q_pumps_yr_mode_per_m2 * pump_control_reduction * floor_area;

  // Distribute the total annual pump energy for this mode across the months.
  return div(mult(v_frac_mode, Q_pumps_mode), frac_total + SAFE_EPSILON);
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

  // Total monthly heating and cooling need (MJ).
  Vector v_Qneed_total = sum(simData.v_Qneed_ht, simData.v_Qneed_cl);

  // Calculate monthly pump energy for heating mode.
  Vector v_Q_pumps_ht =
      calculatePumpEnergyForMode(simData.v_Qneed_ht, v_Qneed_total, heating.E_pumps(),
                                 heating.pumpControlReduction(), structure.floorArea());

  // Calculate monthly pump energy for cooling mode.
  Vector v_Q_pumps_cl =
      calculatePumpEnergyForMode(simData.v_Qneed_cl, v_Qneed_total, cooling.E_pumps(),
                                 cooling.pumpControlReduction(), structure.floorArea());

  // Total pump operational factor.
  Vector v_frac_tot =
      div(sum(simData.v_Qneed_ht, simData.v_Qneed_cl), simData.Qneed_ht_yr + simData.Qneed_cl_yr);
  double frac_total = sum(v_frac_tot);
  double Q_pumps_tot = sum(v_Q_pumps_ht) + sum(v_Q_pumps_cl);

  if (sum(v_Q_pumps_ht) == 0.0 || sum(v_Q_pumps_cl) == 0.0) {
    // If there is just heating or just cooling, use the individual heating or
    // cooling pump energy vector.
    simData.v_Q_pump_tot = sum(v_Q_pumps_ht, v_Q_pumps_cl);
  } else {
    // Otherwise, distribut the combined pump energy proportional to the
    // combined heating/cooling load.
    simData.v_Q_pump_tot = div(mult(v_frac_tot, Q_pumps_tot), frac_total + SAFE_EPSILON);
  }
}

/**
 * Energy Generation
 * NOT INCLUDED YET
 */
void MonthlyModel::energyGeneration() const {
  PROFILE_FUNCTION();
}

/**
 * Calculate domestic hot water (DHW).
 * References: NEN 2916 12.2
 */
void MonthlyModel::calculateHeatedWaterEnergy(MonthlySimulationData &simData) const {
  PROFILE_FUNCTION();

  // Total annual energy demand required for heating DHW (MJ/yr).
  double Q_dhw_yr =
      heating.hotWaterDemand() * (heating.dhw_tset() - heating.dhw_tsupply()) * RHO_CP_WATER;

  double inv_dist_eff = 1.0 / heating.hotWaterDistributionEfficiency();
  double inv_sys_eff = 1.0 / heating.hotWaterSystemEfficiency();
  double inv_KILOWATTHOURS_TO_MEGAJOULES = 1.0 / KILOWATTHOURS_TO_MEGAJOULES;
  bool is_elec = (heating.hotWaterEnergyType() == FuelType::Electric);

  for (int i = 0; i < MONTHS_IN_YEAR; ++i) {
    double monthlyDemand = DAYS_IN_MONTH[i] * Q_dhw_yr;
    double frac_MonthlyDemand_yr = monthlyDemand / DAYS_IN_YEAR;
    double Qe_demand = frac_MonthlyDemand_yr * inv_dist_eff;
    double Q_dhw_demand = Qe_demand * inv_KILOWATTHOURS_TO_MEGAJOULES;
    // v_Q_dhw_solar is zero, so we can ignore it in `dif`
    double Q_dhw_need = std::max(0.0, Q_dhw_demand * inv_sys_eff);

    if (is_elec) {
      simData.v_Q_dhw_elec[i] = Q_dhw_need;
      simData.v_Q_dhw_gas[i] = 0.0;
    } else {
      simData.v_Q_dhw_gas[i] = Q_dhw_need;
      simData.v_Q_dhw_elec[i] = 0.0;
    }
  }

  if (DEBUG_ISO_MODEL_SIMULATION) {
    // Intermediate vectors no longer exist, so cannot print them.
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

    std::cout << "phi_I_tot: " << simData.phi_I_tot << std::endl;

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
std::vector<EndUses> MonthlyModel::outputGeneration(const MonthlySimulationData &simData) const {
  PROFILE_FUNCTION();
  std::vector<EndUses> allResults;
  allResults.reserve(MONTHS_IN_YEAR);

  double floor_area = structure.floorArea();
  double energy_factor =
      (floor_area > 0) ? (1.0 / (floor_area * KILOWATTHOURS_TO_MEGAJOULES)) : 0.0;
  double area_factor = (floor_area > 0) ? (1.0 / floor_area) : 0.0;

  // Plug load factors
  double frac_hrs_wk_day = simData.scheduleData.frac_hrs_wk_day;
  double E_plug_elec_avg =
      building.electricApplianceHeatGainOccupied() * frac_hrs_wk_day +
      building.electricApplianceHeatGainUnoccupied() * (UNITY_FRACTION - frac_hrs_wk_day);
  double E_plug_gas_avg =
      building.gasApplianceHeatGainOccupied() * frac_hrs_wk_day +
      building.gasApplianceHeatGainUnoccupied() * (UNITY_FRACTION - frac_hrs_wk_day);

  for (int i = 0; i < MONTHS_IN_YEAR; i++) {
    double Eelec_ht = simData.v_Qelec_ht[i] * energy_factor;
    double Eelec_cl = simData.v_Qcl_elec_tot[i] * energy_factor;
    double Eelec_int_lt = simData.v_Q_illum_tot[i] * area_factor;
    double Eelec_ext_lt = simData.v_Q_illum_ext_tot[i] * area_factor;
    double Eelec_fan = simData.v_Qfan_tot[i];
    double Eelec_pump = simData.v_Q_pump_tot[i] * energy_factor;
    double Eelec_plug = HOURS_IN_MONTH[i] * E_plug_elec_avg * WATTS_TO_KILOWATTS;
    double Eelec_dhw = simData.v_Q_dhw_elec[i] * area_factor;

    double Egas_ht = simData.v_Qgas_ht[i] * energy_factor;
    double Egas_cl = simData.v_Qcl_gas_tot[i] * energy_factor;
    double Egas_plug = HOURS_IN_MONTH[i] * E_plug_gas_avg * WATTS_TO_KILOWATTS;
    double Egas_dhw = simData.v_Q_dhw_gas[i] * area_factor;

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
    eu.addEndUse(Eelec_ht, EndUseFuelType::Electricity, EndUseCategoryType::Heating);
    eu.addEndUse(Eelec_cl, EndUseFuelType::Electricity, EndUseCategoryType::Cooling);
    eu.addEndUse(Eelec_int_lt, EndUseFuelType::Electricity, EndUseCategoryType::InteriorLights);
    eu.addEndUse(Eelec_ext_lt, EndUseFuelType::Electricity, EndUseCategoryType::ExteriorLights);
    eu.addEndUse(Eelec_fan, EndUseFuelType::Electricity, EndUseCategoryType::Fans);
    eu.addEndUse(Eelec_pump, EndUseFuelType::Electricity, EndUseCategoryType::Pumps);
    eu.addEndUse(Eelec_plug, EndUseFuelType::Electricity, EndUseCategoryType::InteriorEquipment);
    eu.addEndUse(0, EndUseFuelType::Electricity, EndUseCategoryType::ExteriorEquipment);
    eu.addEndUse(Eelec_dhw, EndUseFuelType::Electricity, EndUseCategoryType::WaterSystems);

    eu.addEndUse(Egas_ht, EndUseFuelType::Gas, EndUseCategoryType::Heating);
    eu.addEndUse(Egas_cl, EndUseFuelType::Gas, EndUseCategoryType::Cooling);
    eu.addEndUse(Egas_plug, EndUseFuelType::Gas, EndUseCategoryType::InteriorEquipment);
    eu.addEndUse(Egas_dhw, EndUseFuelType::Gas, EndUseCategoryType::WaterSystems);
    allResults.push_back(eu);
#endif
  }
  return allResults;
}
} // namespace openstudio::isomodel
