/*
 * HourlyModel.h
 *
 *  Created on: Apr 28, 2014
 *      Author: craig
 */

#ifndef ISOMODEL_HOURLYMODEL_HPP
#define ISOMODEL_HOURLYMODEL_HPP

#include "ISOModelAPI.hpp"

#include "ISOResults.hpp"
#include "Simulation.hpp"
#include "TimeFrame.hpp"
#include "MonthlyModel.hpp"

#include <memory>
#include <map>
#include <string>
#include <numeric>
#include <algorithm>
#include <vector>
#include <functional>
#include <iterator>
#include <array> // added

#ifdef ISOMODEL_STANDALONE
#include "EndUses.hpp"
#include "Vector.hpp"
#else
#include "../utilities/data/EndUses.hpp"
#include "../utilities/data/Vector.hpp"
#endif

namespace openstudio {
    namespace isomodel {

        // Struct to hold the results of each hour and the vector of results of all the hours.
        template<typename T>
        struct HourResults
        {
            T Qneed_ht;
            T Qneed_cl;
            T Q_illum_tot;
            T Q_illum_ext_tot;
            T Qfan_tot;
            T Qpump_tot;
            T phi_plug;
            T externalEquipmentEnergyWperm2;
            T Q_dhw;
        };

        class ISOMODEL_API HourlyModel : public Simulation
        {
        public:
            /**
             * Creates an empty HourlyModel. Generally, the HourlyModel should be created using the UserModel::toHourlyModel() method.
             */
            HourlyModel();
            virtual ~HourlyModel();

            /**
             * Calculates the building's hourly EUI using the "simple hourly method"
             * described in ISO 13790:2008. The hourly calculations largely correspond to
             * those described by the simple hourly method in ISO 13790 Annex C. A key
             * difference is that this implementation describes everything in terms of
             * EUI (i.e., per area) throughout the calculations. The end results are the
             * same, but it's important to know that the intermediate results are generally
             * in terms of EUI if you need them for any reason.
             */
            std::vector<EndUses> simulate(bool aggregateByMonth = false);

        private:
            /**
             * Populates the ventilation, fan, exterior equipment, interior equipment,
             * exterior lighting, interior lighting, heating setpoint, and cooling
             * setpoint schedules.
             */
            void populateSchedules();

            void initialize();

            /**
             * Calculates the energy use for one hour and sets the state for the next
             * hour. The hourly calculations largely correspond to those described by the
             * simple hourly method in ISO 13790 Annex C. A key difference is that this
             * implementation describes everything in terms of EUI (i.e., per area). Any
             * discrepency in units where this code uses "units per area" while the
             * standard just uses "units" is likely due to this difference.
             *
             * NOTE: The public interface is unchanged; this method remains private.
             * Internally we now pass a fixed-size array instead of std::vector.
             */
            void calculateHour(int hourOfYear,
                int month,
                int dayOfWeek,
                int hourOfDay,
                double windMps,
                double temperature,
                const std::array<double, 9>& solarRadiation,
                double& TMT1,
                double& tiHeatCool,
                HourResults<double>& results);

            void structureCalculations(double SHGC,
                double wallAreaM2,
                double windowAreaM2,
                double wallUValue,
                double windowUValue,
                double wallSolarAbsorption,
                double solarFactorWith,
                double solarFactorWithout,
                int direction);

            std::vector<double> sumHoursByMonth(const std::vector<double>& hourlyData);

            /** Returns the ventilation schedule. */
            virtual double ventilationSchedule(int hourOfYear, int hourOfDay, int scheduleOffset) {
                return fixedVentilationSchedule[(int)hourOfDay][(int)scheduleOffset];
            }

            /** Returns the exterior equipment schedule. */
            virtual double exteriorEquipmentSchedule(int hourOfYear, int hourOfDay, int scheduleOffset) {
                return fixedExteriorEquipmentSchedule[(int)hourOfDay][(int)scheduleOffset];
            }

            /** Returns the interior equipment schedule. */
            virtual double interiorEquipmentSchedule(int hourOfYear, int hourOfDay, int scheduleOffset) {
                return fixedInteriorEquipmentSchedule[(int)hourOfDay][(int)scheduleOffset];
            }

            /** Returns the exterior lighting schedule. */
            virtual double exteriorLightingSchedule(int hourOfYear, int hourOfDay, int scheduleOffset) {
                return fixedExteriorLightingSchedule[(int)hourOfDay][(int)scheduleOffset];
            }

            /** Returns the interior lighting schedule. */
            virtual double interiorLightingSchedule(int hourOfYear, int hourOfDay, int scheduleOffset) {
                return fixedInteriorLightingSchedule[(int)hourOfDay][(int)scheduleOffset];
            }

            /** Returns the heating setpoint schedule. */
            virtual double heatingSetpointSchedule(int hourOfYear, int hourOfDay, int scheduleOffset) {
                return fixedActualHeatingSetpoint[(int)hourOfDay][(int)scheduleOffset];
            }

            /** Returns the cooling setpoint schedule. */
            virtual double coolingSetpointSchedule(int hourOfYear, int hourOfDay, int scheduleOffset) {
                return fixedActualCoolingSetpoint[(int)hourOfDay][(int)scheduleOffset];
            }

            // Lighting controls.
            // Used to determine the amount of electric light used.
            double maxRatioElectricLighting; // Ratio of electric light used due to lighting controls.
            double elightNatural; // Target lux level in naturally lit area.

            // Ventilation from wind. ISO 15242
            double windImpactSupplyRatio; //I119
            double q4Pa; // ISO 15242 Q_{4Pa}. XXX infiltrationM3PerHourAt4Pa ???
            double windImpactHz; // ISO 15242 H_{z}. H119

            // Thermal Mass
            double Am; // A_{m}. XXX: effectiveMassAreaM2
            double Cm; // C_{m}. XXX: internalHeatCapacityJPerK 

            // Movable shading.
            // These variables are used to model movable shading. ISO 13790 does it
            // by switching between g_{gl} and g_{gl+sh}. The method here allows varying
            // degrees of shading rather than just on or off.
            double shadingUsePerWPerM2; // K146. The shading factor per unit irradiance.

            double areaNaturallyLighted;
            double areaNaturallyLightedRatio;

            // changed from std::vector<double> to fixed-size arrays (private; no API impact)
            std::array<double, 9> nlaWMovableShading;
            std::array<double, 9> naturalLightRatio;
            std::array<double, 9> naturalLightShadeRatioReduction;

            std::array<double, 9> saWMovableShading;
            std::array<double, 9> solarRatio;
            std::array<double, 9> solarShadeRatioReduction;

            // Fan power constants.

            // Wind constants.
            double hzone; // Not totally clear what this is. Something wind related.

            // Heat transfer coefficients.
            double h_ms; // h_{ms} Heat transfer coefficient, mass(m) to surface(s).
            double h_is; // h_{is} Heat transfer coefficient, air(s) to surface(s).

            double H_tris; // H_{tr,is}. Coupling conductance from air(i) to surface(s).
            double hwindowWperkm2; // H_{tr,w}.

            // \Phi_{st} and \Phi_{m} are calculated differently than in ISO 13790
            double prs; // Constant part of \Phi_{st}.
            double prsInterior; // Interior part of \Phi_{st}.
            double prsSolar; // Solar part of \Phi_{st}.
            double prm; // Constant part of \Phi_{m}.
            double prmInterior; // Interior part of \Phi_{m}.
            double prmSolar; // Solar part of \Phi_{m}.

            double H_ms; // H_{ms}
            double hOpaqueWperkm2; // H_{op}
            double hem; // H_{em}

            // Calculated surface values
            double nlams[9];
            double nla[9];
            double sams[9]; // ISO13790 11.3.4
            double sa[9];   // ISO13790 11.3.4
            double htot[9];
            double hWindow[9];

            double fixedVentilationSchedule[24][7];
            double fixedExteriorEquipmentSchedule[24][7];
            double fixedInteriorEquipmentSchedule[24][7];
            double fixedExteriorLightingSchedule[24][7];
            double fixedInteriorLightingSchedule[24][7];
            double fixedActualHeatingSetpoint[24][7];
            double fixedActualCoolingSetpoint[24][7];

            // XXX Unused variables.
            double provisionalCFlowad = 1; // Appears to be unused. Calculation.S106
        };
    }
}
#endif /* ISOMODEL_HOURLYMODEL_HPP */