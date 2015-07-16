/*
 * ISOModel_GTest.cpp
 *
 *  Created on: Dec 5, 2014
 *      Author: nick
 */

#include "gtest/gtest.h"
#include "TestEnvironment.h"

#include "../Properties.hpp"
#include "../UserModel.hpp"

using namespace openstudio::isomodel;

TEST(IsoModelTests, InitializationTests)
{
  UserModel userModel;
  userModel.load(test_data_path + "/ism_props_for_testing_umodel_init_v2.ism");

  EXPECT_DOUBLE_EQ(0.366569597990189, userModel.terrainClass());
  EXPECT_DOUBLE_EQ(0.13797878192703, userModel.floorArea());
  EXPECT_DOUBLE_EQ(0.425419263581922, userModel.buildingHeight());
  EXPECT_DOUBLE_EQ(0.665995505182317, userModel.buildingOccupancyFrom());
  EXPECT_DOUBLE_EQ(0.400372234106352, userModel.buildingOccupancyTo());
  EXPECT_DOUBLE_EQ(0.254850243633116, userModel.equivFullLoadOccupancyFrom());
  EXPECT_DOUBLE_EQ(0.713362082549865, userModel.equivFullLoadOccupancyTo());
  EXPECT_DOUBLE_EQ(0.0453028919599623, userModel.peopleDensityOccupied());
  EXPECT_DOUBLE_EQ(0.374398515315959, userModel.peopleDensityUnoccupied());
  EXPECT_DOUBLE_EQ(0.308476836073534, userModel.heatingOccupiedSetpoint());
  EXPECT_DOUBLE_EQ(0.96115521837837, userModel.heatingUnoccupiedSetpoint());
  EXPECT_DOUBLE_EQ(0.0182141291000549, userModel.coolingOccupiedSetpoint());
  EXPECT_DOUBLE_EQ(0.47279017381788, userModel.coolingUnoccupiedSetpoint());
  EXPECT_DOUBLE_EQ(0.0159043563230605, userModel.elecPowerAppliancesOccupied());
  EXPECT_DOUBLE_EQ(0.877197046873451, userModel.elecPowerAppliancesUnoccupied());
  EXPECT_DOUBLE_EQ(0.413231779700794, userModel.gasPowerAppliancesOccupied());
  EXPECT_DOUBLE_EQ(0.735954395099727, userModel.gasPowerAppliancesUnoccupied());
  EXPECT_DOUBLE_EQ(0.827607402688993, userModel.lightingPowerIntensityOccupied());
  EXPECT_DOUBLE_EQ(0.210627783574828, userModel.lightingPowerIntensityUnoccupied());
  EXPECT_DOUBLE_EQ(0.688613300586997, userModel.exteriorLightingPower());
  EXPECT_DOUBLE_EQ(0.952066322499152, userModel.daylightSensorSystem());
  EXPECT_DOUBLE_EQ(0.191200546809349, userModel.lightingOccupancySensorSystem());
  EXPECT_DOUBLE_EQ(0.295905191092175, userModel.constantIlluminationControl());
  EXPECT_DOUBLE_EQ(0.977647331541828, userModel.coolingSystemCOP());
  EXPECT_DOUBLE_EQ(0.86953551426846, userModel.coolingSystemIPLVToCOPRatio());
  EXPECT_DOUBLE_EQ(1, userModel.heatingEnergyCarrier());
  EXPECT_DOUBLE_EQ(0.710454137223511, userModel.heatingSystemEfficiency());
  EXPECT_DOUBLE_EQ(3, userModel.ventilationType());
  EXPECT_DOUBLE_EQ(0.903704085971796, userModel.freshAirFlowRate());
  EXPECT_DOUBLE_EQ(0.724248760195895, userModel.supplyExhaustRate());
  EXPECT_DOUBLE_EQ(0.49985550202677, userModel.heatRecovery());
  EXPECT_DOUBLE_EQ(0.846564029275989, userModel.exhaustAirRecirclation());
  EXPECT_DOUBLE_EQ(0.139585598177502, userModel.buildingAirLeakage());
  EXPECT_DOUBLE_EQ(0.881916031629701, userModel.dhwDemand());
  EXPECT_DOUBLE_EQ(0.105230439331114, userModel.dhwEfficiency());
  // this is no longer set, we should delete the accessors for it
  // and the variable
  // EXPECT_DOUBLE_EQ(0.791092991177229, userModel.dhwDistributionSystem());
  EXPECT_DOUBLE_EQ(2, userModel.dhwEnergyCarrier());
  EXPECT_DOUBLE_EQ(3, userModel.bemType());
  EXPECT_DOUBLE_EQ(0.590020871911987, userModel.interiorHeatCapacity());
  EXPECT_DOUBLE_EQ(0.256509943938684, userModel.specificFanPower());
  EXPECT_DOUBLE_EQ(0.171213718831364, userModel.fanFlowControlFactor());
  // this is no longer set, we should delete the accessors for it
  // and the variable. Probably replaced by skylightSHGC
  // EXPECT_DOUBLE_EQ(0.577629926945883, userModel.roofSHGC());
  EXPECT_DOUBLE_EQ(0.351700449083525, userModel.wallAreaS());
  EXPECT_DOUBLE_EQ(0.638796629077831, userModel.wallAreaSE());
  EXPECT_DOUBLE_EQ(0.713877579934114, userModel.wallAreaE());
  EXPECT_DOUBLE_EQ(0.0544635225207429, userModel.wallAreaNE());
  EXPECT_DOUBLE_EQ(0.713312047950444, userModel.wallAreaN());
  EXPECT_DOUBLE_EQ(0.316883353660591, userModel.wallAreaNW());
  EXPECT_DOUBLE_EQ(0.963602582100428, userModel.wallAreaW());
  EXPECT_DOUBLE_EQ(0.950016805325306, userModel.wallAreaSW());
  EXPECT_DOUBLE_EQ(0.401348851386038, userModel.roofArea());
  EXPECT_DOUBLE_EQ(0.479173557940235, userModel.wallUvalueS());
  EXPECT_DOUBLE_EQ(0.598665235979741, userModel.wallUvalueSE());
  EXPECT_DOUBLE_EQ(0.592537203218594, userModel.wallUvalueE());
  EXPECT_DOUBLE_EQ(0.317076189922438, userModel.wallUvalueNE());
  EXPECT_DOUBLE_EQ(0.857610736439619, userModel.wallUvalueN());
  EXPECT_DOUBLE_EQ(0.494959077705813, userModel.wallUvalueNW());
  EXPECT_DOUBLE_EQ(0.710302412967452, userModel.wallUvalueW());
  EXPECT_DOUBLE_EQ(0.755347362509827, userModel.wallUvalueSW());
  EXPECT_DOUBLE_EQ(0.508937055452772, userModel.roofUValue());
  EXPECT_DOUBLE_EQ(0.91461449925898, userModel.wallSolarAbsorptionS());
  EXPECT_DOUBLE_EQ(0.928931093579599, userModel.wallSolarAbsorptionSE());
  EXPECT_DOUBLE_EQ(0.435542934183637, userModel.wallSolarAbsorptionE());
  EXPECT_DOUBLE_EQ(0.793609339380358, userModel.wallSolarAbsorptionNE());
  EXPECT_DOUBLE_EQ(0.902389688647158, userModel.wallSolarAbsorptionN());
  EXPECT_DOUBLE_EQ(0.336318028981842, userModel.wallSolarAbsorptionNW());
  EXPECT_DOUBLE_EQ(0.37153202026125, userModel.wallSolarAbsorptionW());
  EXPECT_DOUBLE_EQ(0.418783890513947, userModel.wallSolarAbsorptionSW());
  EXPECT_DOUBLE_EQ(0.223964378497134, userModel.roofSolarAbsorption());
  EXPECT_DOUBLE_EQ(0.583098358149272, userModel.wallThermalEmissivityS());
  EXPECT_DOUBLE_EQ(0.141381800284656, userModel.wallThermalEmissivitySE());
  EXPECT_DOUBLE_EQ(0.837222292557137, userModel.wallThermalEmissivityE());
  EXPECT_DOUBLE_EQ(0.49538931179426, userModel.wallThermalEmissivityNE());
  EXPECT_DOUBLE_EQ(0.871379477772421, userModel.wallThermalEmissivityN());
  EXPECT_DOUBLE_EQ(0.170422643070764, userModel.wallThermalEmissivityNW());
  EXPECT_DOUBLE_EQ(0.761063022176878, userModel.wallThermalEmissivityW());
  EXPECT_DOUBLE_EQ(0.186495812844654, userModel.wallThermalEmissivitySW());
  EXPECT_DOUBLE_EQ(0.907924653508436, userModel.roofThermalEmissivity());
  EXPECT_DOUBLE_EQ(0.606074602940241, userModel.windowAreaS());
  EXPECT_DOUBLE_EQ(0.404342798081098, userModel.windowAreaSE());
  EXPECT_DOUBLE_EQ(0.0612029472801275, userModel.windowAreaE());
  EXPECT_DOUBLE_EQ(0.289843899154198, userModel.windowAreaNE());
  EXPECT_DOUBLE_EQ(0.540818859803666, userModel.windowAreaN());
  EXPECT_DOUBLE_EQ(0.41253025448177, userModel.windowAreaNW());
  EXPECT_DOUBLE_EQ(0.014956739105872, userModel.windowAreaW());
  EXPECT_DOUBLE_EQ(0.899839246505665, userModel.windowAreaSW());
  EXPECT_DOUBLE_EQ(0.135269594888848, userModel.skylightArea());
  EXPECT_DOUBLE_EQ(0.232560858068808, userModel.windowUvalueS());
  EXPECT_DOUBLE_EQ(0.431164085960324, userModel.windowUvalueSE());
  EXPECT_DOUBLE_EQ(0.00477022329159593, userModel.windowUvalueE());
  EXPECT_DOUBLE_EQ(0.71516207439754, userModel.windowUvalueNE());
  EXPECT_DOUBLE_EQ(0.280649559810701, userModel.windowUvalueN());
  EXPECT_DOUBLE_EQ(0.355908313708148, userModel.windowUvalueNW());
  EXPECT_DOUBLE_EQ(0.112872065367925, userModel.windowUvalueW());
  EXPECT_DOUBLE_EQ(0.398611796542468, userModel.windowUvalueSW());
  EXPECT_DOUBLE_EQ(0.712266965230007, userModel.skylightUvalue());
  EXPECT_DOUBLE_EQ(0.255902968619523, userModel.windowSHGCS());
  EXPECT_DOUBLE_EQ(0.401818741289806, userModel.windowSHGCSE());
  EXPECT_DOUBLE_EQ(0.536223533889905, userModel.windowSHGCE());
  EXPECT_DOUBLE_EQ(0.251096592939623, userModel.windowSHGCNE());
  EXPECT_DOUBLE_EQ(0.931256342309665, userModel.windowSHGCN());
  EXPECT_DOUBLE_EQ(0.896808057579816, userModel.windowSHGCNW());
  EXPECT_DOUBLE_EQ(0.981291583238567, userModel.windowSHGCW());
  EXPECT_DOUBLE_EQ(0.148339469077549, userModel.windowSHGCSW());
  EXPECT_DOUBLE_EQ(0.531228639942613, userModel.skylightSHGC());
  EXPECT_DOUBLE_EQ(0.719753126248692, userModel.windowSCFS());
  EXPECT_DOUBLE_EQ(0.719295130996734, userModel.windowSCFSE());
  EXPECT_DOUBLE_EQ(0.62587251635714, userModel.windowSCFE());
  EXPECT_DOUBLE_EQ(0.789338364373816, userModel.windowSCFNE());
  EXPECT_DOUBLE_EQ(0.620542267432122, userModel.windowSCFN());
  EXPECT_DOUBLE_EQ(0.300503015955268, userModel.windowSCFNW());
  EXPECT_DOUBLE_EQ(0.128976467360588, userModel.windowSCFW());
  EXPECT_DOUBLE_EQ(0.947178709804832, userModel.windowSCFSW());
  EXPECT_DOUBLE_EQ(0.902216926946315, userModel.windowSDFS());
  EXPECT_DOUBLE_EQ(0.632486442302954, userModel.windowSDFSE());
  EXPECT_DOUBLE_EQ(0.719004834647601, userModel.windowSDFE());
  EXPECT_DOUBLE_EQ(0.504956302525102, userModel.windowSDFNE());
  EXPECT_DOUBLE_EQ(0.212427137938556, userModel.windowSDFN());
  EXPECT_DOUBLE_EQ(0.0746662195816253, userModel.windowSDFNW());
  EXPECT_DOUBLE_EQ(0.970579615803331, userModel.windowSDFW());
  EXPECT_DOUBLE_EQ(0.617489329894299, userModel.windowSDFSW());
  EXPECT_DOUBLE_EQ(0.3434343, userModel.skylightSCF());
  EXPECT_DOUBLE_EQ(0.2534335, userModel.skylightSDF());
  EXPECT_DOUBLE_EQ(0.523964673586454, userModel.exteriorHeatCapacity());
  // This is redundant with buildingAirLeakage().
  // EXPECT_DOUBLE_EQ(0.139585598177502, userModel.infiltration());
  EXPECT_DOUBLE_EQ(0.287554068015519, userModel.hvacWasteFactor());
  EXPECT_DOUBLE_EQ(0.801121347575538, userModel.hvacHeatingLossFactor());
  EXPECT_DOUBLE_EQ(0.919509843310335, userModel.hvacCoolingLossFactor());
  EXPECT_DOUBLE_EQ(0.33038965168355, userModel.dhwDistributionEfficiency());
  EXPECT_DOUBLE_EQ(0.625403806654488, userModel.heatingPumpControl());
  EXPECT_DOUBLE_EQ(0.0184589116025784, userModel.coolingPumpControl());
  EXPECT_DOUBLE_EQ(0.976673863929532, userModel.heatGainPerPerson());

  EXPECT_STREQ("./ORD.epw", userModel.weatherFilePath().c_str());

  EXPECT_DOUBLE_EQ(0.1, userModel.ventilationIntakeRateUnoccupied());
  EXPECT_DOUBLE_EQ(0.2, userModel.ventilationExhaustRateUnoccupied());
  EXPECT_DOUBLE_EQ(0.3, userModel.infiltrationRateUnoccupied());
  EXPECT_DOUBLE_EQ(0.4, userModel.lightingPowerFixedOccupied());
  EXPECT_DOUBLE_EQ(0.5, userModel.lightingPowerFixedUnoccupied());
  EXPECT_DOUBLE_EQ(0.6, userModel.electricAppliancePowerFixedOccupied());
  EXPECT_DOUBLE_EQ(0.7, userModel.electricAppliancePowerFixedUnoccupied());
  EXPECT_DOUBLE_EQ(0.8, userModel.gasAppliancePowerFixedOccupied());
  EXPECT_DOUBLE_EQ(0.9, userModel.gasAppliancePowerFixedUnoccupied());

  EXPECT_STREQ("./schedule.txt", userModel.scheduleFilePath().c_str());
}

TEST(IsoModelTests, DefaultsTests)
{
  UserModel userModel;
  userModel.load(test_data_path + "/defaults_test_building.ism", test_data_path + "/defaults_test_defaults.ism");

  // The building and the defaults files both set the terrainClass.
  // The building file should take precedence.
  EXPECT_DOUBLE_EQ(0.9, userModel.terrainClass());

  // The rest of the values are only set in one of the other of the two files.
  EXPECT_DOUBLE_EQ(0.13797878192703, userModel.floorArea());
  EXPECT_DOUBLE_EQ(0.425419263581922, userModel.buildingHeight());
  EXPECT_DOUBLE_EQ(0.665995505182317, userModel.buildingOccupancyFrom());
  EXPECT_DOUBLE_EQ(0.400372234106352, userModel.buildingOccupancyTo());
  EXPECT_DOUBLE_EQ(0.254850243633116, userModel.equivFullLoadOccupancyFrom());
  EXPECT_DOUBLE_EQ(0.713362082549865, userModel.equivFullLoadOccupancyTo());
  EXPECT_DOUBLE_EQ(0.0453028919599623, userModel.peopleDensityOccupied());
  EXPECT_DOUBLE_EQ(0.374398515315959, userModel.peopleDensityUnoccupied());
  EXPECT_DOUBLE_EQ(0.308476836073534, userModel.heatingOccupiedSetpoint());
  EXPECT_DOUBLE_EQ(0.96115521837837, userModel.heatingUnoccupiedSetpoint());
  EXPECT_DOUBLE_EQ(0.0182141291000549, userModel.coolingOccupiedSetpoint());
  EXPECT_DOUBLE_EQ(0.47279017381788, userModel.coolingUnoccupiedSetpoint());
  EXPECT_DOUBLE_EQ(0.0159043563230605, userModel.elecPowerAppliancesOccupied());
  EXPECT_DOUBLE_EQ(0.877197046873451, userModel.elecPowerAppliancesUnoccupied());
  EXPECT_DOUBLE_EQ(0.413231779700794, userModel.gasPowerAppliancesOccupied());
  EXPECT_DOUBLE_EQ(0.735954395099727, userModel.gasPowerAppliancesUnoccupied());
  EXPECT_DOUBLE_EQ(0.827607402688993, userModel.lightingPowerIntensityOccupied());
  EXPECT_DOUBLE_EQ(0.210627783574828, userModel.lightingPowerIntensityUnoccupied());
  EXPECT_DOUBLE_EQ(0.688613300586997, userModel.exteriorLightingPower());
  EXPECT_DOUBLE_EQ(0.952066322499152, userModel.daylightSensorSystem());
  EXPECT_DOUBLE_EQ(0.191200546809349, userModel.lightingOccupancySensorSystem());
  EXPECT_DOUBLE_EQ(0.295905191092175, userModel.constantIlluminationControl());
  EXPECT_DOUBLE_EQ(0.977647331541828, userModel.coolingSystemCOP());
  EXPECT_DOUBLE_EQ(0.86953551426846, userModel.coolingSystemIPLVToCOPRatio());
  EXPECT_DOUBLE_EQ(1, userModel.heatingEnergyCarrier());
  EXPECT_DOUBLE_EQ(0.710454137223511, userModel.heatingSystemEfficiency());
  EXPECT_DOUBLE_EQ(3, userModel.ventilationType());
  EXPECT_DOUBLE_EQ(0.903704085971796, userModel.freshAirFlowRate());
  EXPECT_DOUBLE_EQ(0.724248760195895, userModel.supplyExhaustRate());
  EXPECT_DOUBLE_EQ(0.49985550202677, userModel.heatRecovery());
  EXPECT_DOUBLE_EQ(0.846564029275989, userModel.exhaustAirRecirclation());
  EXPECT_DOUBLE_EQ(0.139585598177502, userModel.buildingAirLeakage());
  EXPECT_DOUBLE_EQ(0.881916031629701, userModel.dhwDemand());
  EXPECT_DOUBLE_EQ(0.105230439331114, userModel.dhwEfficiency());
  // this is no longer set, we should delete the accessors for it
  // and the variable
  // EXPECT_DOUBLE_EQ(0.791092991177229, userModel.dhwDistributionSystem());
  EXPECT_DOUBLE_EQ(2, userModel.dhwEnergyCarrier());
  EXPECT_DOUBLE_EQ(3, userModel.bemType());
  EXPECT_DOUBLE_EQ(0.590020871911987, userModel.interiorHeatCapacity());
  EXPECT_DOUBLE_EQ(0.256509943938684, userModel.specificFanPower());
  EXPECT_DOUBLE_EQ(0.171213718831364, userModel.fanFlowControlFactor());
  // this is no longer set, we should delete the accessors for it
  // and the variable. Probably replaced by skylightSHGC
  // EXPECT_DOUBLE_EQ(0.577629926945883, userModel.roofSHGC());
  EXPECT_DOUBLE_EQ(0.351700449083525, userModel.wallAreaS());
  EXPECT_DOUBLE_EQ(0.638796629077831, userModel.wallAreaSE());
  EXPECT_DOUBLE_EQ(0.713877579934114, userModel.wallAreaE());
  EXPECT_DOUBLE_EQ(0.0544635225207429, userModel.wallAreaNE());
  EXPECT_DOUBLE_EQ(0.713312047950444, userModel.wallAreaN());
  EXPECT_DOUBLE_EQ(0.316883353660591, userModel.wallAreaNW());
  EXPECT_DOUBLE_EQ(0.963602582100428, userModel.wallAreaW());
  EXPECT_DOUBLE_EQ(0.950016805325306, userModel.wallAreaSW());
  EXPECT_DOUBLE_EQ(0.401348851386038, userModel.roofArea());
  EXPECT_DOUBLE_EQ(0.479173557940235, userModel.wallUvalueS());
  EXPECT_DOUBLE_EQ(0.598665235979741, userModel.wallUvalueSE());
  EXPECT_DOUBLE_EQ(0.592537203218594, userModel.wallUvalueE());
  EXPECT_DOUBLE_EQ(0.317076189922438, userModel.wallUvalueNE());
  EXPECT_DOUBLE_EQ(0.857610736439619, userModel.wallUvalueN());
  EXPECT_DOUBLE_EQ(0.494959077705813, userModel.wallUvalueNW());
  EXPECT_DOUBLE_EQ(0.710302412967452, userModel.wallUvalueW());
  EXPECT_DOUBLE_EQ(0.755347362509827, userModel.wallUvalueSW());
  EXPECT_DOUBLE_EQ(0.508937055452772, userModel.roofUValue());
  EXPECT_DOUBLE_EQ(0.91461449925898, userModel.wallSolarAbsorptionS());
  EXPECT_DOUBLE_EQ(0.928931093579599, userModel.wallSolarAbsorptionSE());
  EXPECT_DOUBLE_EQ(0.435542934183637, userModel.wallSolarAbsorptionE());
  EXPECT_DOUBLE_EQ(0.793609339380358, userModel.wallSolarAbsorptionNE());
  EXPECT_DOUBLE_EQ(0.902389688647158, userModel.wallSolarAbsorptionN());
  EXPECT_DOUBLE_EQ(0.336318028981842, userModel.wallSolarAbsorptionNW());
  EXPECT_DOUBLE_EQ(0.37153202026125, userModel.wallSolarAbsorptionW());
  EXPECT_DOUBLE_EQ(0.418783890513947, userModel.wallSolarAbsorptionSW());
  EXPECT_DOUBLE_EQ(0.223964378497134, userModel.roofSolarAbsorption());
  EXPECT_DOUBLE_EQ(0.583098358149272, userModel.wallThermalEmissivityS());
  EXPECT_DOUBLE_EQ(0.141381800284656, userModel.wallThermalEmissivitySE());
  EXPECT_DOUBLE_EQ(0.837222292557137, userModel.wallThermalEmissivityE());
  EXPECT_DOUBLE_EQ(0.49538931179426, userModel.wallThermalEmissivityNE());
  EXPECT_DOUBLE_EQ(0.871379477772421, userModel.wallThermalEmissivityN());
  EXPECT_DOUBLE_EQ(0.170422643070764, userModel.wallThermalEmissivityNW());
  EXPECT_DOUBLE_EQ(0.761063022176878, userModel.wallThermalEmissivityW());
  EXPECT_DOUBLE_EQ(0.186495812844654, userModel.wallThermalEmissivitySW());
  EXPECT_DOUBLE_EQ(0.907924653508436, userModel.roofThermalEmissivity());
  EXPECT_DOUBLE_EQ(0.606074602940241, userModel.windowAreaS());
  EXPECT_DOUBLE_EQ(0.404342798081098, userModel.windowAreaSE());
  EXPECT_DOUBLE_EQ(0.0612029472801275, userModel.windowAreaE());
  EXPECT_DOUBLE_EQ(0.289843899154198, userModel.windowAreaNE());
  EXPECT_DOUBLE_EQ(0.540818859803666, userModel.windowAreaN());
  EXPECT_DOUBLE_EQ(0.41253025448177, userModel.windowAreaNW());
  EXPECT_DOUBLE_EQ(0.014956739105872, userModel.windowAreaW());
  EXPECT_DOUBLE_EQ(0.899839246505665, userModel.windowAreaSW());
  EXPECT_DOUBLE_EQ(0.135269594888848, userModel.skylightArea());
  EXPECT_DOUBLE_EQ(0.232560858068808, userModel.windowUvalueS());
  EXPECT_DOUBLE_EQ(0.431164085960324, userModel.windowUvalueSE());
  EXPECT_DOUBLE_EQ(0.00477022329159593, userModel.windowUvalueE());
  EXPECT_DOUBLE_EQ(0.71516207439754, userModel.windowUvalueNE());
  EXPECT_DOUBLE_EQ(0.280649559810701, userModel.windowUvalueN());
  EXPECT_DOUBLE_EQ(0.355908313708148, userModel.windowUvalueNW());
  EXPECT_DOUBLE_EQ(0.112872065367925, userModel.windowUvalueW());
  EXPECT_DOUBLE_EQ(0.398611796542468, userModel.windowUvalueSW());
  EXPECT_DOUBLE_EQ(0.712266965230007, userModel.skylightUvalue());
  EXPECT_DOUBLE_EQ(0.255902968619523, userModel.windowSHGCS());
  EXPECT_DOUBLE_EQ(0.401818741289806, userModel.windowSHGCSE());
  EXPECT_DOUBLE_EQ(0.536223533889905, userModel.windowSHGCE());
  EXPECT_DOUBLE_EQ(0.251096592939623, userModel.windowSHGCNE());
  EXPECT_DOUBLE_EQ(0.931256342309665, userModel.windowSHGCN());
  EXPECT_DOUBLE_EQ(0.896808057579816, userModel.windowSHGCNW());
  EXPECT_DOUBLE_EQ(0.981291583238567, userModel.windowSHGCW());
  EXPECT_DOUBLE_EQ(0.148339469077549, userModel.windowSHGCSW());
  EXPECT_DOUBLE_EQ(0.531228639942613, userModel.skylightSHGC());
  EXPECT_DOUBLE_EQ(0.719753126248692, userModel.windowSCFS());
  EXPECT_DOUBLE_EQ(0.719295130996734, userModel.windowSCFSE());
  EXPECT_DOUBLE_EQ(0.62587251635714, userModel.windowSCFE());
  EXPECT_DOUBLE_EQ(0.789338364373816, userModel.windowSCFNE());
  EXPECT_DOUBLE_EQ(0.620542267432122, userModel.windowSCFN());
  EXPECT_DOUBLE_EQ(0.300503015955268, userModel.windowSCFNW());
  EXPECT_DOUBLE_EQ(0.128976467360588, userModel.windowSCFW());
  EXPECT_DOUBLE_EQ(0.947178709804832, userModel.windowSCFSW());
  EXPECT_DOUBLE_EQ(0.902216926946315, userModel.windowSDFS());
  EXPECT_DOUBLE_EQ(0.632486442302954, userModel.windowSDFSE());
  EXPECT_DOUBLE_EQ(0.719004834647601, userModel.windowSDFE());
  EXPECT_DOUBLE_EQ(0.504956302525102, userModel.windowSDFNE());
  EXPECT_DOUBLE_EQ(0.212427137938556, userModel.windowSDFN());
  EXPECT_DOUBLE_EQ(0.0746662195816253, userModel.windowSDFNW());
  EXPECT_DOUBLE_EQ(0.970579615803331, userModel.windowSDFW());
  EXPECT_DOUBLE_EQ(0.617489329894299, userModel.windowSDFSW());
  EXPECT_DOUBLE_EQ(0.3434343, userModel.skylightSCF());
  EXPECT_DOUBLE_EQ(0.2534335, userModel.skylightSDF());
  EXPECT_DOUBLE_EQ(0.523964673586454, userModel.exteriorHeatCapacity());
  // This is redundant with buildingAirLeakage().
  // EXPECT_DOUBLE_EQ(0.139585598177502, userModel.infiltration());
  EXPECT_DOUBLE_EQ(0.287554068015519, userModel.hvacWasteFactor());
  EXPECT_DOUBLE_EQ(0.801121347575538, userModel.hvacHeatingLossFactor());
  EXPECT_DOUBLE_EQ(0.919509843310335, userModel.hvacCoolingLossFactor());
  EXPECT_DOUBLE_EQ(0.33038965168355, userModel.dhwDistributionEfficiency());
  EXPECT_DOUBLE_EQ(0.625403806654488, userModel.heatingPumpControl());
  EXPECT_DOUBLE_EQ(0.0184589116025784, userModel.coolingPumpControl());
  EXPECT_DOUBLE_EQ(0.976673863929532, userModel.heatGainPerPerson());

  EXPECT_STREQ("./ORD.epw", userModel.weatherFilePath().c_str());

  EXPECT_DOUBLE_EQ(0.1, userModel.ventilationIntakeRateUnoccupied());
  EXPECT_DOUBLE_EQ(0.2, userModel.ventilationExhaustRateUnoccupied());
  EXPECT_DOUBLE_EQ(0.3, userModel.infiltrationRateUnoccupied());
  EXPECT_DOUBLE_EQ(0.4, userModel.lightingPowerFixedOccupied());
  EXPECT_DOUBLE_EQ(0.5, userModel.lightingPowerFixedUnoccupied());
  EXPECT_DOUBLE_EQ(0.6, userModel.electricAppliancePowerFixedOccupied());
  EXPECT_DOUBLE_EQ(0.7, userModel.electricAppliancePowerFixedUnoccupied());
  EXPECT_DOUBLE_EQ(0.8, userModel.gasAppliancePowerFixedOccupied());
  EXPECT_DOUBLE_EQ(0.9, userModel.gasAppliancePowerFixedUnoccupied());

  EXPECT_STREQ("./schedule.txt", userModel.scheduleFilePath().c_str());
}

TEST(IsoModelTests, OptionalPropertiesDefaultsTests) {
  UserModel userModel;
  userModel.load(test_data_path + "/ism_props_for_testing_umodel_init_v2.ism");

  // Expect to find the default values hardcoded into the various Building, Cooling, etc. classes.
  EXPECT_DOUBLE_EQ(0.0, userModel.externalEquipment());
  EXPECT_TRUE(userModel.forcedAirHeating());
  EXPECT_DOUBLE_EQ(7.0, userModel.dT_supp_ht());
  EXPECT_DOUBLE_EQ(0.25, userModel.E_pumps_cl());
  EXPECT_DOUBLE_EQ(1, userModel.T_ht_ctrl_flag());
  EXPECT_DOUBLE_EQ(1, userModel.a_H0());
  EXPECT_DOUBLE_EQ(15, userModel.tau_H0());
  EXPECT_DOUBLE_EQ(0, userModel.DH_YesNo());
  EXPECT_DOUBLE_EQ(0.9, userModel.eta_DH_network());
  EXPECT_DOUBLE_EQ(0.87, userModel.eta_DH_sys());
  EXPECT_DOUBLE_EQ(0.000, userModel.frac_DH_free());
  EXPECT_DOUBLE_EQ(60, userModel.dhw_tset());
  EXPECT_DOUBLE_EQ(20, userModel.dhw_tsupply());
  EXPECT_TRUE(userModel.forcedAirCooling());
  EXPECT_DOUBLE_EQ(1, userModel.T_cl_ctrl_flag());
  EXPECT_DOUBLE_EQ(7.0, userModel.dT_supp_cl());
  EXPECT_DOUBLE_EQ(0, userModel.DC_YesNo());
  EXPECT_DOUBLE_EQ(0.9, userModel.eta_DC_network());
  EXPECT_DOUBLE_EQ(5.5, userModel.eta_DC_COP());
  EXPECT_DOUBLE_EQ(0, userModel.eta_DC_frac_abs());
  EXPECT_DOUBLE_EQ(1, userModel.eta_DC_COP_abs());
  EXPECT_DOUBLE_EQ(0, userModel.frac_DC_free());
  EXPECT_DOUBLE_EQ(0.25, userModel.E_pumps_cl());
  EXPECT_DOUBLE_EQ(7.0, userModel.n_day_start());
  EXPECT_DOUBLE_EQ(18.0, userModel.n_day_end());
  EXPECT_DOUBLE_EQ(50.0, userModel.n_weeks());
  EXPECT_DOUBLE_EQ(1.0, userModel.elecInternalGains());
  EXPECT_DOUBLE_EQ(0.0, userModel.permLightPowerDensity());
  EXPECT_DOUBLE_EQ(0.6, userModel.presenceSensorAd());
  EXPECT_DOUBLE_EQ(0.8, userModel.automaticAd());
  EXPECT_DOUBLE_EQ(0.6, userModel.presenceAutoAd());
  EXPECT_DOUBLE_EQ(1, userModel.manualSwitchAd());
  EXPECT_DOUBLE_EQ(500.0, userModel.presenceSensorLux());
  EXPECT_DOUBLE_EQ(300.0, userModel.automaticLux());
  EXPECT_DOUBLE_EQ(300.0, userModel.presenceAutoLux());
  EXPECT_DOUBLE_EQ(500.0, userModel.manualSwitchLux());
  EXPECT_DOUBLE_EQ(0.0, userModel.naturallyLightedArea());
  EXPECT_DOUBLE_EQ(1.22521 * 0.001012, userModel.rhoCpAir());
  EXPECT_DOUBLE_EQ(4.1813, userModel.rhoCpWater());
  EXPECT_DOUBLE_EQ(0.04, userModel.R_se());
  EXPECT_DOUBLE_EQ(500, userModel.irradianceForMaxShadingUse());
  EXPECT_DOUBLE_EQ(0.5, userModel.shadingFactorAtMaxUse());
  EXPECT_DOUBLE_EQ(4.5, userModel.totalAreaPerFloorArea());
  EXPECT_DOUBLE_EQ(0.25, userModel.win_ff());
  EXPECT_DOUBLE_EQ(0.9, userModel.win_F_W());
  EXPECT_DOUBLE_EQ(0.04, userModel.R_sc_ext());
  EXPECT_DOUBLE_EQ(0.04, userModel.R_se());
  EXPECT_DOUBLE_EQ(500, userModel.irradianceForMaxShadingUse());
  EXPECT_DOUBLE_EQ(0.5, userModel.shadingFactorAtMaxUse());
  EXPECT_DOUBLE_EQ(4.5, userModel.totalAreaPerFloorArea());
  EXPECT_DOUBLE_EQ(0.25, userModel.win_ff());
  EXPECT_DOUBLE_EQ(0.9, userModel.win_F_W());
  EXPECT_DOUBLE_EQ(0.04, userModel.R_sc_ext());
  EXPECT_DOUBLE_EQ(-50.0, userModel.ventPreheatDegC());
  EXPECT_DOUBLE_EQ(2.0, userModel.n50());
  EXPECT_DOUBLE_EQ(39.0, userModel.hzone());
  EXPECT_DOUBLE_EQ(0.65, userModel.p_exp());
  EXPECT_DOUBLE_EQ(0.7, userModel.zone_frac());
  EXPECT_DOUBLE_EQ(0.667, userModel.stack_exp());
  EXPECT_DOUBLE_EQ(0.0146, userModel.stack_coeff());
  EXPECT_DOUBLE_EQ(0.667, userModel.wind_exp());
  EXPECT_DOUBLE_EQ(0.0769, userModel.wind_coeff());
  EXPECT_DOUBLE_EQ(0.75, userModel.dCp());
  EXPECT_EQ(1, userModel.vent_rate_flag());
  EXPECT_DOUBLE_EQ(0.0, userModel.H_ve());
}

TEST(IsoModelTests, OptionalPropertiesOverrideTests) {
  UserModel userModel;
  userModel.load(test_data_path + "/ism_props_for_testing_umodel_init_v2.ism", test_data_path + "/optional_defaults_override.ism");

  // Expect to find the value set in optional_defaults_override.ism.
  EXPECT_DOUBLE_EQ(1.0, userModel.externalEquipment());
  EXPECT_FALSE(userModel.forcedAirHeating());
  EXPECT_DOUBLE_EQ(8.0, userModel.dT_supp_ht());
  EXPECT_DOUBLE_EQ(1.25, userModel.E_pumps_cl());
  EXPECT_DOUBLE_EQ(2.0, userModel.T_ht_ctrl_flag());
  EXPECT_DOUBLE_EQ(2.0, userModel.a_H0());
  EXPECT_DOUBLE_EQ(16.0, userModel.tau_H0());
  EXPECT_DOUBLE_EQ(1.0, userModel.DH_YesNo());
  EXPECT_DOUBLE_EQ(1.9, userModel.eta_DH_network());
  EXPECT_DOUBLE_EQ(1.87, userModel.eta_DH_sys());
  EXPECT_DOUBLE_EQ(1.0, userModel.frac_DH_free());
  EXPECT_DOUBLE_EQ(61.0, userModel.dhw_tset());
  EXPECT_DOUBLE_EQ(21.0, userModel.dhw_tsupply());
  EXPECT_FALSE(userModel.forcedAirCooling());
  EXPECT_DOUBLE_EQ(2.0, userModel.T_cl_ctrl_flag());
  EXPECT_DOUBLE_EQ(8.0, userModel.dT_supp_cl());
  EXPECT_DOUBLE_EQ(1.0, userModel.DC_YesNo());
  EXPECT_DOUBLE_EQ(1.9, userModel.eta_DC_network());
  EXPECT_DOUBLE_EQ(6.5, userModel.eta_DC_COP());
  EXPECT_DOUBLE_EQ(1.0, userModel.eta_DC_frac_abs());
  EXPECT_DOUBLE_EQ(2.0, userModel.eta_DC_COP_abs());
  EXPECT_DOUBLE_EQ(1.0, userModel.frac_DC_free());
  EXPECT_DOUBLE_EQ(1.25, userModel.E_pumps_cl());
  EXPECT_DOUBLE_EQ(8.0, userModel.n_day_start());
  EXPECT_DOUBLE_EQ(19.0, userModel.n_day_end());
  EXPECT_DOUBLE_EQ(51.0, userModel.n_weeks());
  EXPECT_DOUBLE_EQ(2.0, userModel.elecInternalGains());
  EXPECT_DOUBLE_EQ(1.0, userModel.permLightPowerDensity());
  EXPECT_DOUBLE_EQ(1.6, userModel.presenceSensorAd());
  EXPECT_DOUBLE_EQ(1.8, userModel.automaticAd());
  EXPECT_DOUBLE_EQ(1.6, userModel.presenceAutoAd());
  EXPECT_DOUBLE_EQ(2.0, userModel.manualSwitchAd());
  EXPECT_DOUBLE_EQ(501.0, userModel.presenceSensorLux());
  EXPECT_DOUBLE_EQ(301.0, userModel.automaticLux());
  EXPECT_DOUBLE_EQ(301.0, userModel.presenceAutoLux());
  EXPECT_DOUBLE_EQ(501.0, userModel.manualSwitchLux());
  EXPECT_DOUBLE_EQ(1.0, userModel.naturallyLightedArea());
  EXPECT_DOUBLE_EQ(2.2252099999999997, userModel.rhoCpAir());
  EXPECT_DOUBLE_EQ(5.1813, userModel.rhoCpWater());
  EXPECT_DOUBLE_EQ(1.04, userModel.R_se());
  EXPECT_DOUBLE_EQ(501.0, userModel.irradianceForMaxShadingUse());
  EXPECT_DOUBLE_EQ(1.5, userModel.shadingFactorAtMaxUse());
  EXPECT_DOUBLE_EQ(5.5, userModel.totalAreaPerFloorArea());
  EXPECT_DOUBLE_EQ(1.25, userModel.win_ff());
  EXPECT_DOUBLE_EQ(1.9, userModel.win_F_W());
  EXPECT_DOUBLE_EQ(1.04, userModel.R_sc_ext());
  EXPECT_DOUBLE_EQ(1.04, userModel.R_se());
  EXPECT_DOUBLE_EQ(501.0, userModel.irradianceForMaxShadingUse());
  EXPECT_DOUBLE_EQ(1.5, userModel.shadingFactorAtMaxUse());
  EXPECT_DOUBLE_EQ(5.5, userModel.totalAreaPerFloorArea());
  EXPECT_DOUBLE_EQ(1.25, userModel.win_ff());
  EXPECT_DOUBLE_EQ(1.9, userModel.win_F_W());
  EXPECT_DOUBLE_EQ(1.04, userModel.R_sc_ext());
  EXPECT_DOUBLE_EQ(-49.0, userModel.ventPreheatDegC());
  EXPECT_DOUBLE_EQ(3.0, userModel.n50());
  EXPECT_DOUBLE_EQ(40.0, userModel.hzone());
  EXPECT_DOUBLE_EQ(1.65, userModel.p_exp());
  EXPECT_DOUBLE_EQ(1.7, userModel.zone_frac());
  EXPECT_DOUBLE_EQ(1.667, userModel.stack_exp());
  EXPECT_DOUBLE_EQ(1.0146, userModel.stack_coeff());
  EXPECT_DOUBLE_EQ(1.667, userModel.wind_exp());
  EXPECT_DOUBLE_EQ(1.0769, userModel.wind_coeff());
  EXPECT_DOUBLE_EQ(1.75, userModel.dCp());
  EXPECT_EQ(2, userModel.vent_rate_flag());
  EXPECT_DOUBLE_EQ(1.0, userModel.H_ve());
}
