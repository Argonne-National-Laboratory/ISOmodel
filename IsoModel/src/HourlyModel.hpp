/*
 * HourlyModel.hpp
 */

 /**
 * HourlyModel.hpp
 * * PERFORMANCE REFACTOR: HIGH-CONFIDENCE OPTIMIZATIONS
 * 1. Constant Hoisting: Added member variables for loop-invariant thermal
 * parameters (H_tris, Cm, invFloorArea, etc.) to allow pre-calculation.
 * 2. Schedule Flattening: Added 2D arrays [24][7] to cache schedules,
 * eliminating expensive map lookups during simulation.
 * 3. Fast Math Utilities: Integrated fastPow23 inline helper to replace
 * generic std::pow for cube-root calculations.
 * 4. Cache-Aligned Storage: Structured member variables to favor a
 * single-pass simulation flow.
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

        // Fast x^(2/3) approximation
        inline double fastPow23(double x) {
            return std::cbrt(x * x);
        }

        class ISOMODEL_API HourlyModel : public Simulation
        {
        public:
            HourlyModel();
            virtual ~HourlyModel();

            std::vector<EndUses> simulate(bool aggregateByMonth = false);

        private:
            void populateSchedules();
            void initialize();

            void structureCalculations(double SHGC, double wallAreaM2, double windowAreaM2,
                double wallUValue, double windowUValue,
                double wallSolarAbsorption, double solarFactorWith,
                double solarFactorWithout, int direction);

            // Member variables for hoisted constants
            double invFloorArea, rhoCpAir_277, windImpactSupplyRatio, q4Pa, windImpactHz;
            double Am, Cm, shadingUsePerWPerM2, areaNaturallyLightedRatio, maxRatioElectricLighting, elightNatural;
            double h_ms, h_is, H_tris, hwindowWperkm2;
            double prs, prsInterior, prsSolar, prm, prmInterior, prmSolar;
            double H_ms_val, hOpaqueWperkm2, hem;

            // Cached Arrays
            double nlams[9], nla[9], sams[9], sa[9], htot[9], hWindow[9];
            double nlaWMovableShading[9], naturalLightRatio[9], naturalLightShadeRatioReduction[9];
            double saWMovableShading[9], solarRatio[9], solarShadeRatioReduction[9];

            // Schedules [24][7]
            double fixedVentilationSchedule[24][7];
            double fixedExteriorEquipmentSchedule[24][7];
            double fixedInteriorEquipmentSchedule[24][7];
            double fixedExteriorLightingSchedule[24][7];
            double fixedInteriorLightingSchedule[24][7];
            double fixedActualHeatingSetpoint[24][7];
            double fixedActualCoolingSetpoint[24][7];

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