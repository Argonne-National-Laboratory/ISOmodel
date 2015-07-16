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

std::vector<std::string> endUseNames = {"ElecHeat",
                                        "ElecCool",
                                        "ElecIntLights",
                                        "ElecExtLights",
                                        "ElecFans",
                                        "ElecPump",
                                        "ElecEquipInt",
                                        "ElecEquipExt",
                                        "ElectDHW",
                                        "GasHeat",
                                        "GasCool",
                                        "GasEquip",
                                        "GasDHW"};

TEST(IsoModelTests, SimModelTests)
{
  // the expected values are the results of running the "prior to updated parameter names
  // and parsing" version and copying the results as they were printed out to stdout.
  // Consequently these are not "exact" and so we use EXPECT_NEAR with 0.001 to test.

  // These updated expected results are copied from running the model after an update that changes the results.
  // They are simply to alert us to changes to the code that affect the results.
  double expected[12][13] =
  {
    { 0, 0.02311660782, 2.72099861, 0.2578224281, 7.47859911, 0.8081682516, 2.244567869, 0, 0, 42.52122747, 0, 0, 0 },
    { 0, 0.04522502229, 2.457676164, 0.1996044604, 6.053356787, 0.6541506903, 2.027351624, 0, 0, 34.36548521, 0, 0, 0 },
    { 0, 0.1359840861, 2.72099861, 0.1841588772, 4.726183855, 0.5107309118, 2.244567869, 0, 0, 26.63274562, 0, 0, 0 },
    { 0, 0.3419236045, 2.633224461, 0.1782182683, 2.712808412, 0.293157261, 2.172162454, 0, 0, 14.76745542, 0, 0, 0 },
    { 0, 1.083670509, 2.72099861, 0.1473271018, 1.409979821, 0.1523682324, 2.244567869, 0, 0, 5.891329141, 0, 0, 0 },
    { 0, 2.129305467, 2.633224461, 0.1425746146, 0.8372379679, 0.09047538646, 2.172162454, 0, 0, 0.572288422, 0, 0, 0 },
    { 0, 3.287411718, 2.72099861, 0.1473271018, 1.1373708, 0.1229089777, 2.244567869, 0, 0, 0, 0, 0, 0 },
    { 0, 1.79919093, 2.72099861, 0.1473271018, 0.6887618997, 0.07443045041, 2.244567869, 0, 0, 0.3772652583, 0, 0, 0 },
    { 0, 0.7586121348, 2.633224461, 0.1782182683, 0.87511792, 0.09456885025, 2.172162454, 0, 0, 3.487115247, 0, 0, 0 },
    { 0, 0.1805043078, 2.72099861, 0.2025747649, 2.760483789, 0.2983092588, 2.244567869, 0, 0, 15.35668752, 0, 0, 0 },
    { 0, 0.04043121136, 2.633224461, 0.2316837487, 4.680144408, 0.5057556994, 2.172162454, 0, 0, 26.55886429, 0, 0, 0 },
    { 0, 0.01853144248, 2.72099861, 0.2578224281, 7.171447165, 0.7749761461, 2.244567869, 0, 0, 40.78200688, 0, 0, 0 }
  };

  openstudio::isomodel::UserModel userModel;
  userModel.load(test_data_path + "/SmallOffice_v2.ism");
  SimModel simModel = userModel.toSimModel();
  ISOResults results = simModel.simulate();

  for (int i = 0; i < 12; ++i) {
    for (int j = 0; j < 13; ++j) {
      EXPECT_NEAR(expected[i][j], results.monthlyResults[i].getEndUse(j), 0.001) << "Month = " << i << ", End Use = " << endUseNames[j] << std::endl;
    }
  }
}
