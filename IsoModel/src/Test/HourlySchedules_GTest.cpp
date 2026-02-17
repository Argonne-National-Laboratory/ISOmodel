// HourlySchedules_GTest.cpp — Hourly model with custom schedules regression tests.

#include "ISOModelFixture.hpp"

#include "../ISOResults.hpp"
#include "../UserModel.hpp"

#include <gtest/gtest.h>

using namespace openstudio::isomodel;

// Number of months and end-use categories in the results.
constexpr int NUM_MONTHS = 12;
constexpr int NUM_END_USES = 13;

// Tolerance for regression comparisons (results are copied from stdout, not exact).
constexpr double REGRESSION_TOLERANCE = 0.001;

TEST_F(ISOModelFixture, HourlyModelScheduleTests) {
  // Expected monthly results from test_bldg_schedules_out.txt (without month column).
  // Format: ElecHeat, ElecCool, ElecIntLights, ElecExtLights, ElecFans, ElecPump, ElecEquipInt,
  // ElecEquipExt, ElectDHW, GasHeat, GasCool, GasEquip, GasDHW
  double expected[12][13] = {
      {0, 0, 3.002, 0.0130693, 16.6916, 0.186, 3.196, 0.0246029, 0, 46.1865, 0, 0, 0},
      {0, 0, 2.716, 0.000594061, 13.4495, 0.168, 2.888, 0.0221614, 0, 37.1344, 0, 0, 0},
      {0, 0.00908381, 3.037, 0, 10.5838, 0.17825, 3.206, 0.0241333, 0, 28.1998, 0, 0, 0},
      {0, 0.982583, 2.86, 0, 7.40755, 0.1685, 3.08, 0.0244151, 0, 18.437, 0, 0, 0},
      {0, 2.00843, 3.037, 0, 4.67004, 0.168, 3.206, 0.0241333, 0, 9.73886, 0, 0, 0},
      {0, 3.76702, 2.93, 0, 2.90597, 0.177, 3.1, 0.023476, 0, 4.04501, 0, 0, 0},
      {0, 5.19478, 2.967, 0, 2.54555, 0.18475, 3.186, 0.0250724, 0, 2.01997, 0, 0, 0},
      {0, 3.35538, 3.037, 0, 2.53845, 0.18225, 3.206, 0.0241333, 0, 3.47563, 0, 0, 0},
      {0, 1.74363, 2.895, 0, 3.3997, 0.1645, 3.09, 0.0239455, 0, 6.85617, 0, 0, 0},
      {0, 0.146357, 3.002, 0.00653467, 6.73004, 0.1605, 3.196, 0.0246029, 0, 16.6229, 0, 0, 0},
      {0, 0.00738007, 2.93, 0.0130693, 10.5665, 0.17175, 3.1, 0.023476, 0, 28.4171, 0, 0, 0},
      {0, 0, 2.967, 0.0124753, 16.2766, 0.186, 3.186, 0.0250724, 0, 44.7369, 0, 0, 0}};


  openstudio::isomodel::UserModel userModel;
  userModel.load(test_data_path + "/test_bldg_schedules.yaml");
  HourlyModel hourlyModel = userModel.toHourlyModel();
  auto results = hourlyModel.simulate(true); // aggregateByMonth = true

  for (int i = 0; i < NUM_MONTHS; ++i) {
    for (int j = 0; j < NUM_END_USES; ++j) {
      EXPECT_NEAR(expected[i][j], results[i].getEndUse(j), REGRESSION_TOLERANCE)
          << "Month = " << i << ", End Use = " << endUseNames[j] << "\n";
    }
  }
}
