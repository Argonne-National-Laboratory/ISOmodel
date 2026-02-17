// HourlyModel_GTest.cpp — Hourly simulation regression tests.

#include "ISOModelFixture.hpp"

#include "../ISOResults.hpp"
#include "../UserModel.hpp"

#include <array>
#include <gtest/gtest.h>

using namespace openstudio::isomodel;

// Number of months and end-use categories in the results.
constexpr int NUM_MONTHS = 12;
constexpr int NUM_END_USES = 13;

// Tolerance for regression comparisons (results are copied from stdout, not exact).
constexpr double REGRESSION_TOLERANCE = 0.001;

TEST_F(ISOModelFixture, HourlyModelTests) {
  // Expected monthly aggregated results for the hourly model.
  // Updated after ventilation correction (0.34 hardcoded → RHO_CP_AIR_IN_WATT_HOURS).
  const std::array<std::array<double, 13>, 12> expected = {{
      {0, 0, 2.74978, 0.257822, 7.28523, 0.186, 2.24088, 0, 0, 41.4661, 0, 0, 0},
      {0, 0, 2.48852, 0.207327, 5.80074, 0.168, 2.02735, 0, 0, 33.0167, 0, 0, 0},
      {0, 0, 2.78731, 0.210892, 4.22011, 0.1765, 2.26671, 0, 0, 23.961, 0, 0, 0},
      {0, 0.802112, 2.61266, 0.198416, 2.83319, 0.15375, 2.13526, 0, 0, 14.5286, 0, 0, 0},
      {0, 1.54334, 2.78731, 0.159208, 1.5605, 0.138, 2.26671, 0, 0, 5.81642, 0, 0, 0},
      {0, 3.21281, 2.68771, 0.142575, 1.32756, 0.12175, 2.18692, 0, 0, 1.42973, 0, 0, 0},
      {0, 4.64805, 2.71226, 0.148515, 1.59498, 0.119, 2.21505, 0, 0, 0.28226, 0, 0, 0},
      {0, 2.73221, 2.78731, 0.178218, 1.05554, 0.11125, 2.26671, 0, 0, 0.768581, 0, 0, 0},
      {0, 1.27468, 2.65019, 0.189505, 1.02726, 0.117, 2.16109, 0, 0, 3.25967, 0, 0, 0},
      {0, 0.0630526, 2.74978, 0.226931, 2.23488, 0.16025, 2.24088, 0, 0, 12.4503, 0, 0, 0},
      {0, 0, 2.68771, 0.236436, 4.27586, 0.17275, 2.18692, 0, 0, 24.2914, 0, 0, 0},
      {0, 0, 2.71226, 0.258416, 7.02635, 0.186, 2.21505, 0, 0, 39.9926, 0, 0, 0},
  }};

  openstudio::isomodel::UserModel userModel;
  userModel.load(test_data_path + "/test_bldg.yaml");
  HourlyModel hourlyModel = userModel.toHourlyModel();
  auto results = hourlyModel.simulate(true); // aggregateByMonth = true

  for (int i = 0; i < NUM_MONTHS; ++i) {
    for (int j = 0; j < NUM_END_USES; ++j) {
      EXPECT_NEAR(expected[i][j], results[i].getEndUse(j), REGRESSION_TOLERANCE)
          << "Month = " << i << ", End Use = " << endUseNames[j] << "\n";
    }
  }
}
