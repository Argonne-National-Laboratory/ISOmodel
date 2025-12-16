/*
 * HourlyModel.hpp
 *
 * Full restored version including all support methods and precomputed members.
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

        // Helper for x^0.667 ~ x^(2/3) using cbrt(x^2). Faster than std::pow.
        inline double fastPow23(double x) {
            return std::cbrt(x * x);
        }

        class ISOMODEL_API HourlyModel : public Simulation
        {
        public:
            HourlyModel();
            virtual ~HourlyModel();

            // Main entry point for the simulation
            std::vector<EndUses> simulate(bool aggregateByMonth = false);

        private:
            // Support methods for pre-simulation setup
            void populateSchedules();
            void initialize();

            // Structure calculations helper
            void structureCalculations(double SHGC, double wallAreaM2, double windowAreaM2,
                double wallUValue, double windowUValue,
                double wallSolarAbsorption, double solarFactorWith,
                double solarFactorWithout, int direction);

            // Pre-computed constants for loop performance
            double invFloorArea;
            double rhoCpAir_277;
            double windImpactSupplyRatio;
            double q4Pa;
            double windImpactHz;
            double Am;
            double Cm;
            double shadingUsePerWPerM2;
            double areaNaturallyLightedRatio;
            double maxRatioElectricLighting;
            double elightNatural;
            double hzone;
            double h_ms, h_is, H_tris, hwindowWperkm2;
            double prs, prsInterior, prsSolar;
            double prm, prmInterior, prmSolar;
            double H_ms, hOpaqueWperkm2, hem;

            // Cached Arrays for physical properties
            double nlams[9];
            double nla[9];
            double sams[9];
            double sa[9];
            double htot[9];
            double hWindow[9];
            double nlaWMovableShading[9];
            double naturalLightRatio[9];
            double naturalLightShadeRatioReduction[9];
            double saWMovableShading[9];
            double solarRatio[9];
            double solarShadeRatioReduction[9];

            // Schedule Arrays [24][7] used for O(1) hourly lookup
            double fixedVentilationSchedule[24][7];
            double fixedExteriorEquipmentSchedule[24][7];
            double fixedInteriorEquipmentSchedule[24][7];
            double fixedExteriorLightingSchedule[24][7];
            double fixedInteriorLightingSchedule[24][7];
            double fixedActualHeatingSetpoint[24][7];
            double fixedActualCoolingSetpoint[24][7];

            // Helpers from original code
            std::vector<double> sumHoursByMonth(const std::vector<double>& hourlyData);

            // Virtuals from Simulation.hpp
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