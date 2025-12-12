/*
 * ISOModel_GTest.cpp
 *
 *  Created on: Dec 5, 2014
 *      Author: nick
 */

#include "gtest/gtest.h"

#include "ISOModelFixture.hpp"
#include "../UserModel.hpp"

using namespace openstudio::isomodel;

TEST_F(ISOModelFixture, MonthlyModelTests)
{
  // the expected values are the results of running the "prior to updated parameter names
  // and parsing" version and copying the results as they were printed out to stdout.
  // Consequently these are not "exact" and so we use EXPECT_NEAR with 0.001 to test.

  // These updated expected results are copied from running the model after an update that changes the results.
  // They are simply to alert us to changes to the code that affect the results.
  double expected[12][13] =
  {
    {0, 0.01498967975, 2.641326323, 0.2578224281, 9.347675965, 0.8179088504, 2.187376303, 0, 0, 53.17565776, 0, 0, 0},
    {0, 0.02964846487, 2.385714098, 0.1996044604, 7.686021656, 0.6725163731, 1.975694725, 0, 0, 43.68897398, 0, 0, 0},
    {0, 0.08908423082, 2.641326323, 0.1841588772, 6.222024128, 0.5444185935, 2.187376303, 0, 0, 35.2391381, 0, 0, 0},
    {0, 0.2437609353, 2.556122248, 0.1782182683, 3.505630687, 0.3067378861, 2.116815777, 0, 0, 19.47335318, 0, 0, 0},
    {0, 0.8762048882, 2.641326323, 0.1473271018, 1.650357261, 0.1444040011, 2.187376303, 0, 0, 7.668060541, 0, 0, 0},
    {0, 1.730176195, 2.556122248, 0.1425746146, 0.7597026635, 0.06647294306, 2.116815777, 0, 0, 0.9169528979, 0, 0, 0},
    {0, 2.913091177, 2.641326323, 0.1473271018, 1.00786428, 0.08818674475, 2.187376303, 0, 0, -3.617023241e-17, 0, 0, 0},
    {0, 1.519930898, 2.641326323, 0.1473271018, 0.6222102589, 0.05444254588, 2.187376303, 0, 0, 0.5483955731, 0, 0, 0},
    {0, 0.6192127232, 2.556122248, 0.1782182683, 0.9997738536, 0.08747884355, 2.116815777, 0, 0, 4.471143496, 0, 0, 0},
    {0, 0.1256179277, 2.641326323, 0.2025747649, 3.54087291, 0.3098215324, 2.187376303, 0, 0, 19.90659684, 0, 0, 0},
    {0, 0.02568937976, 2.556122248, 0.2316837487, 6.019134793, 0.5266660545, 2.116815777, 0, 0, 34.20917053, 0, 0, 0},
    {0, 0.01236678323, 2.641326323, 0.2578224281, 8.696658891, 0.7609457477, 2.187376303, 0, 0, 49.47535873, 0, 0, 0}
  };

  openstudio::isomodel::UserModel userModel;
  userModel.load(test_data_path + "/test_bldg.yaml");
  auto monthlyModel = userModel.toMonthlyModel();
  auto results = monthlyModel.simulate();

  for (int i = 0; i < 12; ++i) {
    for (int j = 0; j < 13; ++j) {
#ifdef ISOMODEL_STANDALONE
      EXPECT_NEAR(expected[i][j], results[i].getEndUse(j), 0.001) << "Month = " << i << ", End Use = " << endUseNames[j] << "\n";
#else 
      EXPECT_NEAR(expected[i][j], results[i].getEndUse(isoResultsEndUseTypes[j].first, isoResultsEndUseTypes[j].second), 0.001)
        << "Month = " << i << ", End Use = " << endUseNames[j] << "\n";
#endif
    }
  }
}
