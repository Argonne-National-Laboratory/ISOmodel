/*
 * ISOModel_GTest.cpp
 *
 *  Created on: Dec 5, 2014
 *      Author: nick
 */

#include "gtest/gtest.h"

#include "../Properties.hpp"
#include "../UserModel.hpp"

using namespace openstudio::isomodel;

std::string test_data_path;
/*
 * weatherFilePath = ORD.epw
 # Terrain class urban/city = 0.8, suburban/some shielding = 0.9, country/open = 1.0
 terrainClass = 0.8
 # building height is in m,  floor area is in m^2, people density is m2/person
 buildingHeight = 6.33
 occupancyHourEnd = 18
 wallU = 2.1, 234.3, 12.3
 */

TEST(PropertiesTests, KeyValueTests)
{
  Properties props(test_data_path + "/test_properties.props");
  ASSERT_EQ(5, props.size());

  ASSERT_EQ("ORD.epw", props.getProperty("weatherFilePath"));
  ASSERT_EQ(0.8, props.getPropertyAsDouble("terrainClass"));
  ASSERT_EQ(6.33, props.getPropertyAsDouble("buildingHeight"));
  ASSERT_EQ(17.0, props.getPropertyAsDouble("occupancyHourLast"));
  ASSERT_STREQ("2.1, 234.3, 12.3", props.getProperty("wallU").c_str());

  props.putProperty("a string", "some string");
  ASSERT_STREQ("some string", props.getProperty("a string").c_str());
  props.putProperty("some double", 3.14);
  ASSERT_EQ(3.14, props.getPropertyAsDouble("some double"));

  // test case insensitivity
  ASSERT_EQ(6.33, props.getPropertyAsDouble("BUILDINGHEIGHT"));

  std::vector<double> vec;
  props.getPropertyAsDoubleVector("wallU", vec);
  ASSERT_EQ(3, vec.size());
  ASSERT_EQ(2.1, vec[0]);
  ASSERT_EQ(234.3, vec[1]);
  ASSERT_EQ(12.3, vec[2]);
}

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
  EXPECT_DOUBLE_EQ(0.139585598177502, userModel.infiltration());
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

// Monthly simulation results.
/*
 * Month,ElecHeat,ElecCool,ElecIntLights,ElecExtLights,ElecFans,ElecPump,ElecEquipInt,ElecEquipExt,ElectDHW,GasHeat,GasCool,GasEquip,GasDHW
 1, 0, 0.0371925, 2.88034, 0.257822, 7.3185, 0.799035, 2.24457, 0, 0, 41.5822, 0, 0, 0
 2, 0, 0.0694192, 2.6016, 0.199604, 5.89737, 0.643876, 2.02735, 0, 0, 33.43, 0, 0, 0
 3, 0, 0.196458, 2.88034, 0.184159, 4.56589, 0.498505, 2.24457, 0, 0, 25.6013, 0, 0, 0
 4, 0, 0.46986, 2.78743, 0.178218, 2.6064, 0.284567, 2.17216, 0, 0, 13.9098, 0, 0, 0
 5, 0, 1.41522, 2.88034, 0.147327, 1.42405, 0.155478, 2.24457, 0, 0, 5.31852, 0, 0, 0
 6, 0, 2.65012, 2.78743, 0.142575, 1.00172, 0.109368, 2.17216, 0, 0, 0.482898, 0, 0, 0
 7, 0, 3.9228, 2.88034, 0.147327, 1.3572, 0.148179, 2.24457, 0, 0, -3.61702e-17, 0, 0, 0
 8, 0, 2.2641, 2.88034, 0.147327, 0.838663, 0.0915654, 2.24457, 0, 0, 0.31496, 0, 0, 0
 9, 0, 1.03261, 2.78743, 0.178218, 0.897044, 0.0979394, 2.17216, 0, 0, 3.07235, 0, 0, 0
 10, 0, 0.269696, 2.88034, 0.202575, 2.63422, 0.287605, 2.24457, 0, 0, 14.4624, 0, 0, 0
 11, 0, 0.0614263, 2.78743, 0.231684, 4.55298, 0.497095, 2.17216, 0, 0, 25.7937, 0, 0, 0
 12, 0, 0.030399, 2.88034, 0.257822, 7.02313, 0.766787, 2.24457, 0, 0, 39.9145, 0, 0, 0
 *
 */
TEST(IsoModelTests, SimModelTests)
{
  // the expected values are the results of running the "prior to updated parameter names
  // and parsing" version and copying the results as they were printed out to stdout.
  // Consequently these are not "exact" and so we use EXPECT_NEAR with 0.001 to test.
//  double expected[12][14] =
//  {
//  { 0.0, 0.0371925, 2.88034, 0.257822, 7.3185, 0.799035, 2.24457, 0.0, 0.0, 41.5822, 0.0, 0.0, 0.0 },
//  { 0, 0.0694192, 2.6016, 0.199604, 5.89737, 0.643876, 2.02735, 0, 0, 33.43, 0, 0, 0 },
//  { 0, 0.196458, 2.88034, 0.184159, 4.56589, 0.498505, 2.24457, 0, 0, 25.6013, 0, 0, 0 },
//  { 0, 0.46986, 2.78743, 0.178218, 2.6064, 0.284567, 2.17216, 0, 0, 13.9098, 0, 0, 0 },
//  { 0, 1.41522, 2.88034, 0.147327, 1.42405, 0.155478, 2.24457, 0, 0, 5.31852, 0, 0, 0 },
//  { 0, 2.65012, 2.78743, 0.142575, 1.00172, 0.109368, 2.17216, 0, 0, 0.482898, 0, 0, 0 },
//  { 0, 3.9228, 2.88034, 0.147327, 1.3572, 0.148179, 2.24457, 0, 0, -3.61702e-17, 0, 0, 0 },
//  { 0, 2.2641, 2.88034, 0.147327, 0.838663, 0.0915654, 2.24457, 0, 0, 0.31496, 0, 0, 0 },
//  { 0, 1.03261, 2.78743, 0.178218, 0.897044, 0.0979394, 2.17216, 0, 0, 3.07235, 0, 0, 0 },
//  { 0, 0.269696, 2.88034, 0.202575, 2.63422, 0.287605, 2.24457, 0, 0, 14.4624, 0, 0, 0 },
//  { 0, 0.0614263, 2.78743, 0.231684, 4.55298, 0.497095, 2.17216, 0, 0, 25.7937, 0, 0, 0 },
//  { 0, 0.030399, 2.88034, 0.257822, 7.02313, 0.766787, 2.24457, 0, 0, 39.9145, 0, 0, 0 } };

  // These updated expected results are copied from running the model after an update that changes the results.
  // They are simply to alert us to changes to the code that affect the results.
  double expected[12][13] =
  {
    { 0, 0.02322090407, 2.72099861, 0.2578224281, 7.455673413, 0.8079968472, 2.244567869, 0, 0, 42.39053342, 0, 0, 0 },
    { 0, 0.04541606817, 2.457676164, 0.1996044604, 6.033522328, 0.6538734664, 2.027351624, 0, 0, 34.25221507, 0, 0, 0 },
    { 0, 0.1365954214, 2.72099861, 0.1841588772, 4.708232811, 0.5102473052, 2.244567869, 0, 0, 26.52936786, 0, 0, 0 },
    { 0, 0.3433933418, 2.633224461, 0.1782182683, 2.703241157, 0.2929594969, 2.172162454, 0, 0, 14.71010618, 0, 0, 0 },
    { 0, 1.087281902, 2.72099861, 0.1473271018, 1.407233237, 0.1525066826, 2.244567869, 0, 0, 5.868584419, 0, 0, 0 },
    { 0, 2.135244848, 2.633224461, 0.1425746146, 0.8386206276, 0.09088418786, 2.172162454, 0, 0, 0.5684622011, 0, 0, 0 },
    { 0, 3.291368244, 2.72099861, 0.1473271018, 1.13873967, 0.1234091157, 2.244567869, 0, 0, 0, 0, 0, 0 },
    { 0, 1.801426142, 2.72099861, 0.1473271018, 0.689240287, 0.07469532905, 2.244567869, 0, 0, 0.3755864828, 0, 0, 0 },
    { 0, 0.7595245698, 2.633224461, 0.1782182683, 0.8736611279, 0.09468164682, 2.172162454, 0, 0, 3.477026665, 0, 0, 0 },
    { 0, 0.1812146973, 2.72099861, 0.2025747649, 2.751159975, 0.2981526232, 2.244567869, 0, 0, 15.30221924, 0, 0, 0 },
    { 0, 0.04064513228, 2.633224461, 0.2316837487, 4.663755643, 0.5054271622, 2.172162454, 0, 0, 26.46516132, 0, 0, 0 },
    { 0, 0.01860179166, 2.72099861, 0.2578224281, 7.152733878, 0.7751662529, 2.244567869, 0, 0, 40.67535591, 0, 0, 0 }
  };


  openstudio::isomodel::UserModel userModel;
  userModel.load(test_data_path + "/SmallOffice_v2.ism");
  SimModel simModel = userModel.toSimModel();
  ISOResults results = simModel.simulate();

  for (int i = 0; i < 12; ++i) {
    for (int j = 0; j < 13; ++j) {
      EXPECT_NEAR(expected[i][j], results.monthlyResults[i].getEndUse(j), 0.001) << "i = " << i << ", j = " << j << std::endl;
    }
  }
}

/*
Hourly results by month:
ElecHeat,ElecCool,ElecIntLights,ElecExtLights,ElecFans,ElecPump,ElecEquipInt,ElecEquipExt,ElectDHW,GasHeat,GasCool,GasEquip,GasDHW
0, 0, 0.2358, 0.441981, 0, 0, 1.07829, 42.6174, 0, 39.2839, 0, 0, 0
0, 0, 0.21298, 0.399209, 0, 0, 0.973936, 38.4932, 0, 31.2383, 0, 0, 0
0, 0.0113888, 0.2358, 0.441981, 0, 0, 1.07829, 42.6174, 0, 23.079, 0, 0, 0
0, 0.815593, 0.228193, 0.427724, 0, 0, 1.0435, 41.2427, 0, 14.3389, 0, 0, 0
0, 1.37548, 0.2358, 0.441981, 0, 0, 1.07829, 42.6174, 0, 5.98838, 0, 0, 0
0, 2.76303, 0.228193, 0.427724, 0, 0, 1.0435, 41.2427, 0, 1.5327, 0, 0, 0
0, 4.07107, 0.2358, 0.441981, 0, 0, 1.07829, 42.6174, 0, 0.366813, 0, 0, 0
0, 2.45841, 0.2358, 0.441981, 0, 0, 1.07829, 42.6174, 0, 0.87312, 0, 0, 0
0, 1.47979, 0.228193, 0.427724, 0, 0, 1.0435, 41.2427, 0, 3.47906, 0, 0, 0
0, 0.222445, 0.2358, 0.441981, 0, 0, 1.07829, 42.6174, 0, 11.9559, 0, 0, 0
0, 0.00862051, 0.228193, 0.427724, 0, 0, 1.0435, 41.2427, 0, 23.4188, 0, 0, 0
0, 0, 0.2358, 0.441981, 0, 0, 1.07829, 42.6174, 0, 37.8816, 0, 0, 0
*/
TEST(IsoModelTests, ISOHourlyTests)
{
  // the expected values are the results of running the "prior to updated parameter names
  // and parsing" version and copying the results as they were printed out to stdout.
  // Consequently these are not "exact" and so we use EXPECT_NEAR with 0.001 to test.
  
  // These updated expected results are copied from running the model after an update that changes the results.
  // They are simply to alert us to changes to the code that affect the results.
  double expected[12][13] =
  {
    { 0, 0, 1.136909021, 0.2578224281, 3.494650574, 0.186, 1.715983053, 0, 0, 42.37792216, 0, 0, 0 },
    { 0, 0, 1.032447246, 0.2073272521, 2.944433842, 0.168, 1.553660331, 0, 0, 33.85806861, 0, 0, 0 },
    { 0, 0, 1.180012149, 0.2108916174, 2.503422164, 0.17775, 1.744969253, 0, 0, 24.91103764, 0, 0, 0 },
    { 0, 0.7264510575, 1.044617545, 0.1984163387, 1.802511372, 0.15325, 1.623227212, 0, 0, 15.28758925, 0, 0, 0 },
    { 0, 1.406508208, 1.180012144, 0.1592083196, 1.171793561, 0.1425, 1.744969253, 0, 0, 6.457480891, 0, 0, 0 },
    { 0, 2.940503636, 1.130823842, 0.1425746146, 0.9495458427, 0.12425, 1.681199612, 0, 0, 1.751973646, 0, 0, 0 },
    { 0, 4.262592777, 1.093805842, 0.1485152235, 1.030327619, 0.1195, 1.686996852, 0, 0, 0.4161149059, 0, 0, 0 },
    { 0, 2.450234564, 1.180012146, 0.1782182683, 0.7817989486, 0.11275, 1.744969253, 0, 0, 1.053602899, 0, 0, 0 },
    { 0, 1.132566183, 1.087720695, 0.1895054252, 0.8353541063, 0.12325, 1.652213412, 0, 0, 3.843512537, 0, 0, 0 },
    { 0, 0.04307675072, 1.136909008, 0.2269312616, 1.570938269, 0.16325, 1.715983053, 0, 0, 13.37730505, 0, 0, 0 },
    { 0, 0, 1.130823868, 0.2364362359, 2.42230142, 0.17375, 1.681199612, 0, 0, 25.21628218, 0, 0, 0 },
    { 0, 0, 1.093805869, 0.258416489, 3.45021337, 0.186, 1.686996852, 0, 0, 40.92043021, 0, 0, 0 }
  };

  openstudio::isomodel::UserModel userModel;
  userModel.load(test_data_path + "/SmallOffice_v2.ism");
  ISOHourly hourlyModel = userModel.toHourlyModel();
  ISOResults results = hourlyModel.calculateHourly(true); // aggregateByMonth = true

  for (int i = 0; i < 12; ++i) {
    for (int j = 0; j < 13; ++j) {
      EXPECT_NEAR(expected[i][j], results.hourlyResults[i].getEndUse(j), 0.001) << "i = " << i << ", j = " << j << std::endl;
    }
  }
}

TEST(TimeFrameTests, MonthLengthTest) {
  openstudio::isomodel::TimeFrame frame;
  std::vector<int> monthLengths = {
    31, // January
    28, // February
    31, // March
    30, // April
    31, // May
    30, // June
    31, // July
    31, // August
    30, // September
    31, // October
    30, // November
    31 // December
  };

  for (auto i = 1; i != 13; ++i) {
    EXPECT_EQ(monthLengths[i - 1], frame.monthLength(i)) << "When i = " << i;
  }
}

TEST(TimeFrameTests, HourTests) {
  openstudio::isomodel::TimeFrame frame;
  EXPECT_EQ(0, frame.Hour[0]);
  EXPECT_EQ(23, frame.Hour[23]);
  EXPECT_EQ(0, frame.Hour[24]);
  EXPECT_EQ(23, frame.Hour[8759]);
}

TEST(TimeFrameTests, DayOfMonthTests) {
  openstudio::isomodel::TimeFrame frame;
  EXPECT_EQ(1, frame.DayOfMonth[0]);
  EXPECT_EQ(1, frame.DayOfMonth[23]);
  EXPECT_EQ(2, frame.DayOfMonth[24]);
  EXPECT_EQ(31, frame.DayOfMonth[8759]);
}

TEST(TimeFrameTests, DayOfWeekTests) {
  openstudio::isomodel::TimeFrame frame;
  EXPECT_EQ(0, frame.DayOfWeek[0]);
  EXPECT_EQ(0, frame.DayOfWeek[23]);
  EXPECT_EQ(1, frame.DayOfWeek[24]);
  EXPECT_EQ(6, frame.DayOfWeek[167]);
  EXPECT_EQ(0, frame.DayOfWeek[168]);
}

TEST(TimeFrameTests, MonthTests) {
  openstudio::isomodel::TimeFrame frame;
  EXPECT_EQ(1, frame.Month[0]);
  EXPECT_EQ(1, frame.Month[743]);
  EXPECT_EQ(2, frame.Month[744]);
  EXPECT_EQ(12, frame.Month[8759]);
}

TEST(TimeFrameTests, YTDTests) {
  openstudio::isomodel::TimeFrame frame;
  EXPECT_EQ(0, frame.YTD[0]);
  EXPECT_EQ(30, frame.YTD[743]);
  EXPECT_EQ(31, frame.YTD[744]);
  EXPECT_EQ(364, frame.YTD[8759]);
}

// Solar tests. Expected results from http://www.usc.edu/dept-00/dept/architecture/mbs/tools/thermal/sun_calc.html
// Inputs:
// 41.98 N, 87.92 W (matches ORD.EPW)
// Jan 1, 2009, 12:00 (noon).
// GMT -6, Daylight savings: No.
// Outputs:
// Declination: -22.96 deg
// Altitude Angle: 25.05 deg
// Azimuth Angle: -1.18 deg
// Local Solar Time: 12:04
// Hour Angle: -1.16
// Equation of Time: -3.67

TEST(SolarTests, SunPositionTests) {
  openstudio::isomodel::UserModel userModel;
  userModel.load(test_data_path + "/SmallOffice_v2.ism");
  userModel.loadWeather();

  TimeFrame frame;
  SolarRadiation solarRadiation(&frame, userModel.epwData().get());

  int hourOfYear = 12;
  std::cout << "frame.YTD[hourOfYear] = " << frame.YTD[hourOfYear] << std::endl;

  auto revolution = solarRadiation.calculateRevolutionAngle(frame.YTD[hourOfYear]);
  std::cout << "revolution = " << revolution << std::endl;

  auto equationOfTime = solarRadiation.calculateEquationOfTime(revolution);
  EXPECT_NEAR(-3.67, equationOfTime, 0.01);

  auto apparentSolarTime = solarRadiation.calculateApparentSolarTime(frame.Hour[hourOfYear], equationOfTime);

  auto solarDeclination = solarRadiation.calculateSolarDeclination(revolution);
  auto solarHourAngle = solarRadiation.calculateSolarHourAngle(apparentSolarTime);
  auto solarAltitudeAngle = solarRadiation.calculateSolarAltitudeAngle(solarDeclination, solarHourAngle);

  auto solarAzimuthSin = solarRadiation.calculateSolarAzimuthSin(solarDeclination, solarHourAngle, solarAltitudeAngle);
  auto solarAzimuthCos = solarRadiation.calculateSolarAzimuthCos(solarDeclination, solarHourAngle, solarAltitudeAngle);
  auto solarAzimuth = solarRadiation.calculateSolarAzimuth(solarAzimuthSin, solarAzimuthCos);
}

int main(int argc, char** argv)
{
  if (argc < 2) {
    std::cout << "Usage: isomodel_unit_tests test_data_directory" << std::endl;
    return 0;
  } else {
    ::testing::InitGoogleTest(&argc, argv);
    test_data_path = argv[argc - 1];
    std::cout << test_data_path << std::endl;
    return RUN_ALL_TESTS();

  }

}

