// OptimizationCoverage_GTest.cpp
//
// Verifies correctness of recent C++20 optimizations and constant replacements.

#include "../Constants.hpp"
#include "../EpwData.hpp"
#include "../SolarRadiation.hpp"
#include "../UserModel.hpp"

#include <cmath>
#include <gtest/gtest.h>
#include <numbers>
#include <string>
#include <vector>

using namespace openstudio::isomodel;

// 1. Verify Constants updates
TEST(OptimizationCoverage, Constants_Values) {
  // Check C++20 std::numbers usage
  EXPECT_DOUBLE_EQ(PI, std::numbers::pi);

  // Check integer constants used in loops
  EXPECT_EQ(DAYS_IN_YEAR, 365);
  EXPECT_EQ(HOURS_IN_YEAR, 8760);
  EXPECT_EQ(NUM_VERTICAL_SURFACES, 8);

  // Check array sizes
  EXPECT_EQ(WIN_SDF_TABLE.size(), 3);
  EXPECT_EQ(ENV_FORM_FACTORS.size(), 9);
}

// 2. Verify SolarRadiation Math Helpers
// Ensures replacement of magic numbers with constants didn't break formulas
TEST(OptimizationCoverage, SolarRadiation_Math) {
  TimeFrame frame;
  EpwData epw;
  SolarRadiation solar(&frame, &epw);

  // Test Revolution Angle for Day 0
  double rev0 = solar.calculateRevolutionAngle(0);
  EXPECT_DOUBLE_EQ(rev0, 0.0);

  // Test Revolution Angle for Day 365 (should be 2*PI)
  // This verifies DAYS_IN_YEAR constant is used correctly in division
  double rev365 = solar.calculateRevolutionAngle(365);
  EXPECT_DOUBLE_EQ(rev365, 2.0 * PI);

  // Test Equation of Time (approx check for Day 0)
  // 2.2918 * (0.0075 + 0.1868 - 1.4615) approx -2.9
  double eot = solar.calculateEquationOfTime(0.0);
  EXPECT_NEAR(eot, -2.9, 0.1);
}

// Helper class to access protected members of EpwData for testing
class TestEpwData : public EpwData {
public:
  using EpwData::m_data;
  using EpwData::m_latitude;
  using EpwData::m_longitude;
  using EpwData::parseData;
  using EpwData::parseHeader;

  // Helper to resize vectors for testing since parseData expects them ready
  void resizeData(size_t size) {
    for (auto &col : m_data) {
      col.resize(size, 0.0);
    }
  }

  double getData(int col, int row) { return m_data[col][row]; }
};

// 3. Verify EpwData String Parsing Optimizations
// Tests the change from std::string to const std::string& and pointer
// arithmetic
TEST(OptimizationCoverage, EpwData_Parsing) {
  TestEpwData epw;

  // Test Header Parsing
  // Format: LOCATION,City,State,Country,Source,ID,Lat,Lon,TimeZone,Elev
  std::string header = "LOCATION,Denver,CO,USA,WMO,725650,39.83,-104.65,-7.0,1611";
  epw.parseHeader(header);

  EXPECT_NEAR(epw.latitude(), 39.83, 0.001);
  EXPECT_NEAR(epw.longitude(), -104.65, 0.001);

  // Test Data Parsing
  // We need to construct a CSV line where specific indices map to specific
  // columns Indices: DBT=6(0), DPT=7(1), RH=8(2), EGH=13(3), EB=14(4),
  // ED=15(5), WSPD=21(6)

  // Constructing a line with enough commas to reach index 21
  // 0,1,2,3,4,5, 6(DBT), 7(DPT), 8(RH), 9,10,11,12, 13(EGH), 14(EB), 15(ED),
  // 16,17,18,19,20, 21(WSPD) Note: Added trailing comma/dummy field because
  // parseData loop requires a trailing comma to process the field
  std::string dataLine = "1999,1,1,1,0,?, -12.2, -16.1, 72, 0,0,0,0, 500, 800, "
                         "100, 0,0,0,0,0, 2.6, 0";

  epw.resizeData(1); // Resize for 1 row
  epw.parseData(dataLine, 0);

  // Verify values were parsed correctly using strtod on the view
  EXPECT_NEAR(epw.getData(0, 0), -12.2, 0.001); // DBT
  EXPECT_NEAR(epw.getData(1, 0), -16.1, 0.001); // DPT
  EXPECT_NEAR(epw.getData(2, 0), 72.0, 0.001);  // RH
  EXPECT_NEAR(epw.getData(3, 0), 500.0, 0.001); // EGH
  EXPECT_NEAR(epw.getData(6, 0), 2.6, 0.001);   // WSPD
}

// 4. Verify UserModel Modern C++ Features
TEST(OptimizationCoverage, UserModel_ModernFeatures) {
  UserModel user;
  std::string path = "test_weather.epw";

  // Test std::move optimization in setter
  user.setWeatherFilePath(path);
  EXPECT_EQ(user.weatherFilePath(), path);

  // Test [[nodiscard]] getters (compile-time check mostly, but runtime check
  // here)
  EXPECT_FALSE(user.valid());

  // Test string_view compatible setters (implicit conversion)
  user.setBemType("simple");
  EXPECT_EQ(user.bemType(), "simple");

  user.setBemType("ADVANCED"); // Case insensitivity check
  EXPECT_EQ(user.bemType(), "advanced");
}
