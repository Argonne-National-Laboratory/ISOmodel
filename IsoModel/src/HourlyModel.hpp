/*
 * HourlyModel.hpp
 *
 * OPTIMIZATION & REFACTORING (Round 4):
 * 1. Compressed Cache (Float): Changed schedules, weather, and physics in `HourlyCache` 
 * to `float`. This reduces the struct size to ~52 bytes, allowing one hour of 
 * data to fit entirely in a single 64-byte cache line for maximum bandwidth efficiency.
 * 2. Weather Locality: Added `env_temp` and `env_egh` to `HourlyCache`. This 
 * consolidates all inputs for an hour into one contiguous block.
 * 3. Member Cleanup: Removed persistent 2D schedule arrays. These are now 
 * transient stack variables during initialization.
 * 4. Modern C++: Continued use of std::span, std::array, and return structs.
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
#include <span> // Requires C++20

#ifdef ISOMODEL_STANDALONE
#include "EndUses.hpp"
#include "Vector.hpp"
#else
#include "../utilities/data/EndUses.hpp"
#include "../utilities/data/Vector.hpp"
#endif

namespace openstudio {
    namespace isomodel {


        // Compressed Data Structure (Array of Structures)
        // Uses 'float' to fit ~52 bytes, ensuring 1 hour fits in 1 CPU cache line (64 bytes).
        struct HourlyCache {
            // Schedules (0.0 - 1.0)
            float sched_ventilation;
            float sched_ext_equip;
            float sched_int_equip;
            float sched_ext_light;
            float sched_int_light;
            float sched_heat_sp;
            float sched_cool_sp;

            // Environmental
            float env_temp;
            float env_egh;

            // Pre-calculated Physics
            float phys_qWind;
            float phys_qSup;
            float phys_exhSup;
            float phys_tSupp;
        };

        struct GainsResult {
            double phi_int;
            double phii;
            double lighting_tot;
            double qSolarGain;
        };

        struct AirFlowResult {
            double tEnt;
            double hei;
            double h1;
        };

        class ISOMODEL_API HourlyModel : public Simulation
        {
        public:
            HourlyModel();
            virtual ~HourlyModel();

            std::vector<EndUses> simulate(bool aggregateByMonth = false);

        private:
            void initialize();

            // Refactored Helpers
            AirFlowResult calculateAirFlows(double tiHeatCool, const HourlyCache& cache) noexcept;

            GainsResult calculateGains(std::span<const double> curSolar,
                const HourlyCache& cache,
                double schedIntEquip) noexcept;

            double solveThermalBalance(double temperature, double tEnt, double phii, double phi_int,
                double qSolarGain, double hei, double h1, double schedHeatSP,
                double schedCoolSP, double& TMT1, double& tiHeatCool) noexcept;

            std::vector<EndUses> processResults(const std::vector<double>& r_Qneed_ht, const std::vector<double>& r_Qneed_cl,
                const std::vector<double>& r_Q_illum_tot, const std::vector<double>& r_Q_illum_ext_tot,
                const std::vector<double>& r_Qfan_tot, const std::vector<double>& r_Qpump_tot,
                const std::vector<double>& r_phi_plug, const std::vector<double>& r_ext_equip,
                const std::vector<double>& r_Q_dhw, bool aggregateByMonth);

            void structureCalculations(double SHGC, double wallAreaM2, double windowAreaM2,
                double wallUValue, double windowUValue,
                double wallSolarAbsorption, double solarFactorWith,
                double solarFactorWithout, int direction);

            // Constants
            double invFloorArea, rhoCpAir_277, windImpactSupplyRatio, q4Pa, windImpactHz;
            double Am, Cm, shadingUsePerWPerM2, areaNaturallyLightedRatio, maxRatioElectricLighting;
            double elightNatural, hzone, h_ms, h_is, H_tris, hwindowWperkm2;
            double prs, prsInterior, prsSolar, prm, prmInterior, prmSolar, H_ms, hOpaqueWperkm2, hem;

            // Cached Config
            double m_maxIrrad;
            double m_vent_dCp;
            double m_vent_preheat;
            double m_vent_HRE;
            double m_vent_fanPower;
            double m_invAreaNat;
            double m_frac_elec_internal_gains;
            double m_frac_phi_sol_air;
            double m_frac_phi_int_air;

            // Arrays (std::array)
            std::array<double, 9> nlams;
            std::array<double, 9> nla;
            std::array<double, 9> sams;
            std::array<double, 9> sa;
            std::array<double, 9> htot;
            std::array<double, 9> hWindow;
            std::array<double, 9> nlaWMovableShading;
            std::array<double, 9> naturalLightRatio;
            std::array<double, 9> naturalLightShadeRatioReduction;
            std::array<double, 9> saWMovableShading;
            std::array<double, 9> solarRatio;
            std::array<double, 9> solarShadeRatioReduction;
            std::array<double, 9> precalc_nla_shading;
            std::array<double, 9> precalc_solar_shading;

            // Cache Locality Vector
            std::vector<HourlyCache> m_hourlyData;

            std::vector<double> sumHoursByMonth(const std::vector<double>& hourlyData);

            // Helpers to replace removed persistent 2D arrays
            struct WeeklyScheduleData {
                double vent[24][7];
                double extEquip[24][7];
                double intEquip[24][7];
                double extLight[24][7];
                double intLight[24][7];
                double heatSP[24][7];
                double coolSP[24][7];
            };
            void buildWeeklySchedules(WeeklyScheduleData& sched);

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