/*
 * ISOHourly.h
 *
 *	Created on: Apr 28, 2014
 *			Author: craig
 */

#ifndef ISOHOURLY_H_
#define ISOHOURLY_H_
#include <isomodel/TimeFrame.hpp>
#include <isomodel/EpwData.hpp>
#include <isomodel/Structure.hpp>
#include <isomodel/Building.hpp>
#include <isomodel/Lighting.hpp>
#include <isomodel/Ventilation.hpp>
#include <isomodel/Cooling.hpp>
#include <isomodel/Heating.hpp>
#include <isomodel/Population.hpp>
#include <boost/shared_ptr.hpp>

namespace openstudio {
namespace isomodel {


class ISOHourly {
	// Variables that correspond to symbols in ISO13790 have the symbols noted in
	// LaTeX format in the comments.
	double fanDeltaPinPa;//Calculation.T15
	double fanN;//Calculation.T16
	double provisionalCFlowad;//Calculation.S106
	double solarPair;//Calculation.V95
	double intPair;//Calc.T95
	double presenceSensorAd;//Calc.E92 table **************
	double automaticAd;
	double presenceAutoAd;
	double manualSwitchAd;
	double presenceSensorLux;
	double automaticLux;
	double presenceAutoLux;
	double manualSwitchLux;//Finish Calc.E92 table ************
	double shadingRatioWtoM2;//E102
	double shadingMaximumUseRatio;//E101
	double ventDcpWindImpact;//G119
	double AtPerAFloor;//J97
	double hci;//P94
	double hri;//P95
	double inertialAm15;//K88 table *******************
	double inertialAm14;
	double inertialAm12;
	double inertiaParameter5AM;
	double inertiaParameter4AM;
	double inertiaParameter3AM;
	double inertiaParameter2AM;
	double inertiaParameter1AM;
	double calculationCm15;
	double calculationCm14;
	double calculationCm12;
	double inertiaParameter5CM;
	double inertiaParameter4CM;
	double inertiaParameter3CM;
	double inertiaParameter2CM;
	double inertiaParameter1CM;//end K88 table *************
	double maxRatioElectricLighting;
	double elightNatural;
	double areaNaturallyLighted;
	double areaNaturallyLightedRatio;
	double nlaWMovableShadingH;
	double naturalLightRatioH;
	double nlaWMovableShadingW;
	double naturalLightRatioW;
	double nlaWMovableShadingS;
	double K146;//XXX BAA@20140716: shadingUsePerWPerM2
	double CA150;//XXX BAA@20140716: naturalLightShadeRatioReductionH
	double BZ150;//XXX BAA@20140716: naturalLightShadeRatioReductionW
	double naturalLightRatioS;
	double BY150;//XXX BAA@20140716: naturalLightShadeRatioReductionS
	double nlaWMovableShadingE;
	double naturalLightRatioE;
	double BX150;//XXX BAA@20140716: naturalLightShadeRatioReductionE
	double nlaWMovableShadingN;
	double naturalLightRatioN;
	double BW150;//XXX BAA@20140716: naturalLightShadeRatioReductionN
	double saWMovableShadingH;
	double solarRatioH;
	double O150;//XXX BAA@20140716: solarShadeRatioReductionH
	double saWMovableShadingW;
	double solarRatioW;
	double N150;//XXX BAA@20140716: solarShadeRatioReductionW
	double saWMovableShadingS;
	double solarRatioS;
	double M150;//XXX BAA@20140716: solarShadeRatioReductionS
	double saWMovableShadingE;
	double solarRatioE;
	double L150;//XXX BAA@20140716: solarShadeRatioReductionE
	double saWMovableShadingN;
	double solarRatioN;
	double K150;//XXX BAA@20140716: solarShadeRatioReductionN
	double windImpactSupplyRatio;//I119
	double q4Pa;//XXX BAA@20140716: infiltrationM3PerHourAt4Pa - NEED TO CONFIRM.
	double P96;//XXX Calculation Sheet: What variable is this?
	double P97;//XXX BAA@20140716: heatTransferCoefficientMassToSurfWPerM2K ??? - h_{ms}
	double P98;//XXX BAA@20140716: heatTransferCoefficientAirToSurfWPerM2K ??? - h_{is}
	double his;
	double P89;// A_{m} XXX BAA@20140716: effectiveMassAreaM2 - redundant, only used to set inertiaAm.
	double inertiaAm; // A_{m}.
	double hwindowWperkm2;
	double prs;
	double prsInterior;
	double prsSolar;
	double prm;
	double prmInterior;
	double prmSolar;
	double hms; //XXX H_{ms} which is different from h_{ms} (P97). See ISO 13790 eq. 64.
	double hOpaqueWperkm2; // H_{op}
	double hem; // H_{em}
	double P90; // C_{m} XXX BAA@20140716: internalHeatCapacityJPerK - appears unused.
	double calculationCm; // C_{m} XXX BAA@20140716: only gets set with thermalMass > 11 ???
	double windImpactHz;//H119
	static const int NORTH,NORTHEAST,EAST,SOUTHEAST,SOUTH,SOUTHWEST,WEST,NORTHWEST,ROOF;


	//Calculated surface values
	double nlams[9];
	double nla[9];
	double sams[9];
	double sa[9];
	double htot[9];
	double hWindow[9];
	double electPriceUSDperMWh[TIMESLICES];

	//XXX Heat variables based off price reductions & monthly schedules -- EMCAC_UI
	double heatMonthBegin, heatMonthEnd;
	double heatSetpointStrategy;
	double heatSetpointUSDperMWh;
	double heatSetpointIncreaseDegC;

	//XXX Cooling variables based off price reductions & monthly schedules -- EMCAC_UI
	double coolMonthBegin, coolMonthEnd;
	double coolSetpointStrategy;
	double coolSetpointUSDperMWh;
	double coolSetpointReductionDegC;

	//XXX Equipment variables based off price reductionss -- EMCAC_UI
	double equipLoadReductionUSDperMWh;
	double equipControlStrategy;
	double equipLoadReductionFactor;

	//XXX External Equipment usage Q56
	double externalEquipment;

	//XXX Lighting variables based off price reductions -- EMCAC_UI
	double lightLoadReductionUSDperMWh;
	double lightControlStrategy;
	double lightLoadReductionFactor;

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
	std::vector<double > calculateHour(int hourOfYear, int month, int dayOfWeek, int hourOfDay,
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
		return (electPriceUSDperMWh[hourOfYear]>=heatSetpointUSDperMWh) ?
				(fixedActualHeatingSetpoint[(int)hourOfDay-1][(int)scheduleOffset-1]-heatSetpointStrategy*heatSetpointIncreaseDegC) :
				fixedActualHeatingSetpoint[(int)hourOfDay-1][(int)scheduleOffset-1];
	}

	/** Returns the cooling setpoint schedule. */
	virtual double coolingSetpointSchedule(int hourOfYear, int hourOfDay, int scheduleOffset){
		return (electPriceUSDperMWh[hourOfYear]>=coolSetpointUSDperMWh) ?
				(fixedActualCoolingSetpoint[(int)hourOfDay-1][(int)scheduleOffset-1]+coolSetpointStrategy*coolSetpointReductionDegC) :
				fixedActualCoolingSetpoint[(int)hourOfDay-1][(int)scheduleOffset-1];
	}

	/** Returns the cooling enabled schedule. */
	virtual double coolingEnabledSchedule(int hourOfYear, int month){
		if(coolMonthBegin<coolMonthEnd) {
			return (month>=coolMonthBegin && month<=coolMonthEnd) ? 1 : 0;
		} else {
			return (month>=coolMonthBegin || month<=coolMonthEnd) ? 1 : 0;
		}
	}

	/** Returns the heating enabled schedule. */
	virtual double heatingEnabledSchedule(int hourOfYear, int month){
		if(heatMonthBegin<heatMonthEnd){
			return (month>=heatMonthBegin && month<=heatMonthEnd) ? 1 : 0;
		} else {
			return (month>=heatMonthBegin || month<=heatMonthEnd) ? 1 : 0;
		}
	}
	boost::shared_ptr<Structure> structure;
	boost::shared_ptr<Building> building;
	boost::shared_ptr<Lighting> lighting;
	boost::shared_ptr<Ventilation> vent;
	boost::shared_ptr<Cooling> cool;
	boost::shared_ptr<Heating> heat;
	boost::shared_ptr<Population> pop;
	boost::shared_ptr<EpwData> weatherData;
public:
	ISOHourly();
	virtual ~ISOHourly();

	/** Calculates the building's hourly EUI using the "simple hourly method"
	 * described in ISO 13790:2008. The hourly calculations largely correspond to
	 * those described by the simple hourly method in ISO 13790 Annex C. A key
	 * difference is that this implementation describes everything in terms of
	 * EUI (i.e., per area). */
	void calculateHourly();

	/** Set the population. */
	void setPop(boost::shared_ptr<Population> value){pop=value;}

	/** Set the lighting. */
	void setLights(boost::shared_ptr<Lighting> value){lighting=value;}

	/** Set the building. */
	void setBuilding(boost::shared_ptr<Building> value){building=value;}

	/** Set the structure. */
	void setStructure(boost::shared_ptr<Structure> value){structure=value;}

	/** Set the heating. */
	void setHeating(boost::shared_ptr<Heating> value){heat=value;}

	/** Set the cooling. */
	void setCooling(boost::shared_ptr<Cooling> value){cool=value;}

	/** Set the ventilation. */
	void setVentilation(boost::shared_ptr<Ventilation> value){vent=value;}

	/** Set the weather data. */
	void setWeatherData(boost::shared_ptr<EpwData> value) {weatherData = value;}
};
}
}
#endif /* ISOHOURLY_H_ */
