/*
 * HourlyModel.hpp
 *
 * REFACTORING: ISO STANDARD ALIGNMENT & PERFORMANCE OPTIMIZATION
 * - Renamed A_floor_inv -> invFloorArea
 * - Added static optimization members (win_floor_ratio, invFloorArea)
 * - Added persistent result vectors to reduce heap allocation overhead
 * - Marked helper functions as inline for loop performance
 */

#ifndef ISOMODEL_HOURLYMODEL_HPP
#define ISOMODEL_HOURLYMODEL_HPP

#include "ISOModelAPI.hpp"
#include "ISOResults.hpp"
#include "Simulation.hpp"
#include "TimeFrame.hpp"
#include "MonthlyModel.hpp"
#include "MathHelpers.hpp"
#include <vector>
#include <array>
#include <cmath>
#include <span> 
#include <memory> 
#include <string>

#ifdef ISOMODEL_STANDALONE
#include "EndUses.hpp"
#else
#include "../utilities/data/EndUses.hpp"
#endif

namespace openstudio {
    namespace isomodel {

        class EpwData;

        // Compressed Data Structure (Array of Structures)
        struct HourlyCache {
            // Schedules (0.0 - 1.0)
            float sched_q_ve_mech;    // Mechanical ventilation schedule
            float sched_phi_int_App;  // Appliances gain schedule
            float sched_phi_int_L;    // Lighting gain schedule
            float sched_ext_light;    // Exterior lighting control
            float sched_ext_equip;    // Exterior equipment control
            float sched_theta_H_set;  // Heating setpoint
            float sched_theta_C_set;  // Cooling setpoint

            // Environmental
            float theta_e;  // External air temperature
            float I_sol_gh; // Global Horizontal Irradiance

            // Pre-calculated Physics (ISO 15242)
            float q_ve_wind;      // Airflow due to wind
            float q_ve_mech_sup;  // Mechanical supply airflow
            float q_ve_diff;      // Difference (exhaust - supply)
            float theta_sup;      // Supply air temperature
        };

        struct GainsResult {
            double phi_int;      // Total internal gains
            double phi_ia;       // Internal gains to air node
            double phi_int_L;    // Lighting gains
            double phi_sol;      // Solar gains
        };

        struct AirFlowResult {
            double theta_ent;    // Entering air temperature
            double H_ve;         // Ventilation heat transfer coefficient
            double H_tr_1;       // Coupling conductance 1
        };

        // Struct to hold raw CSV schedule data
        struct LoadedScheduleData {
            int Hour;
            double MechVent;
            double IntApp;
            double IntLight;
            int ExtLight;
            int ExtEquip;
            int HeatSet;
            int CoolSet;
        };

        class ISOMODEL_API HourlyModel : public Simulation
        {
        public:
            HourlyModel();
            virtual ~HourlyModel();

            // Original Interface preserved
            std::vector<EndUses> simulate(bool aggregateByMonth = false);

            // NEW: Accessor for the internal schedule cache
            const std::vector<HourlyCache>& getCachedSchedules() const { return m_hourlyData; }
            
            // Set the path for the hourly schedule file
            void setHourlySchedulePath(const std::string& path) { m_hourlySchedulePath = path; }

        private:
            void initialize();

            // Solar Caching Members
            std::shared_ptr<EpwData> m_lastEpwData;
            std::vector<double> m_cachedSolarRadiation; 

            // OPTIMIZATION: Persistent Result Vectors (Avoid Re-allocation)
            std::vector<double> m_phi_H_nd;
            std::vector<double> m_phi_C_nd;
            std::vector<double> m_phi_int_L;
            std::vector<double> m_phi_ext_L;
            std::vector<double> m_phi_fan;
            std::vector<double> m_phi_pump;
            std::vector<double> m_phi_int_App;
            std::vector<double> m_phi_ext_App;
            std::vector<double> m_phi_dhw;

            // Refactored Helpers - Inlined for performance
            inline AirFlowResult calculateAirFlows(double theta_air, const HourlyCache& cache) noexcept;

            inline GainsResult calculateGains(std::span<const double> curSolar,
                const HourlyCache& cache,
                double phi_int_App) noexcept;

            inline double solveThermalBalance(double theta_e, double theta_ent, double phi_ia, double phi_int,
                double phi_sol, double H_ve, double H_tr_1, double theta_H_set,
                double theta_C_set, double& theta_m_prev, double& theta_air) noexcept;

            std::vector<EndUses> processResults(bool aggregateByMonth);

            void structureCalculations(double SHGC, double A_wall, double A_win,
                double U_wall, double U_win,
                double alpha_wall, double F_sh_with,
                double F_sh_without, int direction);

            // Constants
            double invFloorArea, rhoCpAir_277, f_ve_mech_sup, q_ve_4Pa, H_z;
            double A_m, C_m, f_sh_use, f_A_nat, f_L_max;
            double I_lux_nat, H_zone, h_ms, h_is, H_tr_is, H_tr_w;
            double p_rs, p_rs_int, p_rs_sol, p_rm, p_rm_int, p_rm_sol, H_ms, H_op, H_em;
            
            // NEW: Pre-calculated Optimization Member
            double win_floor_ratio; // Optimization: Ratio for solar geom

            // Cached Config
            double m_I_sol_max; 
            double m_Cp_air_pressure;
            double m_theta_ve_preheat;
            double m_eta_ve_rec;
            double m_phi_fan_spec;
            double m_A_nat_inv; 
            double m_f_phi_int_L;
            double m_f_phi_sol_air;
            double m_f_phi_int_air;
            
            std::string m_hourlySchedulePath;

            // Arrays (std::array)
            std::array<double, 9> A_nla_ms;
            std::array<double, 9> A_nla;
            std::array<double, 9> A_sol_ms;
            std::array<double, 9> A_sol;
            std::array<double, 9> H_tot;
            std::array<double, 9> H_win;
            std::array<double, 9> A_nla_ms_norm;
            std::array<double, 9> f_light_ratio;
            std::array<double, 9> f_light_shade_reduction;
            std::array<double, 9> A_sol_ms_norm;
            std::array<double, 9> f_sol_ratio;
            std::array<double, 9> f_sol_shade_reduction;
            std::array<double, 9> precalc_nla_shading;
            std::array<double, 9> precalc_solar_shading;

            // Cache Locality Vector
            std::vector<HourlyCache> m_hourlyData;

            std::vector<double> sumHoursByMonth(const std::vector<double>& hourlyData);

            // Helpers
            struct WeeklyScheduleData {
                double q_ve[24][7];
                double ext_App[24][7];
                double int_App[24][7];
                double ext_L[24][7];
                double int_L[24][7];
                double theta_H[24][7];
                double theta_C[24][7];
            };
            void buildWeeklySchedules(WeeklyScheduleData& sched);
            bool loadSchedulesFromFile(const std::string& path, std::vector<LoadedScheduleData>& data);


            // Virtuals (kept for interface compliance)
            virtual double ventilationSchedule(int, int, int) { return 0; }
            virtual double exteriorEquipmentSchedule(int, int, int) { return 0; }
            virtual double interiorEquipmentSchedule(int, int, int) { return 0; }
            virtual double exteriorLightingSchedule(int, int, int) { return 0; }
            virtual double interiorLightingSchedule(int, int, int) { return 0; }
            virtual double heatingSetpointSchedule(int, int, int) { return 0; }
            virtual double coolingSetpointSchedule(int, int, int) { return 0; }
        };
    }
}
#endif