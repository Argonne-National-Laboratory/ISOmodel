// HourlyModel.cpp
//
// REFACTORING: ISO STANDARD ALIGNMENT & PERFORMANCE
// - Renamed A_floor_inv -> invFloorArea
// - Replaced harmonic mean divisions with multiplication (algebraic
// simplification).
// - Removed smallEpsilon where algebra safely handles zero values.
// - Reused member vectors to eliminate heap allocation in main loop.

#include "HourlyModel.hpp"

#include "Constants.hpp"
#include "EpwData.hpp"
#include "Profiler.hpp"
#include "SolarRadiation.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>
#include <vector>

namespace openstudio::isomodel {

HourlyModel::HourlyModel() noexcept
    : invFloorArea(0), RHO_CP_AIR_277(RHO_CP_AIR_IN_WATT_HOURS), m_I_sol_max(0),
      m_Cp_air_pressure(0), m_theta_ve_preheat(0), m_eta_ve_rec(0), m_phi_fan_spec(0),
      m_A_nat_inv(0), m_f_phi_int_L(0), m_f_phi_sol_air(0), m_f_phi_int_air(0), win_floor_ratio(0) {

  // Safe zero initialization
  A_nla_ms.fill(0);
  A_nla.fill(0);
  A_sol_ms.fill(0);
  A_sol.fill(0);
  H_tot.fill(0);
  H_win.fill(0);
  A_nla_ms_norm.fill(0);
  f_light_ratio.fill(0);
  f_light_shade_reduction.fill(0);
  A_sol_ms_norm.fill(0);
  f_sol_ratio.fill(0);
  f_sol_shade_reduction.fill(0);
  precalc_nla_shading.fill(0);
  precalc_solar_shading.fill(0);
}

// Destructor is defaulted in header

std::vector<EndUses> HourlyModel::simulate(bool aggregateByMonth) {
  PROFILE_FUNCTION();
  initialize(); // Builds schedules and pre-calculates physics

  double theta_m_prev = DEFAULT_INITIAL_TEMP; // T_m,t-1
  double theta_air = DEFAULT_INITIAL_TEMP;    // Theta_air

  // -----------------------------------------------------------
  // OPTIMIZATION: SOLAR CACHING
  // -----------------------------------------------------------
  // Check if we can reuse previous solar calculations.
  // This prevents expensive trigonometry re-calculation on repeated calls.
  if (m_cachedSolarRadiation.empty() || m_lastEpwData != m_epwData) {
    TimeFrame frame;
    SolarRadiation pos(&frame, m_epwData.get());
    pos.Calculate();
    m_cachedSolarRadiation = pos.eglobeFlat();
    m_lastEpwData = m_epwData;
  }

  // Use the cached vector
  const std::vector<double> &eglobeFlat = m_cachedSolarRadiation;
  // -----------------------------------------------------------

  const auto &data = m_epwData->dataRef();
  const std::vector<double> &egh = data[EGH];

  // OPTIMIZATION: Reuse Member Vectors (No Allocation)
  if (m_phi_H_nd.size() != HOURS_IN_YEAR) {
    m_phi_H_nd.resize(HOURS_IN_YEAR);
    m_phi_C_nd.resize(HOURS_IN_YEAR);
    m_phi_int_L.resize(HOURS_IN_YEAR);
    m_phi_ext_L.resize(HOURS_IN_YEAR);
    m_phi_fan.resize(HOURS_IN_YEAR);
    m_phi_pump.resize(HOURS_IN_YEAR);
    m_phi_int_App.resize(HOURS_IN_YEAR);
    m_phi_ext_App.resize(HOURS_IN_YEAR);
    m_phi_dhw.assign(HOURS_IN_YEAR, 0.0);
  } else {
    std::ranges::fill(m_phi_dhw, 0.0);
  }

  // Cache loop constants
  const double heat_dT_supp = m_heating.dT_supp_ht();
  const double cool_dT_supp = m_cooling.dT_supp_cl();
  const double heat_occ_sp = m_heating.temperatureSetPointOccupied();
  const double cool_occ_sp = m_cooling.temperatureSetPointOccupied();
  const double heat_E_pumps = m_heating.E_pumps();
  const double heat_pumpRed = m_heating.pumpControlReduction();
  const double cool_E_pumps = m_cooling.E_pumps();
  const double cool_pumpRed = m_cooling.pumpControlReduction();
  const double lights_extEnergy = m_lights.exteriorEnergy();
  const double fan_power_factor = m_phi_fan_spec * (1.0 / 3.6); // Convert to W/(m^3/h)

  // Optimization: Pre-calculate pump powers for efficiency in loop
  const double pump_cool_power_active = cool_E_pumps * cool_pumpRed;
  const double pump_heat_power_active = heat_E_pumps * heat_pumpRed;
  const double _RHO_CP_AIR_IN_WATT_HOURS = RHO_CP_AIR_IN_WATT_HOURS;

  for (int i = 0; i < HOURS_IN_YEAR; ++i) {
    const HourlyCache &cache = m_hourlyData[i];
    double theta_e = cache.theta_e;

    // ISO 13790 10.4.2: Internal heat gains from appliances (\Phi_{int,A})
    m_phi_ext_App[i] = cache.sched_ext_equip * invFloorArea;
    m_phi_int_App[i] = cache.sched_phi_int_App;

    // 1. Gains calculation
    // Calculates \Phi_{int} (ISO 13790 10.2.2 eq. 35) and \Phi_{sol} (11.3.2
    // eq. 43) INLINED
    GainsResult gains =
        calculateGains(std::span<const double>(&eglobeFlat[i * 8], 8), cache, m_phi_int_App[i]);
    m_phi_int_L[i] = gains.phi_int_L;

    // 2. Airflow (Using Pre-calculated Physics from Initialize)
    // ISO 15242 calculations for wind and stack effect
    // INLINED
    AirFlowResult flow = calculateAirFlows(theta_air, cache);

    // 3. Solve RC Network (Unconditional Linear)
    // Solves the 5R1C network described in ISO 13790 Annex C
    // Calculates \Phi_{HC,nd} (Heating/Cooling need)
    // INLINED
    double phi_HC_nd = solveThermalBalance(
        theta_e, flow.theta_ent, gains.phi_ia, gains.phi_int, gains.phi_sol, flow.H_ve, flow.H_tr_1,
        cache.sched_theta_H_set, cache.sched_theta_C_set, theta_m_prev, theta_air);

    m_phi_H_nd[i] = std::max(0.0, phi_HC_nd);
    m_phi_C_nd[i] = std::max(0.0, -phi_HC_nd);

    // 4. Auxiliary Calculations
    double q_ve_mech = cache.sched_q_ve_mech * KILOWATTHOURS_TO_MEGAJOULES *
                       invFloorArea; // Convert to energy units

    // Calculate Air Volume for fans (V_{air}) based on heating/cooling delivery
    // needs Using RHO_CP_AIR_277 (Wh/m3K)
    double V_air = std::max(
        {q_ve_mech,
         m_phi_H_nd[i] / (((heat_occ_sp + heat_dT_supp) - theta_air) * _RHO_CP_AIR_IN_WATT_HOURS +
                          SAFE_EPSILON),
         m_phi_C_nd[i] / ((theta_air - (cool_occ_sp - cool_dT_supp)) * _RHO_CP_AIR_IN_WATT_HOURS +
                          SAFE_EPSILON)});

    // Fan energy: V_{air} * specific fan power
    m_phi_fan[i] = V_air * fan_power_factor;

    // OPTIMIZATION: Dynamic Pump Energy (Branching prediction)
    m_phi_pump[i] = (m_phi_C_nd[i] > 0) ? pump_cool_power_active
                                        : ((m_phi_H_nd[i] > 0) ? pump_heat_power_active : 0.0);

    // Exterior lighting (only when sun is down)
    m_phi_ext_L[i] =
        (cache.I_sol_gh > 0) ? 0.0 : (lights_extEnergy * cache.sched_ext_light * invFloorArea);
  }

  return processResults(aggregateByMonth);
}

// -------------------------------------------------------------------------
// INLINE HELPER FUNCTIONS
// -------------------------------------------------------------------------

inline GainsResult HourlyModel::calculateGains(std::span<const double> curSolar,
                                               const HourlyCache &cache,
                                               double phi_int_App) noexcept {
  PROFILE_FUNCTION();

  GainsResult res;
  double lightingLevelSum = 0.0;
  res.phi_sol = 0.0;
  const double I_max = m_I_sol_max;

  // ISO 13790 11.3.2 eq. 43: \Phi_{sol,k} (Solar gains)
  // Includes movable shading logic (switching between g_gl and g_gl+sh)
  for (int k = 0; k < 8; ++k) {
    double I_k = curSolar[k];
    // Optimized min check
    double I_cl = std::min(I_k, I_max);
    lightingLevelSum += I_k * (f_light_ratio[k] + precalc_nla_shading[k] * I_cl);
    res.phi_sol += I_k * (f_sol_ratio[k] + precalc_solar_shading[k] * I_cl);
  }
  {
    // Roof (EGH) - Use I_sol_gh from cache
    double I_k = cache.I_sol_gh;
    double I_cl = std::min(I_k, I_max);
    lightingLevelSum += I_k * (f_light_ratio[8] + precalc_nla_shading[8] * I_cl);
    res.phi_sol += I_k * (f_sol_ratio[8] + precalc_solar_shading[8] * I_cl);
  }

  // ISO 13790 10.4.3: \Phi_{int,L} (Lighting Gains)
  double lightingLevel = lightingLevelSum * m_A_nat_inv;
  double f_L =
      std::max(0.0,
               f_L_max * (1.0 - lightingLevel / (I_lux_nat + SAFE_EPSILON))); // Use epsilon for
                                                                              // small divisor
  res.phi_int_L = (f_L * f_A_nat + (1.0 - f_A_nat) * f_L_max) * cache.sched_phi_int_L;

  // ISO 13790 10.2.2 eq. 35: \Phi_{int} (Total internal gains)
  res.phi_int = phi_int_App + (res.phi_int_L * m_f_phi_int_L);

  // ISO 13790 C.2 eq. C.1: \Phi_{ia} (Gains split to air node)
  res.phi_ia = m_f_phi_sol_air * res.phi_sol + m_f_phi_int_air * res.phi_int;

  return res;
}

inline AirFlowResult HourlyModel::calculateAirFlows(double theta_air,
                                                    const HourlyCache &cache) noexcept {
  PROFILE_FUNCTION();

  AirFlowResult res;
  double theta_e = cache.theta_e;
  // calculate absolute delta T with a minimum value of 1E-5 because there is
  // always some difference between indoor and outdoor temperature in reality
  double absDT = std::max(std::abs(theta_e - theta_air), 1e-5);

  // ISO 15242 6.7.1 Step 1: q_{stack} (Stack effect)
  double q_ve_stack =
      STACK_FACTOR * q_ve_4Pa * fastPow23(EFFECTIVE_STACK_HEIGHT_FRACTION * H_z * absDT);

  // Promote float physics to double
  double q_ve_wind = cache.q_ve_wind;
  double q_ve_diff = cache.q_ve_diff;

  // ISO 15242 6.7.1 Step 2: q_{exfiltration}
  // Protection needed here: stack and wind could both be zero
  double q_ve_sw = q_ve_stack + q_ve_wind + SAFE_EPSILON; // Use epsilon for small additive
                                                          // factor
  double q_ve_exf =
      std::max(0.0, std::max(q_ve_stack, q_ve_wind) -
                        std::abs(q_ve_diff) * (Q_INFIL_STACT_FRACTION * q_ve_stack +
                                               Q_INFIL_WIND_FRACTION * q_ve_wind / q_ve_sw));

  // ISO 15242 6.7.2: q_{ent} (Total entering air)
  double q_ve_ent = std::max(0.0, (double)q_ve_diff) + q_ve_exf + cache.q_ve_mech_sup;

  // ISO 13790 9.3: \theta_{sup} (Supply temperature)
  // Protection needed here: q_ve_ent can be zero
  res.theta_ent = (theta_e * (std::max(0.0, (double)q_ve_diff) + q_ve_exf) +
                   cache.theta_sup * cache.q_ve_mech_sup) /
                  (q_ve_ent + SAFE_EPSILON); // Use epsilon for small
                                             // additive factor

  // ISO 13790 9.3.1 eq. 21: H_{ve} (Ventilation heat transfer coefficient)
  res.H_ve = RHO_CP_AIR_IN_WATT_HOURS * q_ve_ent;

  // ISO 13790 C.3 eq. C.6: H_{tr,1}
  // OPTIMIZATION: Harmonic Mean Simplification (A*B)/(A+B)
  // Replaces: 1.0 / (1.0 / (res.H_ve + smallEpsilon) + 1.0 / H_tr_is);
  // Removes 2 divisions and redundant epsilon
  res.H_tr_1 = (res.H_ve * H_tr_is) / (res.H_ve + H_tr_is);

  return res;
}

inline double HourlyModel::solveThermalBalance(double theta_e, double theta_ent, double phi_ia,
                                               double phi_int, double phi_sol, double H_ve,
                                               double H_tr_1, double theta_H_set,
                                               double theta_C_set, double &theta_m_prev,
                                               double &theta_air) noexcept {
  PROFILE_FUNCTION();

  // ISO 13790 C.3 eq. C.7: H_{tr,2}
  double H_tr_2 = H_tr_1 + H_tr_w;

  // ISO 13790 C.3 eq. C.9: H_{tr,3}
  // OPTIMIZATION: Harmonic Mean Simplification (A*B)/(A+B)
  // Replaces: 1.0 / (1.0 / H_tr_2 + 1.0 / H_ms)
  // No epsilon needed as H_tr_2 and H_ms are positive.
  double H_tr_3 = (H_tr_2 * H_ms) / (H_tr_2 + H_ms);

  double H_tr_3_H_em = 0.5 * (H_tr_3 + H_em);
  double H_tr_3_H_tr_2 = H_tr_3 / H_tr_2;

  // OPTIMIZATION: Common Divisor Extraction
  // (H_tr_is + H_ve) is used twice.
  // H_tr_is always > 0, so no epsilon needed.
  double inv_H_tr_is_plus_H_ve = 1.0 / (H_tr_is + H_ve);

  // ALGEBRAIC LIMIT:
  // Term (H_tr_1 / H_ve) simplifies algebraically to (H_tr_is / (H_ve +
  // H_tr_is)) We call this ratio R_ve_tr. It allows calculation even if H_ve is
  // 0.
  double R_ve_tr = H_tr_is * inv_H_tr_is_plus_H_ve;

  double d_phim_dp = H_tr_3_H_tr_2 * R_ve_tr; // d(phim)/dP (using R_ve_tr instead of H_tr_1/H_ve)

  double Cm_units = C_m / KILOWATTHOURS_TO_MEGAJOULES;
  double Cm_plus_H = Cm_units + H_tr_3_H_em;
  double d_thetam_dp = d_phim_dp / Cm_plus_H;

  double H_denom = H_ms + H_tr_w + H_tr_1;

  // Replaced (H_tr_1 * H_ve_inv) with R_ve_tr
  double d_thetas_dp = (H_ms * 0.5 * d_thetam_dp + R_ve_tr) / H_denom;

  // Slope d(\theta_{air})/d(\phi)
  double d_theta_air_dp = (H_tr_is * d_thetas_dp + 1.0) * inv_H_tr_is_plus_H_ve;

  // ISO 13790 C.2 eq. C.3: \Phi_{st} (Generalized)
  double phi_st = p_rs_sol * phi_sol + p_rs_int * phi_int;
  // ISO 13790 C.2 eq. C.2: \Phi_{m} (Generalized)
  double phi_m = p_rm_sol * phi_sol + p_rm_int * phi_int;
  double mid = phi_st + H_tr_w * theta_e + H_tr_1 * theta_ent;

  // ISO 13790 C.3 eq. C.5: \Phi_{mtot}
  double phi_mtot = phi_m + H_em * theta_e + H_tr_3_H_tr_2 * mid + d_phim_dp * phi_ia;

  // ISO 13790 C.3 eq. C.4: \theta_{m,t}
  double theta_m = (theta_m_prev * (Cm_units - H_tr_3_H_em) + phi_mtot) / Cm_plus_H;

  // ISO 13790 C.3 eq. C.10: \theta_{s}
  // Replaced (H_tr_1 * phi_ia * H_ve_inv) with (R_ve_tr * phi_ia)
  double theta_s = (H_ms * 0.5 * (theta_m_prev + theta_m) + mid + R_ve_tr * phi_ia) / H_denom;

  // ISO 13790 C.3 eq. C.11: \theta_{air} (Indoor air temperature)
  double theta_air_0 = (H_tr_is * theta_s + H_ve * theta_ent + phi_ia) * inv_H_tr_is_plus_H_ve;

  // Calculate required Heating/Cooling power (\Phi_{HC,nd})
  double phi_HC_nd = 0.0;
  if (theta_air_0 < theta_H_set) {
    phi_HC_nd = (theta_H_set - theta_air_0) / d_theta_air_dp;
    if (phi_HC_nd < 0)
      phi_HC_nd = 0;
  } else if (theta_air_0 > theta_C_set) {
    phi_HC_nd = (theta_C_set - theta_air_0) / d_theta_air_dp;
    if (phi_HC_nd > 0)
      phi_HC_nd = 0;
  }

  // Update State for next step
  double phi_m_final = phi_mtot + d_phim_dp * phi_HC_nd;
  theta_m_prev = (theta_m_prev * (Cm_units - H_tr_3_H_em) + phi_m_final) / Cm_plus_H;
  theta_air = theta_air_0 + d_theta_air_dp * phi_HC_nd;

  return phi_HC_nd;
}

std::vector<EndUses> HourlyModel::processResults(bool aggregateByMonth) {
  PROFILE_FUNCTION();

  double phi_H_tot = std::accumulate(m_phi_H_nd.begin(), m_phi_H_nd.end(), 0.0);
  double phi_C_tot = std::accumulate(m_phi_C_nd.begin(), m_phi_C_nd.end(),
                                     0.0); // Total cooling need
  double f_H = std::max(phi_H_tot / (phi_C_tot + phi_H_tot + SAFE_EPSILON),
                        0.1); // Use epsilon for small additive factor

  double s_ht =
      (1.0 + m_heating.hvacLossFactor() + m_heating.hotcoldWasteFactor() / f_H) / m_heating.efficiency();
  double s_cl =
      (1.0 + m_cooling.hvacLossFactor() + m_heating.hotcoldWasteFactor() / (1.0 - f_H)) / m_cooling.cop();

  std::vector<EndUses> results;
  if (!aggregateByMonth)
    results.reserve(HOURS_IN_YEAR);

  auto mapToEU = [&](EndUses &eu, double h, double c, double il, double el, double fn, double pm,
                     double pi, double pe, double dw) {
    double total_heat_req = h * s_ht * WATTS_TO_KILOWATTS;
    double elec_ht = 0.0;
    double gas_ht = 0.0;
    if (m_heating.energyType() == FuelType::Electric) {
      elec_ht = total_heat_req;
    } else {
      gas_ht = total_heat_req;
    }

    eu.addEndUse(0, elec_ht);
    eu.addEndUse(1, c * s_cl * WATTS_TO_KILOWATTS);
    eu.addEndUse(2, il * WATTS_TO_KILOWATTS);
    eu.addEndUse(3, el * WATTS_TO_KILOWATTS);
    eu.addEndUse(4, fn * WATTS_TO_KILOWATTS);
    eu.addEndUse(5, pm * WATTS_TO_KILOWATTS);
    eu.addEndUse(6, pi * WATTS_TO_KILOWATTS);
    eu.addEndUse(7, pe * WATTS_TO_KILOWATTS);
    eu.addEndUse(8, dw * WATTS_TO_KILOWATTS);
    eu.addEndUse(9, gas_ht);
  };

  if (aggregateByMonth) {
    for (int m = 0; m < MONTHS_IN_YEAR; ++m) {
      EndUses eu;
      std::array<double, 9> sums{};
      for (int i = MONTH_END_HOURS[m]; i < MONTH_END_HOURS[m + 1]; ++i) {
        sums[0] += m_phi_H_nd[i];
        sums[1] += m_phi_C_nd[i];
        sums[2] += m_phi_int_L[i];
        sums[3] += m_phi_ext_L[i];
        sums[4] += m_phi_fan[i];
        sums[5] += m_phi_pump[i];
        sums[6] += m_phi_int_App[i];
        sums[7] += m_phi_ext_App[i];
        sums[8] += m_phi_dhw[i];
      }
      mapToEU(eu, sums[0], sums[1], sums[2], sums[3], sums[4], sums[5], sums[6], sums[7], sums[8]);
      results.push_back(eu);
    }
  } else {
    for (int i = 0; i < HOURS_IN_YEAR; ++i) {
      EndUses eu;
      mapToEU(eu, m_phi_H_nd[i], m_phi_C_nd[i], m_phi_int_L[i], m_phi_ext_L[i], m_phi_fan[i],
              m_phi_pump[i], m_phi_int_App[i], m_phi_ext_App[i], m_phi_dhw[i]);
      results.push_back(eu);
    }
  }
  return results;
}

void HourlyModel::initialize() {
  PROFILE_FUNCTION();
  double floorArea = m_structure.floorArea();

  // OPTIMIZATION 1: Inverse Floor Area
  // Calculate 1.0 / floorArea once.
  invFloorArea = 1.0 / (floorArea + SAFE_EPSILON);

  m_I_sol_max = m_structure.irradianceForMaxShadingUse();
  m_Cp_air_pressure = m_ventilation.dCp();
  m_theta_ve_preheat = m_ventilation.ventPreheatDegC();
  m_eta_ve_rec = m_ventilation.heatRecoveryEfficiency();
  m_phi_fan_spec = m_ventilation.fanPower();

  m_f_phi_int_L = m_lights.elecInternalGains();
  m_f_phi_sol_air = m_simSettings.phiSolFractionToAirNode();
  m_f_phi_int_air = m_simSettings.phiIntFractionToAirNode();

  auto lightingOccupancySensorDimmingFraction = m_building.lightingOccupancySensor();
  auto daylightSensorDimmingFraction = m_lights.dimmingFraction();

  if (lightingOccupancySensorDimmingFraction < 1.0 && daylightSensorDimmingFraction < 1.0) {
    f_L_max = m_lights.presenceAutoAd();
    I_lux_nat = m_lights.presenceAutoLux();
  } else if (lightingOccupancySensorDimmingFraction < 1.0) {
    f_L_max = m_lights.presenceSensorAd();
    I_lux_nat = m_lights.presenceSensorLux();
  } else if (daylightSensorDimmingFraction < 1.0) {
    f_L_max = m_lights.automaticAd();
    I_lux_nat = m_lights.automaticLux();
  } else {
    f_L_max = m_lights.manualSwitchAd();
    I_lux_nat = m_lights.manualSwitchLux();
  }

  f_A_nat = std::max(0.0001, m_lights.naturallyLightedArea()) * invFloorArea;
  m_A_nat_inv = (f_A_nat > 0) ? (LIGHTING_LEVEL_COEFF / f_A_nat) : 0.0;

  // Optimization: Solar Geometry Ratio
  // Using pre-calculated inverse floor area
  win_floor_ratio = m_structure.windowArea().empty() ? 0.0 : m_structure.windowArea()[0] * invFloorArea;

  for (int i = 0; i != NUM_TOTAL_SURFACES; ++i) {
    structureCalculations(m_structure.windowShadingDevice()[i], m_structure.wallArea()[i],
                          m_structure.windowArea()[i], m_structure.wallUniform()[i],
                          m_structure.windowUniform()[i], m_structure.wallSolarAbsorption()[i],
                          m_structure.windowShadingCorrectionFactor()[i],
                          m_structure.windowNormalIncidenceSolarEnergyTransmittance()[i], i);

    A_nla_ms_norm[i] = A_nla_ms[i] * invFloorArea;
    f_light_ratio[i] = A_nla[i] * invFloorArea;
    f_light_shade_reduction[i] = A_nla_ms_norm[i] - f_light_ratio[i];
    A_sol_ms_norm[i] = A_sol_ms[i] * invFloorArea;
    f_sol_ratio[i] = A_sol[i] * invFloorArea;
    f_sol_shade_reduction[i] = A_sol_ms_norm[i] - f_sol_ratio[i];
  }

  f_sh_use = m_structure.shadingFactorAtMaxUse() / m_structure.irradianceForMaxShadingUse();

  for (int i = 0; i < NUM_TOTAL_SURFACES; ++i) {
    precalc_nla_shading[i] = f_sh_use * f_light_shade_reduction[i];
    precalc_solar_shading[i] = f_sol_shade_reduction[i] * f_sh_use;
  }

  q_ve_4Pa = std::max(0.000001,
                      (N50_TO_Q4 * (m_ventilation.n50() * (floorArea * m_structure.buildingHeight()))) *
                          invFloorArea);

  h_ms = m_simSettings.hci() + m_simSettings.hri() * H_MS_FACTOR;
  h_is = 1.0 / (1.0 / m_simSettings.hci() - 1.0 / h_ms);
  H_tr_is = h_is * m_structure.totalAreaPerFloorArea();

  const auto &wallAreas = m_structure.wallArea();
  double A_wall_total = std::accumulate(wallAreas.begin(), wallAreas.end(), 0.0);

  C_m = (m_structure.interiorHeatCapacity() +
         (m_structure.wallHeatCapacity() * A_wall_total * invFloorArea)) /
        1000.0;

  // OPTIMIZATION 2: Mass Area (A_m) Interpolation
  // Moved out of hourly loop because C_m is constant.
  if (C_m > VERY_HEAVY)
    A_m = 3.5;
  else if (C_m > HEAVY)
    A_m = std::lerp(3.0, 3.5, (C_m - HEAVY) / (VERY_HEAVY - HEAVY));
  else if (C_m > MEDIUM)
    A_m = std::lerp(2.5, 3.0, (C_m - MEDIUM) / (HEAVY - MEDIUM));
  else
    A_m = 2.5;

  double H_win_sum = 0.0, H_wall_sum_total = 0.0;
  for (int i = 0; i != NUM_TOTAL_SURFACES; ++i) {
    H_win_sum += H_win[i];
    H_wall_sum_total += H_tot[i] - H_win[i];
  }
  H_tr_w = H_win_sum * invFloorArea;

  p_rs =
      (m_structure.totalAreaPerFloorArea() - A_m - H_tr_w / h_ms) / m_structure.totalAreaPerFloorArea();
  p_rs_int = (1.0 - m_simSettings.phiIntFractionToAirNode()) * p_rs;
  p_rs_sol = (1.0 - m_simSettings.phiSolFractionToAirNode()) * p_rs;
  p_rm = A_m / m_structure.totalAreaPerFloorArea();
  p_rm_int = (1.0 - m_simSettings.phiIntFractionToAirNode()) * p_rm;
  p_rm_sol = (1.0 - m_simSettings.phiSolFractionToAirNode()) * p_rm;

  H_ms = h_ms * A_m;
  // OPTIMIZATION: H_em Calculation
  H_em = 1.0 / (1.0 / std::max(H_wall_sum_total * invFloorArea, 0.000001) - 1.0 / H_ms);

  H_z = std::max(0.1, m_ventilation.hzone());
  f_ve_mech_sup = std::max(0.00001, m_ventilation.fanControlFactor());

  m_hourlyData.resize(HOURS_IN_YEAR); // Ensure m_hourlyData is sized

  const auto &data = m_epwData->dataRef();

  const std::vector<double> &wind = data[WSPD];
  const std::vector<double> &temp = data[DBT];
  const std::vector<double> &egh = data[EGH];

  TimeFrame frame;
  for (int i = 0; i < HOURS_IN_YEAR; ++i) {
    int h = frame.Hour[i];
    int d = frame.DayOfWeek[i];
    HourlyCache &c = m_hourlyData[i];

    // Populate schedule-related fields from the returned scheduleData
    c.sched_q_ve_mech = m_preloadedScheduleData[i].sched_q_ve_mech;
    c.sched_ext_equip = m_preloadedScheduleData[i].sched_ext_equip;
    c.sched_phi_int_App = m_preloadedScheduleData[i].sched_phi_int_App;
    c.sched_ext_light = m_preloadedScheduleData[i].sched_ext_light;
    c.sched_phi_int_L = m_preloadedScheduleData[i].sched_phi_int_L;
    c.sched_theta_H_set = m_preloadedScheduleData[i].sched_theta_H_set;
    c.sched_theta_C_set = m_preloadedScheduleData[i].sched_theta_C_set;

    c.theta_e = (float)temp[i];
    c.I_sol_gh = (float)egh[i];

    c.q_ve_wind =
        (float)(WIND_FACTOR * q_ve_4Pa * fastPow23(m_Cp_air_pressure * wind[i] * wind[i]));

    double q_ve = c.sched_q_ve_mech * KILOWATTHOURS_TO_MEGAJOULES * invFloorArea;
    c.q_ve_mech_sup = (float)(q_ve * f_ve_mech_sup);
    c.q_ve_diff = (float)(-(c.q_ve_mech_sup - q_ve));
    c.theta_sup = (float)(std::max(m_theta_ve_preheat,
                                   std::lerp(temp[i], DEFAULT_INITIAL_TEMP, m_eta_ve_rec)));
  }
}

void HourlyModel::setPreloadedScheduleData(
    std::vector<schedules::ScheduleDataForHourlyCache> data) {
  m_preloadedScheduleData = std::move(data);
}

inline void HourlyModel::structureCalculations(double SHGC, double A_wall, double A_win,
                                               double U_wall, double U_win, double alpha_wall,
                                               double F_sh_with, double F_sh_without,
                                               int direction) {
  PROFILE_FUNCTION();

  double WindowT = SHGC / SHGC_CLEAR_GLASS;
  A_nla_ms[direction] = A_win * WindowT;
  A_nla[direction] = A_win * WindowT;
  A_sol_ms[direction] = A_wall * (alpha_wall * U_wall * m_structure.R_se()) + A_win * F_sh_with;
  A_sol[direction] = A_wall * (alpha_wall * U_wall * m_structure.R_se()) + A_win * F_sh_without;
  H_tot[direction] = A_wall * U_wall + A_win * U_win;
  H_win[direction] = A_win * U_win;
}
} // namespace openstudio::isomodel
