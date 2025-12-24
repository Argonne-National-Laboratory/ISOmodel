/*
 * HourlyModel.cpp
 *
 * REFACTORING: ISO STANDARD ALIGNMENT
 * Implementation uses variable names consistent with ISO 13790 and ISO 15242.
 * OPTIMIZATION: Implemented Solar Radiation Caching.
 */

#include "Constants.hpp"

#include "HourlyModel.hpp"
#include "SolarRadiation.hpp" 
#include <algorithm>
#include <numeric>
#include <cmath>
#include <cfloat> 
#include <vector>

namespace openstudio {
    namespace isomodel {

        void printMatrix(const char* matName, double* mat, unsigned int dim1, unsigned int dim2) {}

        HourlyModel::HourlyModel() : A_floor_inv(0), rhoCpAir_277(0), m_I_sol_max(0), m_Cp_air_pressure(0), 
            m_theta_ve_preheat(0), m_eta_ve_rec(0), m_phi_fan_spec(0), m_A_nat_inv(0),
            m_f_phi_int_L(0), m_f_phi_sol_air(0), m_f_phi_int_air(0) {
            
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

            // Result Vectors (Renamed to ISO symbols)
            std::vector<double> phi_H_nd(hoursInYear);       // Heating need
            std::vector<double> phi_C_nd(hoursInYear);       // Cooling need
            std::vector<double> phi_int_L(hoursInYear);      // Interior Lighting
            std::vector<double> phi_ext_L(hoursInYear);      // Exterior Lighting
            std::vector<double> phi_fan(hoursInYear);        // Fan energy
            std::vector<double> phi_pump(hoursInYear);       // Pump energy
            std::vector<double> phi_int_App(hoursInYear);    // Equipment/Plug loads
            std::vector<double> phi_ext_App(hoursInYear);    // Exterior Equipment
            std::vector<double> phi_dhw(hoursInYear, 0.0);   // DHW

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

            for (int i = 0; i < hoursInYear; ++i) {
                const HourlyCache& cache = m_hourlyData[i];
                
                // ISO 13790: theta_e (external temp)
                double theta_e = cache.theta_e;

                // ISO 13790 10.4.2: Internal heat gains from appliances (\Phi_{int,A})
                phi_ext_App[i] = cache.sched_ext_equip * A_floor_inv;
                phi_int_App[i] = cache.sched_phi_int_App;

                // 1. Gains calculation 
                // Calculates \Phi_{int} (ISO 13790 10.2.2 eq. 35) and \Phi_{sol} (11.3.2 eq. 43)
                GainsResult gains = calculateGains(std::span<const double>(&eglobeFlat[i * 8], 8), cache, phi_int_App[i]);
                phi_int_L[i] = gains.phi_int_L;

                // 2. Airflow (Using Pre-calculated Physics from Initialize)
                // ISO 15242 calculations for wind and stack effect
                AirFlowResult flow = calculateAirFlows(theta_air, cache);

                // 3. Solve RC Network (Unconditional Linear)
                // Solves the 5R1C network described in ISO 13790 Annex C
                // Calculates \Phi_{HC,nd} (Heating/Cooling need)
                double phi_HC_nd = solveThermalBalance(theta_e, flow.theta_ent, gains.phi_ia, gains.phi_int, gains.phi_sol, flow.H_ve, flow.H_tr_1,
                    cache.sched_theta_H_set, cache.sched_theta_C_set,
                    theta_m_prev, theta_air);

                phi_H_nd[i] = std::max(0.0, phi_HC_nd);
                phi_C_nd[i] = std::max(0.0, -phi_HC_nd);

                // 4. Auxiliary Calculations
                double q_ve_mech = cache.sched_q_ve_mech * kWh2MJ * A_floor_inv; // Convert to energy units

                // Calculate Air Volume for fans (V_{air}) based on heating/cooling delivery needs
                // Using rhoCpAir_277 (Wh/m3K)
                double V_air = std::max({ q_ve_mech,
                    phi_H_nd[i] / (((heat_occ_sp + heat_dT_supp) - theta_air) * rhoCpAir_277 + DBL_MIN),
                    phi_C_nd[i] / ((theta_air - (cool_occ_sp - cool_dT_supp)) * rhoCpAir_277 + DBL_MIN) });

                // Fan energy: V_{air} * specific fan power
                phi_fan[i] = V_air * fan_power_factor;
                
                // Pump energy
                phi_pump[i] = (phi_C_nd[i] > 0) ? (cool_E_pumps * cool_pumpRed) :
                    ((phi_H_nd[i] > 0) ? (heat_E_pumps * heat_pumpRed) : 0.0);
                
                // Exterior lighting (only when sun is down)
                phi_ext_L[i] = (cache.I_sol_gh > 0) ? 0.0 : (lights_extEnergy * cache.sched_ext_light * A_floor_inv);
            }

            return processResults(phi_H_nd, phi_C_nd, phi_int_L, phi_ext_L, phi_fan, phi_pump, phi_int_App, phi_ext_App, phi_dhw, aggregateByMonth);
        }

        void HourlyModel::initialize() {
            double floorArea = structure.floorArea();
            A_floor_inv = (floorArea > 0) ? 1.0 / floorArea : 0.0;
            rhoCpAir_277 = phys.rhoCpAir() * 277.777778; 
            
            m_I_sol_max = structure.irradianceForMaxShadingUse();
            m_Cp_air_pressure = ventilation.dCp();
            m_theta_ve_preheat = ventilation.ventPreheatDegC();
            m_eta_ve_rec = ventilation.heatRecoveryEfficiency();
            m_phi_fan_spec = ventilation.fanPower();
            
            m_f_phi_int_L = lights.elecInternalGains();
            m_f_phi_sol_air = simSettings.phiSolFractionToAirNode();
            m_f_phi_int_air = simSettings.phiIntFractionToAirNode();

            // Lighting Control Initialization
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

            f_A_nat = std::max(0.0001, lights.naturallyLightedArea()) * A_floor_inv;
            m_A_nat_inv = (f_A_nat > 0) ? (53.0 / f_A_nat) : 0.0;

            // Structure solar calculations
            for (int i = 0; i != numTotalSurfaces; ++i) {
                structureCalculations(structure.windowShadingDevice()[i], structure.wallArea()[i],
                    structure.windowArea()[i], structure.wallUniform()[i], structure.windowUniform()[i],
                    structure.wallSolarAbsorption()[i], structure.windowShadingCorrectionFactor()[i],
                    structure.windowNormalIncidenceSolarEnergyTransmittance()[i], i);

                A_nla_ms_norm[i] = A_nla_ms[i] * A_floor_inv;
                f_light_ratio[i] = A_nla[i] * A_floor_inv;
                f_light_shade_reduction[i] = A_nla_ms_norm[i] - f_light_ratio[i];
                A_sol_ms_norm[i] = A_sol_ms[i] * A_floor_inv;
                f_sol_ratio[i] = A_sol[i] * A_floor_inv;
                f_sol_shade_reduction[i] = A_sol_ms_norm[i] - f_sol_ratio[i];
            }

            f_sh_use = structure.shadingFactorAtMaxUse() / structure.irradianceForMaxShadingUse();
            
            for(int i=0; i<numTotalSurfaces; ++i) {
                precalc_nla_shading[i] = f_sh_use * f_light_shade_reduction[i];
                precalc_solar_shading[i] = f_sol_shade_reduction[i] * f_sh_use;
            }

            // ISO 15242 Annex D Table D.1: Total air leakage at 4Pa
            q_ve_4Pa = std::max(0.000001, (0.19 * (ventilation.n50() * (floorArea * structure.buildingHeight()))) * A_floor_inv);

            // ISO 13790 12.2.2: h_ms fixed at 9.1 W/m^2K unless overridden
            h_ms = simSettings.hci() + simSettings.hri() * 1.2;
            // ISO 13790 7.2.2.2: h_is
            h_is = 1.0 / (1.0 / simSettings.hci() - 1.0 / h_ms);
            // ISO 13790 7.2.2.2 eq. 9: H_{tr,is}
            H_tr_is = h_is * structure.totalAreaPerFloorArea();

            const auto& wallAreas = structure.wallArea();
            double A_wall_total = std::accumulate(wallAreas.begin(), wallAreas.end(), 0.0);

            // Calculate internal heat capacity (Cm) - ISO 13790 12.3.1.2
            C_m = (structure.interiorHeatCapacity() + (structure.wallHeatCapacity() * A_wall_total * A_floor_inv)) / 1000.0;

            // Calculate Am (Effective Mass Area) - ISO 13790 12.3.1.2 Table 12
            if (C_m > 370.0) A_m = 3.5;
            else if (C_m > 260.0) A_m = 3.0 + 0.5 * ((C_m - 260) / 110.0);
            else if (C_m > 165.0) A_m = 2.5 + 0.5 * ((C_m - 165) / 95.0);
            else A_m = 2.5;

            double H_win_sum = 0.0, H_wall_sum = 0.0;
            for (int i = 0; i != numTotalSurfaces; ++i) { H_win_sum += H_win[i]; H_wall_sum += H_tot[i] - H_win[i]; }
            H_tr_w = H_win_sum * A_floor_inv;

            // Constant portions of \Phi_{st} and \Phi_{m} (ISO 13790 Annex C.2)
            p_rs = (structure.totalAreaPerFloorArea() - A_m - H_tr_w / h_ms) / structure.totalAreaPerFloorArea();
            p_rs_int = (1.0 - simSettings.phiIntFractionToAirNode()) * p_rs;
            p_rs_sol = (1.0 - simSettings.phiSolFractionToAirNode()) * p_rs;
            p_rm = A_m / structure.totalAreaPerFloorArea();
            p_rm_int = (1.0 - simSettings.phiIntFractionToAirNode()) * p_rm;
            p_rm_sol = (1.0 - simSettings.phiSolFractionToAirNode()) * p_rm;

            // ISO 13790 12.2.2 eq. 64: H_{ms}
            H_ms = h_ms * A_m;
            // ISO 13790 12.2.2 eq. 63: H_{em}
            H_em = 1.0 / (1.0 / std::max(H_wall_sum * A_floor_inv, 0.000001) - 1.0 / H_ms);
            
            H_z = std::max(0.1, ventilation.hzone());
            f_ve_mech_sup = std::max(0.00001, ventilation.fanControlFactor());

            // ----------------------------------------------------
            // OPTIMIZATION: FLATTEN SCHEDULES & PRE-CALC PHYSICS
            // ----------------------------------------------------
            WeeklyScheduleData weekly;
            buildWeeklySchedules(weekly); 

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

                // Convert schedules to float for cache compression
                c.sched_q_ve_mech   = (float)weekly.q_ve[h][d];
                c.sched_ext_equip   = (float)weekly.ext_App[h][d];
                c.sched_phi_int_App = (float)weekly.int_App[h][d];
                c.sched_ext_light   = (float)weekly.ext_L[h][d];
                c.sched_phi_int_L   = (float)weekly.int_L[h][d];
                c.sched_theta_H_set = (float)weekly.theta_H[h][d];
                c.sched_theta_C_set = (float)weekly.theta_C[h][d];
                
                // Copy Weather to Cache
                c.theta_e = (float)temp[i];
                c.I_sol_gh = (float)egh[i];

                // Pre-calc Physics (float is sufficient)
                // ISO 15242 6.7.1 Step 1: q_{v,wind}
                c.q_ve_wind = (float)(0.0769 * q_ve_4Pa * fastPow23(m_Cp_air_pressure * wind[i] * wind[i]));
                
                // Ventilation supply/exhaust
                double q_ve = c.sched_q_ve_mech * kWh2MJ * A_floor_inv;
                c.q_ve_mech_sup = (float)(q_ve * f_ve_mech_sup);
                c.q_ve_diff = (float)(-(c.q_ve_mech_sup - q_ve)); // ISO 15242 q_{v-diff}
                c.theta_sup = (float)(std::max(m_theta_ve_preheat, (1.0 - m_eta_ve_rec) * temp[i] + m_eta_ve_rec * 20.0));
            }
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

        GainsResult HourlyModel::calculateGains(std::span<const double> curSolar,
            const HourlyCache& cache, double phi_int_App) noexcept {
            
            GainsResult res;
            double lightingLevelSum = 0.0;
            res.phi_sol = 0.0;
            const double I_max = m_I_sol_max;

            // ISO 13790 11.3.2 eq. 43: \Phi_{sol,k} (Solar gains)
            // Includes movable shading logic (switching between g_gl and g_gl+sh)
            for (int k = 0; k < 8; ++k) {
                double I_k = curSolar[k];
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

        AirFlowResult HourlyModel::calculateAirFlows(double theta_air, const HourlyCache& cache) noexcept {
            
            AirFlowResult res;
            double theta_e = cache.theta_e;
            double absDT = std::max(std::fabs(theta_e - theta_air), 1e-5);
            
            // ISO 15242 6.7.1 Step 1: q_{stack} (Stack effect)
            double q_ve_stack = 0.0146 * q_ve_4Pa * fastPow23(0.5 * H_z * absDT);
            
            // Promote float physics to double
            double q_ve_wind = cache.q_ve_wind;
            double q_ve_diff = cache.q_ve_diff;

            // ISO 15242 6.7.1 Step 2: q_{exfiltration}
            double q_ve_sw = q_ve_stack + q_ve_wind + smallEpsilon;
            double q_ve_exf = std::max(0.0, std::max(q_ve_stack, q_ve_wind) - std::fabs(q_ve_diff) * (0.5 * q_ve_stack + 0.667 * q_ve_wind / q_ve_sw));

            // ISO 15242 6.7.2: q_{ent} (Total entering air)
            double q_ve_ent = (q_ve_diff > 0 ? q_ve_diff : 0.0) + q_ve_exf + cache.q_ve_mech_sup;
            
            // ISO 13790 9.3: \theta_{sup} (Supply temperature)
            res.theta_ent = (theta_e * ((q_ve_diff > 0 ? q_ve_diff : 0.0) + q_ve_exf) + cache.theta_sup * cache.q_ve_mech_sup) / (q_ve_ent + smallEpsilon);
            
            // ISO 13790 9.3.1 eq. 21: H_{ve} (Ventilation heat transfer coefficient)
            res.H_ve = rhoCpAirWh * q_ve_ent;
            
            // ISO 13790 C.3 eq. C.6: H_{tr,1}
            res.H_tr_1 = 1.0 / (1.0 / (res.H_ve + smallEpsilon) + 1.0 / H_tr_is);
            return res;
        }

        double HourlyModel::solveThermalBalance(double theta_e, double theta_ent, double phi_ia, double phi_int,
            double phi_sol, double H_ve, double H_tr_1, double theta_H_set,
            double theta_C_set, double& theta_m_prev, double& theta_air) noexcept {
            
            // ISO 13790 C.3 eq. C.7: H_{tr,2}
            double H_tr_2 = H_tr_1 + H_tr_w;
            // ISO 13790 C.3 eq. C.9: H_{tr,3}
            double H_tr_3 = 1.0 / (1.0 / H_tr_2 + 1.0 / H_ms);
            
            double H_tr_3_H_em = 0.5 * (H_tr_3 + H_em);
            double H_tr_3_H_tr_2 = H_tr_3 / H_tr_2;
            
            double H_ve_safe = H_ve + smallEpsilon;
            double H_ve_inv = 1.0 / H_ve_safe; 
            double d_phim_dp = (H_tr_3_H_tr_2 * H_tr_1) * H_ve_inv; // d(phim)/dP
            
            double Cm_units = C_m / kWh2MJ;
            double Cm_plus_H = Cm_units + H_tr_3_H_em;
            double d_thetam_dp = d_phim_dp / Cm_plus_H;
            
            double H_denom = H_ms + H_tr_w + H_tr_1;
            double d_thetas_dp = (H_ms * 0.5 * d_thetam_dp + H_tr_1 * H_ve_inv) / H_denom;
            
            // Slope d(\theta_{air})/d(\phi)
            double d_theta_air_dp = (H_tr_is * d_thetas_dp + 1.0) / (H_tr_is + H_ve); 
            
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
            double theta_s   = (H_ms * 0.5 * (theta_m_prev + theta_m) + mid + H_tr_1 * phi_ia * H_ve_inv) / H_denom;
            
            // ISO 13790 C.3 eq. C.11: \theta_{air} (Indoor air temperature)
            double theta_air_0 = (H_tr_is * theta_s + H_ve * theta_ent + phi_ia) / (H_tr_is + H_ve);
            
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

        std::vector<EndUses> HourlyModel::processResults(const std::vector<double>& phi_H_nd, const std::vector<double>& phi_C_nd,
            const std::vector<double>& phi_int_L, const std::vector<double>& phi_ext_L,
            const std::vector<double>& phi_fan, const std::vector<double>& phi_pump,
            const std::vector<double>& phi_int_App, const std::vector<double>& phi_ext_App,
            const std::vector<double>& phi_dhw, bool aggregateByMonth) {
            
            double phi_H_tot = std::accumulate(phi_H_nd.begin(), phi_H_nd.end(), 0.0);
            double phi_C_tot = std::accumulate(phi_C_nd.begin(), phi_C_nd.end(), 0.0);
            double f_H = std::max(phi_H_tot / (phi_C_tot + phi_H_tot + DBL_MIN), 0.1);
            
            double s_ht = (1.0 / (1.0 / (1.0 + heating.hvacLossFactor() + heating.hotcoldWasteFactor() / f_H))) / heating.efficiency();
            double s_cl = (1.0 / (1.0 / (1.0 + cooling.hvacLossFactor() + heating.hotcoldWasteFactor() / (1.0 - f_H)))) / cooling.cop();

            bool electricHeat = (heating.energyType() == 1);
            std::vector<EndUses> results;
            if (!aggregateByMonth) results.reserve(hoursInYear);

            auto mapToEU = [&](EndUses& eu, double h, double c, double il, double el, double fn, double pm, double pi, double pe, double dw) {
                double elec_ht = electricHeat ? (h * s_ht * W2kW) : 0.0;
                double gas_ht = electricHeat ? 0.0 : (h * s_ht * W2kW);
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
                const int monthEnds[] = { 0, 744, 1416, 2160, 2880, 3624, 4344, 5088, 5832, 6552, 7296, 8016, 8760 };
                for (int m = 0; m < monthsInYear; ++m) {
                    EndUses eu;
                    double sums[9] = { 0 };
                    for (int i = monthEnds[m]; i < monthEnds[m + 1]; ++i) {
                        sums[0] += phi_H_nd[i]; sums[1] += phi_C_nd[i]; sums[2] += phi_int_L[i]; sums[3] += phi_ext_L[i];
                        sums[4] += phi_fan[i]; sums[5] += phi_pump[i]; sums[6] += phi_int_App[i]; sums[7] += phi_ext_App[i]; sums[8] += phi_dhw[i];
                    }
                    mapToEU(eu, sums[0], sums[1], sums[2], sums[3], sums[4], sums[5], sums[6], sums[7], sums[8]);
                    results.push_back(eu);
                }
            }
            else {
                for (int i = 0; i < hoursInYear; ++i) {
                    EndUses eu;
                    mapToEU(eu, phi_H_nd[i], phi_C_nd[i], phi_int_L[i], phi_ext_L[i], phi_fan[i], phi_pump[i], phi_int_App[i], phi_ext_App[i], phi_dhw[i]);
                    results.push_back(eu);
                }
            }
            return results;
        }

        void HourlyModel::structureCalculations(double SHGC, double A_wall, double A_win,
            double U_wall, double U_win, double alpha_wall,
            double F_sh_with, double F_sh_without, int direction) {
            
            double WindowT = SHGC / 0.87;
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