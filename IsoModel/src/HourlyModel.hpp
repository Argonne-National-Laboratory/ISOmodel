/*
 * HourlyModel.hpp
 *
 * Refactored version: Modularized simulation logic while preserving public API.
 * Includes all precomputed members and support methods for the ISO 13790 hourly method.
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

        //// Helper for x^0.667 ~ x^(2/3) using cbrt(x^2). Faster than std::pow.
        // The standard says to take x^0.667 but that's actually an approximation of x^2/3 that comes from theory
        // jus use x^2/3 as it's probably more accurate anyhow
        inline double fastPow23(double x) {
            return std::cbrt(x * x);
        }

        // here is a correction if we decide we really need it
        ////**Logic: x ^ 0.667 = x ^ (2 / 3) * x ^ 0.000333...
        ////* Using Taylor : x ^ eps ~ 1 + eps * ln(x)
        ////* 
        //inline double fastPow23(double x) {
        //if (x <= 0) return 0;
        //    // Core calculation: x^(2/3)
        //    double x23 = std::cbrt(x * x);

        //        // Correction factor to move from 0.666... to 0.667
        //        // Difference is ~1/3000. 
        //        // x^0.667 ~ x^(2/3) * (1 + ln(x) * 0.0003333)
        //        return x23 * (1.0 + std::log(x) * 0.0003333333333333);
        //}

        class ISOMODEL_API HourlyModel : public Simulation
        {
        public:
            HourlyModel();
            virtual ~HourlyModel();

            // Main entry point for the simulation - Public API preserved
            std::vector<EndUses> simulate(bool aggregateByMonth = false);

        private:
            // --- Simulation Lifecycle ---
            void populateSchedules();
            void initialize();

            // --- Refactored Modular Components ---

            // Calculates internal gains and solar gains for a specific hour
            void calculateGains(int hourIdx, const std::vector<double>& curSolar, double egh_i,
                double schedIntLight, double schedIntEquip,
                double& out_phi_int, double& out_phii,
                double& out_lighting_tot, double& out_qSolarGain);

            // Solves the thermal RC network for a single timestep
            double solveThermalBalance(double temperature, double tEnt, double phii, double phi_int,
                double qSolarGain, double hei, double h1, double schedHeatSP,
                double schedCoolSP, double& TMT1, double& tiHeatCool);

            // Calculates ventilation and infiltration mass flows
            void calculateAirFlows(double windMps, double temperature, double tiHeatCool, double ventExh,
                double& out_tEnt, double& out_hei, double& out_h1);

            // Processes the raw hourly heating/cooling needs into final EndUses
            std::vector<EndUses> processResults(const std::vector<double>& r_Qneed_ht, const std::vector<double>& r_Qneed_cl,
                const std::vector<double>& r_Q_illum_tot, const std::vector<double>& r_Q_illum_ext_tot,
                const std::vector<double>& r_Qfan_tot, const std::vector<double>& r_Qpump_tot,
                const std::vector<double>& r_phi_plug, const std::vector<double>& r_ext_equip,
                const std::vector<double>& r_Q_dhw, bool aggregateByMonth);

            // Structure calculations helper
            void structureCalculations(double SHGC, double wallAreaM2, double windowAreaM2,
                double wallUValue, double windowUValue,
                double wallSolarAbsorption, double solarFactorWith,
                double solarFactorWithout, int direction);

            // Pre-computed constants for loop performance
            double invFloorArea, rhoCpAir_277, windImpactSupplyRatio, q4Pa, windImpactHz;
            double Am, Cm, shadingUsePerWPerM2, areaNaturallyLightedRatio, maxRatioElectricLighting;
            double elightNatural, hzone, h_ms, h_is, H_tris, hwindowWperkm2;
            double prs, prsInterior, prsSolar, prm, prmInterior, prmSolar, H_ms, hOpaqueWperkm2, hem;

            // Cached Arrays for physical properties
            double nlams[9], nla[9], sams[9], sa[9], htot[9], hWindow[9];
            double nlaWMovableShading[9], naturalLightRatio[9], naturalLightShadeRatioReduction[9];
            double saWMovableShading[9], solarRatio[9], solarShadeRatioReduction[9];

            // Schedule Arrays [24][7] used for O(1) hourly lookup
            double fixedVentilationSchedule[24][7], fixedExteriorEquipmentSchedule[24][7];
            double fixedInteriorEquipmentSchedule[24][7], fixedExteriorLightingSchedule[24][7];
            double fixedInteriorLightingSchedule[24][7], fixedActualHeatingSetpoint[24][7], fixedActualCoolingSetpoint[24][7];

            // Helpers from original code
            std::vector<double> sumHoursByMonth(const std::vector<double>& hourlyData);

            // Virtuals from Simulation.hpp - Removed 'override' to prevent VS2022 errors
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