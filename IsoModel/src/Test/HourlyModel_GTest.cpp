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
  double expected[12][13] =
  {
    { 0, 0, 2.74978, 0.257822, 7.25748, 0.186, 2.24088, 0, 0, 41.3248, 0, 0, 0 },
    { 0, 0, 2.48852, 0.207327, 5.78027, 0.168, 2.02735, 0, 0, 32.9002, 0, 0, 0 },
    { 0, 0, 2.78731, 0.210892, 4.20455, 0.1765, 2.26671, 0, 0, 23.8715, 0, 0, 0 },
    { 0, 0.803466, 2.61266, 0.198416, 2.82539, 0.15375, 2.13526, 0, 0, 14.4764, 0, 0, 0 },
    { 0, 1.5465, 2.78731, 0.159208, 1.56159, 0.138, 2.26671, 0, 0, 5.79526, 0, 0, 0 },
    { 0, 3.21538, 2.68771, 0.142575, 1.3296, 0.12175, 2.18692, 0, 0, 1.42311, 0, 0, 0 },
    { 0, 4.64883, 2.71226, 0.148515, 1.59728, 0.119, 2.21505, 0, 0, 0.280847, 0, 0, 0 },
    { 0, 2.73518, 2.78731, 0.178218, 1.0586, 0.11075, 2.26671, 0, 0, 0.765168, 0, 0, 0 },
    { 0, 1.27682, 2.65019, 0.189505, 1.02824, 0.11675, 2.16109, 0, 0, 3.24793, 0, 0, 0 },
    { 0, 0.0637051, 2.74978, 0.226931, 2.22808, 0.16, 2.24088, 0, 0, 12.406, 0, 0, 0 },
    { 0, 0, 2.68771, 0.236436, 4.26135, 0.17275, 2.18692, 0, 0, 24.208, 0, 0, 0 },
    { 0, 0, 2.71226, 0.258416, 7.00312, 0.186, 2.21505, 0, 0, 39.8604, 0, 0, 0 }
  };

  openstudio::isomodel::UserModel userModel;
  userModel.load(test_data_path + "/SmallOffice_v2_ism.yaml");
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
