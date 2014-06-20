/*
 * ISOHourly.h
 *
 *  Created on: Apr 28, 2014
 *      Author: craig
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
	double K146;//XXX Calculation Sheet: What variable is this?
	double CA150;//XXX Calculation Sheet: What variable is this?
	double BZ150;//XXX Calculation Sheet: What variable is this?
	double naturalLightRatioS;
	double BY150;//XXX Calculation Sheet: What variable is this?
	double nlaWMovableShadingE;
	double naturalLightRatioE;
	double BX150;//XXX Calculation Sheet: What variable is this?
	double nlaWMovableShadingN;
	double naturalLightRatioN;
	double BW150;//XXX Calculation Sheet: What variable is this?
	double saWMovableShadingH;
	double solarRatioH;
	double O150;//XXX Calculation Sheet: What variable is this?
	double saWMovableShadingW;
	double solarRatioW;
	double N150;//XXX Calculation Sheet: What variable is this?
	double saWMovableShadingS;
	double solarRatioS;
	double M150;//XXX Calculation Sheet: What variable is this?
	double saWMovableShadingE;
	double solarRatioE;
	double L150;//XXX Calculation Sheet: What variable is this?
	double saWMovableShadingN;
	double solarRatioN;
	double K150;//XXX Calculation Sheet: What variable is this?
	double windImpactSupplyRatio;//I119
	double q4Pa;//XXX SingleBldg.Q6
	double P96;//XXX Calculation Sheet: What variable is this?
	double P97;//XXX Calculation Sheet: What variable is this?
	double P98;//XXX Calculation Sheet: What variable is this?
	double his;
	double P89;
	double inertiaAm;
	double hwindowWperkm2;
	double prs;
	double prsInterior;
	double prsSolar;
	double prm;
	double prmInterior;
	double prmSolar;
	double hms;
	double hOpaqueWperkm2;
	double hem;
	double P90;//XXX Calculation Sheet: What variable is this?
	double calculationCm;
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


	void populateSchedules();

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

	virtual double fanSchedule(int hourOfYear, int hourOfDay, int scheduleOffset){
		return fixedFanSchedule[(int)hourOfDay-1][(int)scheduleOffset-1];
	}
	virtual double ventilationSchedule(int hourOfYear, int hourOfDay, int scheduleOffset){
		return fixedVentilationSchedule[(int)hourOfDay-1][(int)scheduleOffset-1];
	}
	virtual double exteriorEquipmentSchedule(int hourOfYear, int hourOfDay, int scheduleOffset){
		return fixedExteriorEquipmentSchedule[(int)hourOfDay-1][(int)scheduleOffset-1];
	}
	virtual double interiorEquipmentSchedule(int hourOfYear, int hourOfDay, int scheduleOffset){
		return fixedInteriorEquipmentSchedule[(int)hourOfDay-1][(int)scheduleOffset-1];
	}
	virtual double exteriorLightingSchedule(int hourOfYear, int hourOfDay, int scheduleOffset){
		return fixedExteriorLightingSchedule[(int)hourOfDay-1][(int)scheduleOffset-1];
	}
	virtual double interiorLightingSchedule(int hourOfYear, int hourOfDay, int scheduleOffset){
		return fixedInteriorLightingSchedule[(int)hourOfDay-1][(int)scheduleOffset-1];
	}
	virtual double heatingSetpointSchedule(int hourOfYear, int hourOfDay, int scheduleOffset){
		return (electPriceUSDperMWh[hourOfYear]>=heatSetpointUSDperMWh) ?
				(fixedActualHeatingSetpoint[(int)hourOfDay-1][(int)scheduleOffset-1]-heatSetpointStrategy*heatSetpointIncreaseDegC) :
				fixedActualHeatingSetpoint[(int)hourOfDay-1][(int)scheduleOffset-1];
	}
	virtual double coolingSetpointSchedule(int hourOfYear, int hourOfDay, int scheduleOffset){
		return (electPriceUSDperMWh[hourOfYear]>=coolSetpointUSDperMWh) ?
				(fixedActualCoolingSetpoint[(int)hourOfDay-1][(int)scheduleOffset-1]+coolSetpointStrategy*coolSetpointReductionDegC) :
				fixedActualCoolingSetpoint[(int)hourOfDay-1][(int)scheduleOffset-1];
	}
	virtual double coolingEnabledSchedule(int hourOfYear, int month){
		if(coolMonthBegin<coolMonthEnd) {
			return (month>=coolMonthBegin && month<=coolMonthEnd) ? 1 : 0;
		} else {
			return (month>=coolMonthBegin || month<=coolMonthEnd) ? 1 : 0;
		}
	}
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
	void calculateHourly();
  void setPop(boost::shared_ptr<Population> value){pop=value;}
  void setLights(boost::shared_ptr<Lighting> value){lighting=value;}
  void setBuilding(boost::shared_ptr<Building> value){building=value;}
  void setStructure(boost::shared_ptr<Structure> value){structure=value;}
  void setHeating(boost::shared_ptr<Heating> value){heat=value;}
  void setCooling(boost::shared_ptr<Cooling> value){cool=value;}
  void setVentilation(boost::shared_ptr<Ventilation> value){vent=value;}
  void setWeatherData(boost::shared_ptr<EpwData> value) {weatherData = value;}
};
}
}
#endif /* ISOHOURLY_H_ */
