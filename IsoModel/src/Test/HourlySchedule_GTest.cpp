#include "ISOModelFixture.hpp"

#include "../HourlyModel.hpp"
#include "../UserModel.hpp"

#include <fstream>
#include <gtest/gtest.h>
#include <sstream>
#include <string>

using namespace openstudio::isomodel;

TEST_F(ISOModelFixture, HourlyScheduleTests) {
  // Load the model with schedules
  UserModel userModel;
  std::string yamlPath = "test_data/test_bldg_schedules.yaml";

  ASSERT_NO_THROW(userModel.load(yamlPath)) << "Failed to load " << yamlPath;

  // Convert to HourlyModel
  HourlyModel hourlyModel;
  ASSERT_NO_THROW(hourlyModel = userModel.toHourlyModel()) << "Failed to convert to HourlyModel";

  // Run simulation
  std::vector<EndUses> results;
  ASSERT_NO_THROW(results = hourlyModel.simulate(false)) << "Simulation failed";

  // Verify we got 8760 hourly results
  EXPECT_EQ(8760, results.size()) << "Expected 8760 hourly results";

  // Read expected output
  std::ifstream expectedFile("test_data/test_bldg_schedules_out.txt");
  ASSERT_TRUE(expectedFile.is_open()) << "Failed to open test_bldg_schedules_out.txt";

  // Parse and compare results
  std::string line;
  int lineNum = 0;

  // Skip header if present
  if (std::getline(expectedFile, line) && line.find("Hour") != std::string::npos) {
    lineNum++;
  }

  const double tolerance = 0.01; // 1% tolerance for floating point comparison

  for (size_t i = 0; i < results.size() && std::getline(expectedFile, line); ++i) {
    std::istringstream iss(line);
    int hour;
    double expectedHeating, expectedCooling, expectedLighting, expectedEquipment;

    // Parse expected values (format: hour, heating, cooling, lighting, equipment, ...)
    if (iss >> hour >> expectedHeating >> expectedCooling >> expectedLighting >>
        expectedEquipment) {
      EXPECT_EQ(i, hour) << "Hour mismatch at line " << lineNum;

      double actualHeating = results[i].getEndUse(0);   // Electric heating
      double actualCooling = results[i].getEndUse(1);   // Cooling
      double actualLighting = results[i].getEndUse(2);  // Interior lighting
      double actualEquipment = results[i].getEndUse(6); // Interior equipment

      // Compare with tolerance
      if (expectedHeating > 0.0) {
        EXPECT_NEAR(expectedHeating, actualHeating, expectedHeating * tolerance)
            << "Heating mismatch at hour " << i;
      }
      if (expectedCooling > 0.0) {
        EXPECT_NEAR(expectedCooling, actualCooling, expectedCooling * tolerance)
            << "Cooling mismatch at hour " << i;
      }
      if (expectedLighting > 0.0) {
        EXPECT_NEAR(expectedLighting, actualLighting, expectedLighting * tolerance)
            << "Lighting mismatch at hour " << i;
      }
      if (expectedEquipment > 0.0) {
        EXPECT_NEAR(expectedEquipment, actualEquipment, expectedEquipment * tolerance)
            << "Equipment mismatch at hour " << i;
      }
    }
    lineNum++;
  }

  expectedFile.close();
}

TEST_F(ISOModelFixture, HourlyScheduleDataAccessTests) {
  // Test that we can access hourly data properly
  UserModel userModel;
  userModel.load("test_data/test_bldg_schedules.yaml");

  HourlyModel hourlyModel = userModel.toHourlyModel();

  // Access hourly data
  const auto &hourlyData = hourlyModel.getHourlyData();

  EXPECT_EQ(8760, hourlyData.size()) << "Expected 8760 hours of data";

  // Verify data structure has expected fields
  for (size_t i = 0; i < std::min(size_t(24), hourlyData.size()); ++i) {
    // Just verify we can access the fields without crashing
    float ventilation = hourlyData[i].q_ve_mech;
    float appliances = hourlyData[i].phi_int_App;
    float lighting = hourlyData[i].phi_int_L;
    float extLight = hourlyData[i].ext_light;
    float extEquip = hourlyData[i].ext_equip;
    float heatSetpoint = hourlyData[i].theta_H_set;
    float coolSetpoint = hourlyData[i].theta_C_set;

    // All schedule values should be non-negative
    EXPECT_GE(ventilation, 0.0f) << "Negative ventilation at hour " << i;
    EXPECT_GE(appliances, 0.0f) << "Negative appliances at hour " << i;
    EXPECT_GE(lighting, 0.0f) << "Negative lighting at hour " << i;
  }
}