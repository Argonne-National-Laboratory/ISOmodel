/*
 * ISOModel_GTest.cpp
 *
 *  Created on: Dec 5, 2014
 *      Author: nick
 */

#include "gtest/gtest.h"

#include "ISOModelFixture.hpp"

#include "../UserModel.hpp"
#include "../ISOResults.hpp"

using namespace openstudio::isomodel;

TEST_F(ISOModelFixture, HourlyModelTests)
{
  // The expected values are the results of running the "prior to updated parameter names
  // and parsing" version and copying the results as they were printed out to stdout.
  // Consequently these are not "exact" and so we use EXPECT_NEAR with 0.001 to test.
  
  // These updated expected results are copied from running the model after an update that changes the results.
  // They are simply to alert us to changes to the code that affect the results.
  // these were the original that used pow(x,0.667) and the original solarradiation
  // there are small changes because we now use an equivalent of pow(x,2/3) which is the intention of the standard
  // and we change how solarradiation is accumulated which changes roundoff error in the .1-0.5%% range.  Together these
  // are about a 0.1-1% change so generate new numbers
  //double expected[12][13] =
  //{
  //  { 0, 0, 2.74978, 0.257822, 7.25748, 0.186, 2.24088, 0, 0, 41.3248, 0, 0, 0 },
  //  { 0, 0, 2.48852, 0.207327, 5.77691, 0.168, 2.02735, 0, 0, 32.881, 0, 0, 0 },
  //  { 0, 0, 2.78731, 0.210892, 4.20221, 0.1765, 2.26671, 0, 0, 23.8585, 0, 0, 0 },
  //  { 0, 0.802753, 2.61266, 0.198416, 2.82334, 0.15375, 2.13526, 0, 0, 14.4713, 0, 0, 0 },
  //  { 0, 1.54618, 2.78731, 0.159208, 1.55733, 0.138, 2.26671, 0, 0, 5.7928, 0, 0, 0 },
  //  { 0, 3.2177, 2.68771, 0.142575, 1.32783, 0.12175, 2.18692, 0, 0, 1.42211, 0, 0, 0 },
  //  { 0, 4.64978, 2.71226, 0.148515, 1.59525, 0.119, 2.21505, 0, 0, 0.280617, 0, 0, 0 },
  //  { 0, 2.73324, 2.78731, 0.178218, 1.05517, 0.11075, 2.26671, 0, 0, 0.764797, 0, 0, 0  },
  //  { 0, 1.27623, 2.65019, 0.189505, 1.02548, 0.11675, 2.16109, 0, 0, 3.24652, 0, 0, 0 },
  //  { 0, 0.0638968, 2.74978, 0.226931, 2.22637, 0.16, 2.24088, 0, 0, 12.3989, 0, 0, 0 },
  //  { 0, 0, 2.68771, 0.236436, 4.25925, 0.17275, 2.18692, 0, 0, 24.1964, 0, 0, 0 },
  //  { 0, 0, 2.71226, 0.258416, 7.00312, 0.186, 2.21505, 0, 0, 39.8604, 0, 0, 0 }
  //};
    // here are the correct comparisons for the refactored code
    double expected[12][13] =
    {
      { 0, 0, 2.74978, 0.257822, 7.25656, 0.186, 2.24088, 0, 0, 41.3029, 0, 0, 0 },
      { 0, 0, 2.48852, 0.207327, 5.77721, 0.168, 2.02735, 0, 0, 32.8827, 0, 0, 0 },
      { 0, 0, 2.78731, 0.210892, 4.2024, 0.1765, 2.26671, 0, 0, 23.8597, 0, 0, 0 },
      { 0, 0.80357, 2.61266, 0.198416, 2.82328, 0.154, 2.13526, 0, 0, 14.4695, 0, 0, 0 },
      { 0, 1.54671, 2.78731, 0.159208, 1.55742, 0.138, 2.26671, 0, 0, 5.79229, 0, 0, 0 },
      { 0, 3.21555, 2.68771, 0.142575, 1.32715, 0.12175, 2.18692, 0, 0, 1.42229, 0, 0, 0 },
      { 0, 4.6489, 2.71226, 0.148515, 1.59497, 0.119, 2.21505, 0, 0, 0.280664, 0, 0, 0 },
      { 0, 2.73537, 2.78731, 0.178218, 1.05584, 0.11075, 2.26671, 0, 0, 0.764696, 0, 0, 0 },
      { 0, 1.27694, 2.65019, 0.189505, 1.02569, 0.11675, 2.16109, 0, 0, 3.2464, 0, 0, 0 },
      {0, 0.0637435, 2.74978, 0.226931, 2.2266, 0.16, 2.24088, 0, 0, 12.4006, 0, 0, 0},
      { 0, 0, 2.68771, 0.236436, 4.25931, 0.17275, 2.18692, 0, 0, 24.1968, 0, 0, 0 },
      { 0, 0, 2.71226, 0.258416, 6.99955, 0.186, 2.21505, 0, 0, 39.8401, 0, 0, 0 }
    };

  openstudio::isomodel::UserModel userModel;
  userModel.load(test_data_path + "/test_bldg.yaml");
  HourlyModel hourlyModel = userModel.toHourlyModel();
  auto results = hourlyModel.simulate(true); // aggregateByMonth = true

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
