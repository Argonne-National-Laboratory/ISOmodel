/*
 * HourlyModel.cpp
 *
 * REFACTORING: ISO STANDARD ALIGNMENT & PERFORMANCE
 * - Renamed A_floor_inv -> invFloorArea
 * - Replaced harmonic mean divisions with multiplication (algebraic simplification).
 * - Removed smallEpsilon where algebra safely handles zero values.
 * - Reused member vectors to eliminate heap allocation in main loop.
 */

#include "Constants.hpp"
#include "HourlyModel.hpp"
#include "SolarRadiation.hpp" 
#include <algorithm>
#include <numeric>
#include <cmath>
#include <cfloat> 
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

namespace openstudio {
    namespace isomodel {

        void printMatrix(const char* matName, double* mat, unsigned int dim1, unsigned int dim2) {}

        HourlyModel::HourlyModel() : invFloorArea(0), rhoCpAir_277(rhoCpAirWh), m_I_sol_max(0), m_Cp_air_pressure(0), 
            m_theta_ve_preheat(0), m_eta_ve_rec(0), m_phi_fan_spec(0), m_A_nat_inv(0),
            m_f_phi_int_L(0), m_f_phi_sol_air(0), m_f_phi_int_air(0), win_floor_ratio(0) {
            
            // Safe zero initialization
            A_nla_ms.fill(0); A_nla.fill(0); A_sol_ms.fill(0); A_sol.fill(0); H_tot.fill(0); H_win.fill(0);
            A_nla_ms_norm.fill(0); f_light_ratio.fill(0); f_light_shade_reduction.fill(0);
            A_sol_ms_norm.fill(0); f_sol_ratio.fill(0); f_sol_shade_reduction.fill(0);
            precalc_nla_shading.fill(0); precalc_solar_shading.fill(0);
        }
        
        HourlyModel::~HourlyModel() {}

        std::vector<EndUses> HourlyModel::simulate(bool aggregateByMonth)
        {
            initialize(); // Builds schedules and pre-calculates physics

            double theta_m_prev = 20.0; // T_m,t-1
            double theta_air = 20.0;    // Theta_air

            // -----------------------------------------------------------
            // OPTIMIZATION: SOLAR CACHING
            // -----------------------------------------------------------
            // Check if we can reuse previous solar calculations.
            // This prevents expensive trigonometry re-calculation on repeated calls.
            if (m_cachedSolarRadiation.empty() || m_lastEpwData != epwData) {
                TimeFrame frame; 
                SolarRadiation pos(&frame, epwData.get());
                pos.Calculate();
                m_cachedSolarRadiation = pos.eglobeFlat();
                m_lastEpwData = epwData;
            }

            // Use the cached vector
            const std::vector<double>& eglobeFlat = m_cachedSolarRadiation;
            // -----------------------------------------------------------

            const auto& data = epwData->data();
            const std::vector<double>& egh = data[EGH]; 

            // OPTIMIZATION: Reuse Member Vectors (No Allocation)
            if (m_phi_H_nd.size() != hoursInYear) {
                m_phi_H_nd.resize(hoursInYear); m_phi_C_nd.resize(hoursInYear);
                m_phi_int_L.resize(hoursInYear); m_phi_ext_L.resize(hoursInYear);
                m_phi_fan.resize(hoursInYear); m_phi_pump.resize(hoursInYear);
                m_phi_int_App.resize(hoursInYear); m_phi_ext_App.resize(hoursInYear);
                m_phi_dhw.assign(hoursInYear, 0.0);
            } else {
                std::fill(m_phi_dhw.begin(), m_phi_dhw.end(), 0.0);
            }

            // Cache loop constants
            const double heat_dT_supp = heating.dT_supp_ht();
            const double cool_dT_supp = cooling.dT_supp_cl();
            const double heat_occ_sp = heating.temperatureSetPointOccupied();
            const double cool_occ_sp = cooling.temperatureSetPointOccupied();
            const double heat_E_pumps = heating.E_pumps();
            const double heat_pumpRed = heating.pumpControlReduction();
            const double cool_E_pumps = cooling.E_pumps();
            const double cool_pumpRed = cooling.pumpControlReduction();
            const double lights_extEnergy = lights.exteriorEnergy();
            const double fan_power_factor = m_phi_fan_spec * 0.277778; // Convert to W/(m^3/h)
            
            // Optimization: Pre-calculate pump powers for efficiency in loop
            const double pump_cool_power_active = cool_E_pumps * cool_pumpRed;
            const double pump_heat_power_active = heat_E_pumps * heat_pumpRed;
            const double _rhoCpAirWh = rhoCpAirWh; 

            for (int i = 0; i < hoursInYear; ++i) {
                const HourlyCache& cache = m_hourlyData[i];
                double theta_e = cache.theta_e;

                // ISO 13790 10.4.2: Internal heat gains from appliances (\Phi_{int,A})
                m_phi_ext_App[i] = cache.sched_ext_equip * invFloorArea;
                m_phi_int_App[i] = cache.sched_phi_int_App;

                // 1. Gains calculation 
                // Calculates \Phi_{int} (ISO 13790 10.2.2 eq. 35) and \Phi_{sol} (11.3.2 eq. 43)
                // INLINED
                GainsResult gains = calculateGains(std::span<const double>(&eglobeFlat[i * 8], 8), cache, m_phi_int_App[i]);
                m_phi_int_L[i] = gains.phi_int_L;

                // 2. Airflow (Using Pre-calculated Physics from Initialize)
                // ISO 15242 calculations for wind and stack effect
                // INLINED
                AirFlowResult flow = calculateAirFlows(theta_air, cache);

                // 3. Solve RC Network (Unconditional Linear)
                // Solves the 5R1C network described in ISO 13790 Annex C
                // Calculates \Phi_{HC,nd} (Heating/Cooling need)
                // INLINED
                double phi_HC_nd = solveThermalBalance(theta_e, flow.theta_ent, gains.phi_ia, gains.phi_int, gains.phi_sol, flow.H_ve, flow.H_tr_1,
                    cache.sched_theta_H_set, cache.sched_theta_C_set,
                    theta_m_prev, theta_air);

                m_phi_H_nd[i] = std::max(0.0, phi_HC_nd);
                m_phi_C_nd[i] = std::max(0.0, -phi_HC_nd);

                // 4. Auxiliary Calculations
                double q_ve_mech = cache.sched_q_ve_mech * kWh2MJ * invFloorArea; // Convert to energy units

                // Calculate Air Volume for fans (V_{air}) based on heating/cooling delivery needs
                // Using rhoCpAir_277 (Wh/m3K)
                double V_air = std::max({ q_ve_mech,
                    m_phi_H_nd[i] / (((heat_occ_sp + heat_dT_supp) - theta_air) * _rhoCpAirWh + DBL_MIN),
                    m_phi_C_nd[i] / ((theta_air - (cool_occ_sp - cool_dT_supp)) * _rhoCpAirWh + DBL_MIN) });

                // Fan energy: V_{air} * specific fan power
                m_phi_fan[i] = V_air * fan_power_factor;
                
                // OPTIMIZATION: Dynamic Pump Energy (Branching prediction)
                m_phi_pump[i] = (m_phi_C_nd[i] > 0) ? pump_cool_power_active :
                              ((m_phi_H_nd[i] > 0) ? pump_heat_power_active : 0.0);
                
                // Exterior lighting (only when sun is down)
                m_phi_ext_L[i] = (cache.I_sol_gh > 0) ? 0.0 : (lights_extEnergy * cache.sched_ext_light * invFloorArea);
            }

            return processResults(aggregateByMonth);
        }

        // -------------------------------------------------------------------------
        // INLINE HELPER FUNCTIONS
        // -------------------------------------------------------------------------

        inline GainsResult HourlyModel::calculateGains(std::span<const double> curSolar,
            const HourlyCache& cache, double phi_int_App) noexcept {
            
            GainsResult res;
            double lightingLevelSum = 0.0;
            res.phi_sol = 0.0;
            const double I_max = m_I_sol_max;

            // ISO 13790 11.3.2 eq. 43: \Phi_{sol,k} (Solar gains)
            // Includes movable shading logic (switching between g_gl and g_gl+sh)
            for (int k = 0; k < 8; ++k) {
                double I_k = curSolar[k];
                // Optimized min check
                double I_cl = (I_k < I_max) ? I_k : I_max;
                lightingLevelSum += I_k * (f_light_ratio[k] + precalc_nla_shading[k] * I_cl);
                res.phi_sol += I_k * (f_sol_ratio[k] + precalc_solar_shading[k] * I_cl);
            }
            {
                // Roof (EGH) - Use I_sol_gh from cache
                double I_k = cache.I_sol_gh;
                double I_cl = (I_k < I_max) ? I_k : I_max;
                lightingLevelSum += I_k * (f_light_ratio[8] + precalc_nla_shading[8] * I_cl);
                res.phi_sol += I_k * (f_sol_ratio[8] + precalc_solar_shading[8] * I_cl);
            }

            // ISO 13790 10.4.3: \Phi_{int,L} (Lighting Gains)
            double lightingLevel = lightingLevelSum * m_A_nat_inv;
            double f_L = std::max(0.0, f_L_max * (1.0 - lightingLevel / (I_lux_nat + DBL_MIN)));
            res.phi_int_L = (f_L * f_A_nat + (1.0 - f_A_nat) * f_L_max) * cache.sched_phi_int_L;
            
            // ISO 13790 10.2.2 eq. 35: \Phi_{int} (Total internal gains)
            res.phi_int = phi_int_App + (res.phi_int_L * m_f_phi_int_L);
            
            // ISO 13790 C.2 eq. C.1: \Phi_{ia} (Gains split to air node)
            res.phi_ia = m_f_phi_sol_air * res.phi_sol + m_f_phi_int_air * res.phi_int;
            
            return res;
        }

        inline AirFlowResult HourlyModel::calculateAirFlows(double theta_air, const HourlyCache& cache) noexcept {
            
            AirFlowResult res;
            double theta_e = cache.theta_e;
            double absDT = std::max(std::fabs(theta_e - theta_air), 1e-5);
            
            // ISO 15242 6.7.1 Step 1: q_{stack} (Stack effect)
            double q_ve_stack = stackFactor * q_ve_4Pa * fastPow23(effectiveStackHeightFraction * H_z * absDT);
            
            // Promote float physics to double
            double q_ve_wind = cache.q_ve_wind;
            double q_ve_diff = cache.q_ve_diff;

            // ISO 15242 6.7.1 Step 2: q_{exfiltration}
            // Protection needed here: stack and wind could both be zero
            double q_ve_sw = q_ve_stack + q_ve_wind + smallEpsilon;
            double q_ve_exf = std::max(0.0, std::max(q_ve_stack, q_ve_wind) - std::fabs(q_ve_diff) * (qInfilStackFraction * q_ve_stack + qInfilWindFraction * q_ve_wind / q_ve_sw));

            // ISO 15242 6.7.2: q_{ent} (Total entering air)
            double q_ve_ent = (q_ve_diff > 0 ? q_ve_diff : 0.0) + q_ve_exf + cache.q_ve_mech_sup;
            
            // ISO 13790 9.3: \theta_{sup} (Supply temperature)
            // Protection needed here: q_ve_ent can be zero
            res.theta_ent = (theta_e * ((q_ve_diff > 0 ? q_ve_diff : 0.0) + q_ve_exf) + cache.theta_sup * cache.q_ve_mech_sup) / (q_ve_ent + smallEpsilon);
            
            // ISO 13790 9.3.1 eq. 21: H_{ve} (Ventilation heat transfer coefficient)
            res.H_ve = rhoCpAirWh * q_ve_ent;
            
            // ISO 13790 C.3 eq. C.6: H_{tr,1}
            // OPTIMIZATION: Harmonic Mean Simplification (A*B)/(A+B)
            // Replaces: 1.0 / (1.0 / (res.H_ve + smallEpsilon) + 1.0 / H_tr_is);
            // Removes 2 divisions and redundant epsilon
            res.H_tr_1 = (res.H_ve * H_tr_is) / (res.H_ve + H_tr_is);
            
            return res;
        }

        inline double HourlyModel::solveThermalBalance(double theta_e, double theta_ent, double phi_ia, double phi_int,
            double phi_sol, double H_ve, double H_tr_1, double theta_H_set,
            double theta_C_set, double& theta_m_prev, double& theta_air) noexcept {
            
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
            // Term (H_tr_1 / H_ve) simplifies algebraically to (H_tr_is / (H_ve + H_tr_is))
            // We call this ratio R_ve_tr. It allows calculation even if H_ve is 0.
            double R_ve_tr = H_tr_is * inv_H_tr_is_plus_H_ve; 
            
            double d_phim_dp = H_tr_3_H_tr_2 * R_ve_tr; // d(phim)/dP (using R_ve_tr instead of H_tr_1/H_ve)
            
            double Cm_units = C_m / kWh2MJ;
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
            double theta_s   = (H_ms * 0.5 * (theta_m_prev + theta_m) + mid + R_ve_tr * phi_ia) / H_denom;
            
            // ISO 13790 C.3 eq. C.11: \theta_{air} (Indoor air temperature)
            double theta_air_0 = (H_tr_is * theta_s + H_ve * theta_ent + phi_ia) * inv_H_tr_is_plus_H_ve;
            
            // Calculate required Heating/Cooling power (\Phi_{HC,nd})
            double phi_HC_nd = 0.0;
            if (theta_air_0 < theta_H_set) {
                phi_HC_nd = (theta_H_set - theta_air_0) / d_theta_air_dp;
                if (phi_HC_nd < 0) phi_HC_nd = 0; 
            } else if (theta_air_0 > theta_C_set) {
                phi_HC_nd = (theta_C_set - theta_air_0) / d_theta_air_dp;
                if (phi_HC_nd > 0) phi_HC_nd = 0; 
            }

            // Update State for next step
            double phi_m_final = phi_mtot + d_phim_dp * phi_HC_nd;
            theta_m_prev = (theta_m_prev * (Cm_units - H_tr_3_H_em) + phi_m_final) / Cm_plus_H;
            theta_air = theta_air_0 + d_theta_air_dp * phi_HC_nd;
            
            return phi_HC_nd;
        }

        std::vector<EndUses> HourlyModel::processResults(bool aggregateByMonth) {
            
            double phi_H_tot = std::accumulate(m_phi_H_nd.begin(), m_phi_H_nd.end(), 0.0);
            double phi_C_tot = std::accumulate(m_phi_C_nd.begin(), m_phi_C_nd.end(), 0.0);
            double f_H = std::max(phi_H_tot / (phi_C_tot + phi_H_tot + DBL_MIN), 0.1);
            
            double s_ht = (1.0 / (1.0 / (1.0 + heating.hvacLossFactor() + heating.hotcoldWasteFactor() / f_H))) / heating.efficiency();
            double s_cl = (1.0 / (1.0 / (1.0 + cooling.hvacLossFactor() + heating.hotcoldWasteFactor() / (1.0 - f_H)))) / cooling.cop();

            bool electricHeat = (heating.energyType() == 1);
            std::vector<EndUses> results;
            if (!aggregateByMonth) results.reserve(hoursInYear);

            auto mapToEU = [&](EndUses& eu, double h, double c, double il, double el, double fn, double pm, double pi, double pe, double dw) {
                double total_heat_req = h * s_ht * W2kW;
                double elec_ht = total_heat_req * electricHeat;
                double gas_ht = total_heat_req - elec_ht;

#ifdef ISOMODEL_STANDALONE
                eu.addEndUse(0, elec_ht); eu.addEndUse(1, c * s_cl * W2kW); eu.addEndUse(2, il * W2kW); eu.addEndUse(3, el * W2kW);
                eu.addEndUse(4, fn * W2kW); eu.addEndUse(5, pm * W2kW); eu.addEndUse(6, pi * W2kW); eu.addEndUse(7, pe * W2kW);
                eu.addEndUse(8, dw * W2kW); eu.addEndUse(9, gas_ht);
#else
                eu.addEndUse(elec_ht, EndUseFuelType::Electricity, EndUseCategoryType::Heating);
                eu.addEndUse(c * s_cl * W2kW, EndUseFuelType::Electricity, EndUseCategoryType::Cooling);
                eu.addEndUse(il * W2kW, EndUseFuelType::Electricity, EndUseCategoryType::InteriorLights);
                eu.addEndUse(el * W2kW, EndUseFuelType::Electricity, EndUseCategoryType::ExteriorLights);
                eu.addEndUse(fn * W2kW, EndUseFuelType::Electricity, EndUseCategoryType::Fans);
                eu.addEndUse(pm * W2kW, EndUseFuelType::Electricity, EndUseCategoryType::Pumps);
                eu.addEndUse(pi * W2kW, EndUseFuelType::Electricity, EndUseCategoryType::InteriorEquipment);
                eu.addEndUse(pe * W2kW, EndUseFuelType::Electricity, EndUseCategoryType::ExteriorEquipment);
                eu.addEndUse(dw * W2kW, EndUseFuelType::Electricity, EndUseCategoryType::WaterSystems);
                eu.addEndUse(gas_ht, EndUseFuelType::Gas, EndUseCategoryType::Heating);
#endif
                };

            if (aggregateByMonth) {
                for (int m = 0; m < monthsInYear; ++m) {
                    EndUses eu;
                    double sums[9] = { 0 };
                    for (int i = monthEndHours[m]; i < monthEndHours[m + 1]; ++i) {
                        sums[0] += m_phi_H_nd[i]; sums[1] += m_phi_C_nd[i]; sums[2] += m_phi_int_L[i]; sums[3] += m_phi_ext_L[i];
                        sums[4] += m_phi_fan[i]; sums[5] += m_phi_pump[i]; sums[6] += m_phi_int_App[i]; sums[7] += m_phi_ext_App[i]; sums[8] += m_phi_dhw[i];
                    }
                    mapToEU(eu, sums[0], sums[1], sums[2], sums[3], sums[4], sums[5], sums[6], sums[7], sums[8]);
                    results.push_back(eu);
                }
            }
            else {
                for (int i = 0; i < hoursInYear; ++i) {
                    EndUses eu;
                    mapToEU(eu, m_phi_H_nd[i], m_phi_C_nd[i], m_phi_int_L[i], m_phi_ext_L[i], m_phi_fan[i], m_phi_pump[i], m_phi_int_App[i], m_phi_ext_App[i], m_phi_dhw[i]);
                    results.push_back(eu);
                }
            }
            return results;
        }

        void HourlyModel::initialize() {
            double floorArea = structure.floorArea();
            
            // OPTIMIZATION 1: Inverse Floor Area
            // Calculate 1.0 / floorArea once.
            invFloorArea = 1.0 / (floorArea + 1E-15);

            m_I_sol_max = structure.irradianceForMaxShadingUse();
            m_Cp_air_pressure = ventilation.dCp();
            m_theta_ve_preheat = ventilation.ventPreheatDegC();
            m_eta_ve_rec = ventilation.heatRecoveryEfficiency();
            m_phi_fan_spec = ventilation.fanPower();
            
            m_f_phi_int_L = lights.elecInternalGains();
            m_f_phi_sol_air = simSettings.phiSolFractionToAirNode();
            m_f_phi_int_air = simSettings.phiIntFractionToAirNode();

            auto lightingOccupancySensorDimmingFraction = building.lightingOccupancySensor();
            auto daylightSensorDimmingFraction = lights.dimmingFraction();

            if (lightingOccupancySensorDimmingFraction < 1.0 && daylightSensorDimmingFraction < 1.0) {
                f_L_max = lights.presenceAutoAd();
                I_lux_nat = lights.presenceAutoLux();
            }
            else if (lightingOccupancySensorDimmingFraction < 1.0) {
                f_L_max = lights.presenceSensorAd();
                I_lux_nat = lights.presenceSensorLux();
            }
            else if (daylightSensorDimmingFraction < 1.0) {
                f_L_max = lights.automaticAd();
                I_lux_nat = lights.automaticLux();
            }
            else {
                f_L_max = lights.manualSwitchAd();
                I_lux_nat = lights.manualSwitchLux();
            }

            f_A_nat = std::max(0.0001, lights.naturallyLightedArea()) * invFloorArea;
            m_A_nat_inv = (f_A_nat > 0) ? (53.0 / f_A_nat) : 0.0;
            
            // Optimization: Solar Geometry Ratio
            // Using pre-calculated inverse floor area
            win_floor_ratio = structure.windowArea().empty() ? 0.0 : structure.windowArea()[0] * invFloorArea; 

            for (int i = 0; i != numTotalSurfaces; ++i) {
                structureCalculations(structure.windowShadingDevice()[i], structure.wallArea()[i],
                    structure.windowArea()[i], structure.wallUniform()[i], structure.windowUniform()[i],
                    structure.wallSolarAbsorption()[i], structure.windowShadingCorrectionFactor()[i],
                    structure.windowNormalIncidenceSolarEnergyTransmittance()[i], i);

                A_nla_ms_norm[i] = A_nla_ms[i] * invFloorArea;
                f_light_ratio[i] = A_nla[i] * invFloorArea;
                f_light_shade_reduction[i] = A_nla_ms_norm[i] - f_light_ratio[i];
                A_sol_ms_norm[i] = A_sol_ms[i] * invFloorArea;
                f_sol_ratio[i] = A_sol[i] * invFloorArea;
                f_sol_shade_reduction[i] = A_sol_ms_norm[i] - f_sol_ratio[i];
            }

            f_sh_use = structure.shadingFactorAtMaxUse() / structure.irradianceForMaxShadingUse();
            
            for(int i=0; i<numTotalSurfaces; ++i) {
                precalc_nla_shading[i] = f_sh_use * f_light_shade_reduction[i];
                precalc_solar_shading[i] = f_sol_shade_reduction[i] * f_sh_use;
            }

            q_ve_4Pa = std::max(0.000001, (n50ToQ4 * (ventilation.n50() * (floorArea * structure.buildingHeight()))) * invFloorArea);

            h_ms = simSettings.hci() + simSettings.hri() * 1.2;
            h_is = 1.0 / (1.0 / simSettings.hci() - 1.0 / h_ms);
            H_tr_is = h_is * structure.totalAreaPerFloorArea();

            const auto& wallAreas = structure.wallArea();
            double A_wall_total = std::accumulate(wallAreas.begin(), wallAreas.end(), 0.0);

            C_m = (structure.interiorHeatCapacity() + (structure.wallHeatCapacity() * A_wall_total * invFloorArea)) / 1000.0;

            // OPTIMIZATION 2: Mass Area (A_m) Interpolation
            // Moved out of hourly loop because C_m is constant.
            if (C_m > veryHeavy) A_m = 3.5;
            else if (C_m > heavy) A_m = 3.0 + 0.5 * ((C_m - heavy) / 110.0);
            else if (C_m > medium) A_m = 2.5 + 0.5 * ((C_m - medium) / 95.0);
            else A_m = 2.5;

            double H_win_sum = 0.0, H_wall_sum_total = 0.0;
            for (int i = 0; i != numTotalSurfaces; ++i) { H_win_sum += H_win[i]; H_wall_sum_total += H_tot[i] - H_win[i]; }
            H_tr_w = H_win_sum * invFloorArea;

            p_rs = (structure.totalAreaPerFloorArea() - A_m - H_tr_w / h_ms) / structure.totalAreaPerFloorArea();
            p_rs_int = (1.0 - simSettings.phiIntFractionToAirNode()) * p_rs;
            p_rs_sol = (1.0 - simSettings.phiSolFractionToAirNode()) * p_rs;
            p_rm = A_m / structure.totalAreaPerFloorArea();
            p_rm_int = (1.0 - simSettings.phiIntFractionToAirNode()) * p_rm;
            p_rm_sol = (1.0 - simSettings.phiSolFractionToAirNode()) * p_rm;

            H_ms = h_ms * A_m;
            // OPTIMIZATION: H_em Calculation
            H_em = 1.0 / (1.0 / std::max(H_wall_sum_total * invFloorArea, 0.000001) - 1.0 / H_ms);
            
            H_z = std::max(0.1, ventilation.hzone());
            f_ve_mech_sup = std::max(0.00001, ventilation.fanControlFactor());

            WeeklyScheduleData weekly;
            std::vector<LoadedScheduleData> fileData;
            bool useFile = false;
            
            // Try to load from file if path is present and not "false"
            if (!m_hourlySchedulePath.empty()) {
                std::string lowerPath = m_hourlySchedulePath;
                std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);
                if (lowerPath != "false") {
                     useFile = loadSchedulesFromFile(m_hourlySchedulePath, fileData);
                }
            }

            if (!useFile) {
                buildWeeklySchedules(weekly);
            }

            m_hourlyData.resize(hoursInYear);
            
            const auto& data = epwData->data();
            const std::vector<double>& wind = data[WSPD];
            const std::vector<double>& temp = data[DBT];
            const std::vector<double>& egh = data[EGH];
            
            TimeFrame frame;
            for(int i=0; i<hoursInYear; ++i) {
                int h = frame.Hour[i];
                int d = frame.DayOfWeek[i];
                HourlyCache& c = m_hourlyData[i];

                if (useFile) {
                    const auto& row = fileData[i];
                    c.sched_q_ve_mech = (float)row.MechVent; // Assumed to be in same units as weekly (L/s?) or is it directly used? 
                    // Note: file values are used directly. If file contains raw flow, ensure it matches expectations.
                    // However, in existing code: c.sched_q_ve_mech = weekly.q_ve[h][d] = ventRate.
                    
                    c.sched_ext_equip = (float)row.ExtEquip; // Using as double/float directly
                    c.sched_phi_int_App = (float)row.IntApp;
                    c.sched_ext_light = (float)row.ExtLight;
                    c.sched_phi_int_L = (float)row.IntLight;
                    c.sched_theta_H_set = (float)row.HeatSet;
                    c.sched_theta_C_set = (float)row.CoolSet;
                } else {
                    c.sched_q_ve_mech   = (float)weekly.q_ve[h][d];
                    c.sched_ext_equip   = (float)weekly.ext_App[h][d];
                    c.sched_phi_int_App = (float)weekly.int_App[h][d];
                    c.sched_ext_light   = (float)weekly.ext_L[h][d];
                    c.sched_phi_int_L   = (float)weekly.int_L[h][d];
                    c.sched_theta_H_set = (float)weekly.theta_H[h][d];
                    c.sched_theta_C_set = (float)weekly.theta_C[h][d];
                }
                
                c.theta_e = (float)temp[i];
                c.I_sol_gh = (float)egh[i];

                c.q_ve_wind = (float)(windFactor * q_ve_4Pa * fastPow23(m_Cp_air_pressure * wind[i] * wind[i]));
                
                double q_ve = c.sched_q_ve_mech * kWh2MJ * invFloorArea;
                c.q_ve_mech_sup = (float)(q_ve * f_ve_mech_sup);
                c.q_ve_diff = (float)(-(c.q_ve_mech_sup - q_ve)); 
                c.theta_sup = (float)(std::max(m_theta_ve_preheat, (1.0 - m_eta_ve_rec) * temp[i] + m_eta_ve_rec * 20.0));
            }
        }
        
        bool HourlyModel::loadSchedulesFromFile(const std::string& path, std::vector<LoadedScheduleData>& data) {
            std::ifstream file(path);
            if (!file.is_open()) {
                std::cerr << "Error: Could not open schedule file: " << path << std::endl;
                return false;
            }

            std::string line;
            std::getline(file, line); // Skip header

            data.clear();
            data.reserve(8760);

            while (std::getline(file, line)) {
                std::stringstream ss(line);
                std::string cell;
                LoadedScheduleData row;
                
                try {
                    // Expected format: Hour,MechVent,IntApp,IntLight,ExtLight,ExtEquip,HeatSet,CoolSet
                    std::getline(ss, cell, ','); row.Hour = std::stoi(cell);
                    std::getline(ss, cell, ','); row.MechVent = std::stod(cell);
                    std::getline(ss, cell, ','); row.IntApp = std::stod(cell);
                    std::getline(ss, cell, ','); row.IntLight = std::stod(cell);
                    std::getline(ss, cell, ','); row.ExtLight = std::stoi(cell);
                    std::getline(ss, cell, ','); row.ExtEquip = std::stoi(cell);
                    std::getline(ss, cell, ','); row.HeatSet = std::stoi(cell);
                    std::getline(ss, cell, ','); row.CoolSet = std::stoi(cell);
                    data.push_back(row);
                } catch (...) {
                    std::cerr << "Error parsing line in schedule file: " << line << std::endl;
                    return false;
                }
            }
            
            if (data.size() < 8760) {
                 std::cerr << "Warning: Schedule file has fewer than 8760 rows (" << data.size() << ")" << std::endl;
                 // Could fill remainder or fail. For now, let's warn.
                 // To prevent crash in loop, resize with defaults or last value
                 if(data.empty()) return false;
                 data.resize(8760, data.back()); 
            }
            
            return true;
        }

        void HourlyModel::buildWeeklySchedules(WeeklyScheduleData& sched) {
            const int dayStart = static_cast<int>(pop.daysStart()), dayEnd = static_cast<int>(pop.daysEnd());
            const int hourStart = static_cast<int>(pop.hoursStart()), hourEnd = static_cast<int>(pop.hoursEnd());
            const double ventRate = ventilation.supplyRate(), extEquip = building.externalEquipment();
            const double intOcc = building.electricApplianceHeatGainOccupied(), intUnocc = building.electricApplianceHeatGainUnoccupied();
            const double intLtOcc = lights.powerDensityOccupied(), intLtUnocc = lights.powerDensityUnoccupied();
            const double htOcc = heating.temperatureSetPointOccupied(), htUnocc = heating.temperatureSetPointUnoccupied();
            const double clOcc = cooling.temperatureSetPointOccupied(), clUnocc = cooling.temperatureSetPointUnoccupied();

            for (int h = 0; h < hoursInDay; ++h) {
                bool hoccupied = (h >= hourStart && h <= hourEnd);
                for (int d = 0; d < daysInWeek; ++d) {
                    bool popoccupied = hoccupied && (d >= dayStart && d <= dayEnd);
                    sched.q_ve[h][d] = hoccupied ? ventRate : 0.0;
                    sched.ext_App[h][d] = extEquip;
                    sched.int_App[h][d] = popoccupied ? intOcc : intUnocc;
                    sched.ext_L[h][d] = 1.0;
                    sched.int_L[h][d] = popoccupied ? intLtOcc : intLtUnocc;
                    sched.theta_H[h][d] = popoccupied ? htOcc : htUnocc;
                    sched.theta_C[h][d] = popoccupied ? clOcc : clUnocc;
                }
            }
        }

        void HourlyModel::structureCalculations(double SHGC, double A_wall, double A_win,
            double U_wall, double U_win, double alpha_wall,
            double F_sh_with, double F_sh_without, int direction) {
            
            double WindowT = SHGC / SHGCClearGlass;
            A_nla_ms[direction] = A_win * WindowT;
            A_nla[direction] = A_win * WindowT;
            A_sol_ms[direction] = A_wall * (alpha_wall * U_wall * structure.R_se()) + A_win * F_sh_with;
            A_sol[direction] = A_wall * (alpha_wall * U_wall * structure.R_se()) + A_win * F_sh_without;
            H_tot[direction] = A_wall * U_wall + A_win * U_win;
            H_win[direction] = A_win * U_win;
        }

        std::vector<double> HourlyModel::sumHoursByMonth(const std::vector<double>& hourlyData) { return {}; }
    }
}
