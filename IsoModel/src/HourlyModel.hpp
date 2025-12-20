/*
 * HourlyModel.hpp
 *
 * OPTIMIZATION SUMMARY (Round 3 - Fixed):
 * 1. Data Locality (Array of Structures): Replaced multiple separate vectors
 * with a single `std::vector<HourlyCache>`. This ensures all schedule and 
 * pre-calculated physics data for a specific hour are adjacent in memory.
 * 2. Unconditional Linear Solver: Reverted `solveThermalBalance` to the unconditional 
 * calculation to improve CPU pipelining (branchless).
 * 3. Solar Pointer: Retained the fast pointer-based solar access.
 * 4. Fix: Corrected function signature for calculateGains.
 */

#ifndef ISOMODEL_HOURLYMODEL_HPP
#define ISOMODEL_HOURLYMODEL_HPP

#include "ISOModelAPI.hpp"
#include "ISOResults.hpp"
#include "Simulation.hpp"
#include "TimeFrame.hpp"
#include "MonthlyModel.hpp"
#include <vector>
#include <array>
#include <cmath>

#ifdef ISOMODEL_STANDALONE
#include "EndUses.hpp"
#include "Vector.hpp"
#else
#include "../utilities/data/EndUses.hpp"
#include "../utilities/data/Vector.hpp"
#endif

namespace openstudio {
    namespace isomodel {

        inline double fastPow23(double x) {
            return std::cbrt(x * x);
        }

        // Data structure for strict memory locality
        struct HourlyCache {
            // Schedules
            double sched_ventilation;
            double sched_ext_equip;
            double sched_int_equip;
            double sched_ext_light;
            double sched_int_light;
            double sched_heat_sp;
            double sched_cool_sp;

            // Pre-calculated Physics
            double phys_qWind;
            double phys_qSup;
            double phys_exhSup;
            double phys_tSupp;
        };

        class ISOMODEL_API HourlyModel : public Simulation
        {
        public:
            HourlyModel();
            virtual ~HourlyModel();

            std::vector<EndUses> simulate(bool aggregateByMonth = false);

        private:
            void populateSchedules();
            void initialize();

            // Helpers
            void calculateAirFlows(double temperature, double tiHeatCool, 
                const HourlyCache& cache,
                double& out_tEnt, double& out_hei, double& out_h1);

            // FIX: Added missing schedIntEquip argument to match .cpp definition
            void calculateGains(const double* curSolar, double egh_i,
                const HourlyCache& cache,
                double schedIntEquip,
                double& out_phi_int, double& out_phii,
                double& out_lighting_tot, double& out_qSolarGain);

            double solveThermalBalance(double temperature, double tEnt, double phii, double phi_int,
                double qSolarGain, double hei, double h1, double schedHeatSP,
                double schedCoolSP, double& TMT1, double& tiHeatCool);

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

            // Arrays
            double nlams[9], nla[9], sams[9], sa[9], htot[9], hWindow[9];
            double nlaWMovableShading[9], naturalLightRatio[9], naturalLightShadeRatioReduction[9];
            double saWMovableShading[9], solarRatio[9], solarShadeRatioReduction[9];
            double precalc_nla_shading[9];
            double precalc_solar_shading[9];

            // 2D Schedule Helpers (Intermediate usage only)
            double fixedVentilationSchedule[24][7], fixedExteriorEquipmentSchedule[24][7];
            double fixedInteriorEquipmentSchedule[24][7], fixedExteriorLightingSchedule[24][7];
            double fixedInteriorLightingSchedule[24][7], fixedActualHeatingSetpoint[24][7], fixedActualCoolingSetpoint[24][7];

            // OPTIMIZATION: Single vector of structs for cache locality
            std::vector<HourlyCache> m_hourlyData;

            std::vector<double> sumHoursByMonth(const std::vector<double>& hourlyData);

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