/*
 * ISOHourly.h
 *
 *	Created on: Apr 28, 2014
 *			Author: craig
 */

#ifndef ISOHOURLY_H_
#define ISOHOURLY_H_

#include "TimeFrame.hpp"
#include "EpwData.hpp"
#include "Structure.hpp"
#include "Building.hpp"
#include "Lighting.hpp"
#include "Ventilation.hpp"
#include "Cooling.hpp"
#include "Heating.hpp"
#include "Population.hpp"
#include "SimModel.hpp"

#include <boost/shared_ptr.hpp>
#include <map>
#include <string>

namespace openstudio {
namespace isomodel {


class ISOHourly {
	// BAA@20150717: Variables that correspond to symbols in ISO 13790 have the symbols noted
	// in LaTeX format in the comments. Symbols from other standards have their
	// standard noted. Symbols are case sensitive, e.g., H_{ms} is different than
	// h_{ms}. References to spreadsheet cells are from the Gerogia Tech
	// ISOHourly spreadsheet.  Suggested name changes are marked with XXX. If I'm
	// not confident in the suggested name, it's followed with '???'.	

	// Fans. http://www.engineeringtoolbox.com/fans-efficiency-power-consumption-d_197.html
	double fanDeltaPinPa; // dp. Total pressure increase in the fan. Calculation.T15
	double fanN; // \mu_{f}. Fan efficiency. Calculation.T16

	// Lighting controls.
	// Occupancy based lighting control lighting use adjustment factors.
	double presenceSensorAd; // Calc.E92 table **************
	double automaticAd;
	double presenceAutoAd;
	double manualSwitchAd;
	// Daylight based lighting control target lux levels.
	double presenceSensorLux;
	double automaticLux;
	double presenceAutoLux;
	double manualSwitchLux; // Finish Calc.E92 table ************
	// Used to determine the amount of electric light used.
	double maxRatioElectricLighting; // Ratio of electric light used due to lighting controls.
	double elightNatural; // Target lux level in naturally lit area.

	// Ventilation from wind. ISO 15242
	double ventDcpWindImpact; // ISO 15242 dcp. G119
	double windImpactSupplyRatio;//I119
	double q4Pa; // ISO 15242 Q_{4Pa}. XXX infiltrationM3PerHourAt4Pa ???
	double windImpactHz; // ISO 15242 H_{z}. H119

	// Geometry
	double AtPerAFloor; // \Lambda_{at}. Ratio of total interior surface area to floor area. J97

	// Thermal Mass
	double Am; // A_{m}. XXX: effectiveMassAreaM2
	double Cm; // C_{m}. XXX: internalHeatCapacityJPerK 

	// Movable shading.
	// These three variables are used to model movable shading. ISO 13790 does it
	// by switching between g_{gl} and g_{gl+sh}. The method here allows varying
	// degrees of shading rather than just on or off.
	double shadingMaximumUseRatio; // The shading factor of the movable shading when in full use. E101
	double shadingRatioWtoM2; // The irradiance at which shading is in full use. E102
	double shadingUsePerWPerM2; // K146. The shading factor per unit irradiance. XXX: shadingUsePerWPerM2

	double areaNaturallyLighted;
	double areaNaturallyLightedRatio;
	double nlaWMovableShadingH;
	double naturalLightRatioH;
	double nlaWMovableShadingW;
	double naturalLightRatioW;
	double nlaWMovableShadingS;
	double naturalLightShadeRatioReductionH; // CA150 XXX naturalLightShadeRatioReductionH
	double naturalLightShadeRatioReductionW; // BZ150 XXX naturalLightShadeRatioReductionW
	double naturalLightRatioS;
	double naturalLightShadeRatioReductionS; // BY150 XXX naturalLightShadeRatioReductionS
	double nlaWMovableShadingE;
	double naturalLightRatioE;
	double naturalLightShadeRatioReductionE; // BX150 XXX naturalLightShadeRatioReductionE
	double nlaWMovableShadingN;
	double naturalLightRatioN;
	double naturalLightShadeRatioReductionN; // BW150 XXX naturalLightShadeRatioReductionN
	double saWMovableShadingH;
	double solarRatioH;
	double solarShadeRatioReductionH; // O150 XXX solarShadeRatioReductionH
	double saWMovableShadingW;
	double solarRatioW;
	double solarShadeRatioReductionW; // N150 XXX solarShadeRatioReductionW
	double saWMovableShadingS;
	double solarRatioS;
	double solarShadeRatioReductionS; // M150 XXX solarShadeRatioReductionS
	double saWMovableShadingE;
	double solarRatioE;
	double solarShadeRatioReductionE; // L150 XXX solarShadeRatioReductionE
	double saWMovableShadingN;
	double solarRatioN;
	double solarShadeRatioReductionN; // K150 XXX solarShadeRatioReductionN

	double hci; // =2.5. P94
	double hri; // =5.5. P95
	double P96; // =hri*1.2 XXX Calculation Sheet: What variable is this?
	double P97; // h_{ms} XXX heatTransferCoefficientMassToSurfWPerM2K ???
	double P98; // 1/h_{is}. Not sure why inverted. XXX heatTransferCoefficientAirToSurfWPerM2K ???

	double his; // H_{tr,is}
	double hwindowWperkm2; // H_{tr,w}.

	// \Phi_{st} and \Phi_{m} are calculated differently than in ISO 13790 to
	// allow variation in the values that factor the amount of interior and solar
	// heat gain that heats the air. These variables are used in those
	// calculations.
	double solarPair; // Fraction of solar heat gain that heats the air. Calculation.V95
	double intPair; // Fraction of interior heat gain that heats the air. Calc.T95
	double prs; // Constant part of \Phi_{st}.
	double prsInterior; // Interior part of \Phi_{st}.
	double prsSolar; // Solar part of \Phi_{st}.
	double prm; // Constant part of \Phi_{m}.
	double prmInterior; // Interior part of \Phi_{m}.
	double prmSolar; // Solar part of \Phi_{m}.

	double hms; // H_{ms}
	double hOpaqueWperkm2; // H_{op}
	double hem; // H_{em}

	static const int NORTH,NORTHEAST,EAST,SOUTHEAST,SOUTH,SOUTHWEST,WEST,NORTHWEST,ROOF;

	//Calculated surface values
	double nlams[9];
	double nla[9];
	double sams[9];
	double sa[9];
	double htot[9];
	double hWindow[9];
	double electPriceUSDperMWh[TIMESLICES];

	//XXX External Equipment usage Q56
	double externalEquipment;

	//XXX Unknown Lighting Variables
	double electInternalGains;
	double permLightPowerDensityWperM2;

	//XXX Unknown Vent Variables
	double ventPreheatDegC;

	double fixedVentilationSchedule[24][7];
	double fixedFanSchedule[24][7];
	double fixedExteriorEquipmentSchedule[24][7];
	double fixedInteriorEquipmentSchedule[24][7];
	double fixedExteriorLightingSchedule[24][7];
	double fixedInteriorLightingSchedule[24][7];
	double fixedActualHeatingSetpoint[24][7];
	double fixedActualCoolingSetpoint[24][7];

	// XXX Unused variables.
	double provisionalCFlowad; // Appears to be unused. Calculation.S106

protected:

	/** Populates the ventilation, fan, exterior equipment, interior equipment,
	 * exterior lighting, interior lighting, heating setpoint, and cooling
	 * setpoint schedules. */
	void populateSchedules();

	/** Calculates the energy use for one hour and sets the state for the next
	 * hour. The hourly calculations largely correspond to those described by the
	 * simple hourly method in ISO 13790 Annex C. A key difference is that this
	 * implementation describes everything in terms of EUI (i.e., per area). Any
	 * discrepency in units where this code uses "units per area" while the
	 * standard just uses "units" is likely due to this difference. */
	std::map<std::string, double> calculateHour(int hourOfYear, int month, int dayOfWeek, int hourOfDay,
			double windMps,	double temperature, double electPriceUSDperMWh,
			double solarRadiationN, double solarRadiationE,
			double solarRadiationS, double solarRadiationW,
			double solarRadiationH,
			double& TMT1, double& tiHeatCool);
	void initialize();
	void structureCalculations(double SHGC,
			double wallAreaM2, double windowAreaM2,
			double wallUValue, double windowUValue,
			double wallSolarAbsorption,
			double solarFactorWith, double solarFactorWithout,
			int direction){
		double WindowT = SHGC / 0.87;
		nlams[direction] = windowAreaM2 * WindowT;//natural lighted area movable shade
		nla[direction] = windowAreaM2 * WindowT;//natural lighted area
		sams[direction] = wallAreaM2 * (wallSolarAbsorption*windowUValue/23) + windowAreaM2 * solarFactorWith;
		sa[direction] = wallAreaM2 * (wallSolarAbsorption*windowUValue/23) + windowAreaM2 * solarFactorWithout;
		htot[direction] = wallAreaM2*wallUValue + windowAreaM2 * windowUValue;
		hWindow[direction] = windowAreaM2 * windowUValue;
	}

	/** Returns the fan schedule. */
	virtual double fanSchedule(int hourOfYear, int hourOfDay, int scheduleOffset){
		return fixedFanSchedule[(int)hourOfDay-1][(int)scheduleOffset-1];
	}

	/** Returns the ventilation schedule. */
	virtual double ventilationSchedule(int hourOfYear, int hourOfDay, int scheduleOffset){
		return fixedVentilationSchedule[(int)hourOfDay-1][(int)scheduleOffset-1];
	}

	/** Returns the exterior equipment schedule. */
	virtual double exteriorEquipmentSchedule(int hourOfYear, int hourOfDay, int scheduleOffset){
		return fixedExteriorEquipmentSchedule[(int)hourOfDay-1][(int)scheduleOffset-1];
	}

	/** Returns the interior equipment schedule. */
	virtual double interiorEquipmentSchedule(int hourOfYear, int hourOfDay, int scheduleOffset){
		return fixedInteriorEquipmentSchedule[(int)hourOfDay-1][(int)scheduleOffset-1];
	}

	/** Returns the exterior lighting schedule. */
	virtual double exteriorLightingSchedule(int hourOfYear, int hourOfDay, int scheduleOffset){
		return fixedExteriorLightingSchedule[(int)hourOfDay-1][(int)scheduleOffset-1];
	}

	/** Returns the interior lighting schedule. */
	virtual double interiorLightingSchedule(int hourOfYear, int hourOfDay, int scheduleOffset){
		return fixedInteriorLightingSchedule[(int)hourOfDay-1][(int)scheduleOffset-1];
	}

	/** Returns the heating setpoint schedule. */
	virtual double heatingSetpointSchedule(int hourOfYear, int hourOfDay, int scheduleOffset){
		return fixedActualHeatingSetpoint[(int)hourOfDay-1][(int)scheduleOffset-1];
	}

	/** Returns the cooling setpoint schedule. */
	virtual double coolingSetpointSchedule(int hourOfYear, int hourOfDay, int scheduleOffset){
		return fixedActualCoolingSetpoint[(int)hourOfDay-1][(int)scheduleOffset-1];
	}

	boost::shared_ptr<Population> pop;
	boost::shared_ptr<Lighting> lights;
	boost::shared_ptr<Building> building;
	boost::shared_ptr<Structure> structure;
	boost::shared_ptr<Heating> heating;
	boost::shared_ptr<Cooling> cooling;
	boost::shared_ptr<Ventilation> ventilation;
	boost::shared_ptr<EpwData> weatherData;
public:
	ISOHourly();
	virtual ~ISOHourly();

	/** Calculates the building's hourly EUI using the "simple hourly method"
	 * described in ISO 13790:2008. The hourly calculations largely correspond to
	 * those described by the simple hourly method in ISO 13790 Annex C. A key
	 * difference is that this implementation describes everything in terms of
	 * EUI (i.e., per area). */
	ISOResults calculateHourly();

	/** Set the population. */
	void setPop(boost::shared_ptr<Population> value){pop=value;}

	/** Set the lighting. */
	void setLights(boost::shared_ptr<Lighting> value){lights=value;}

	/** Set the building. */
	void setBuilding(boost::shared_ptr<Building> value){building=value;}

	/** Set the structure. */
	void setStructure(boost::shared_ptr<Structure> value){structure=value;}

	/** Set the heating. */
	void setHeating(boost::shared_ptr<Heating> value){heating=value;}

	/** Set the cooling. */
	void setCooling(boost::shared_ptr<Cooling> value){cooling=value;}

	/** Set the ventilation. */
	void setVentilation(boost::shared_ptr<Ventilation> value){ventilation=value;}

	/** Set the weather data. */
	void setWeatherData(boost::shared_ptr<EpwData> value) {weatherData = value;}
};
}
}
#endif /* ISOHOURLY_H_ */
