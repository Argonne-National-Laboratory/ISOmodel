/*
 * ISOModel_GTest.cpp
 *
 *  Created on: Dec 5, 2014
 *      Author: nick
 */

#include "gtest/gtest.h"

#include "ISOModelFixture.hpp"

#include "../Properties.hpp"

using namespace openstudio::isomodel;

/*
 * weatherFilePath = ORD.epw
 # Terrain class urban/city = 0.8, suburban/some shielding = 0.9, country/open = 1.0
 terrainClass = 0.8
 # building height is in m,  floor area is in m^2, people density is m2/person
 buildingHeight = 6.33
 occupancyHourEnd = 18
 wallU = 2.1, 234.3, 12.3
 */

TEST_F(ISOModelFixture, PropsKeyValueTests)
{
  Properties props(test_data_path + "/test_properties.props");
  ASSERT_EQ(5, props.size());

  ASSERT_EQ("ORD.epw", *props.getProperty("weatherFilePath"));
  ASSERT_EQ(0.8, *props.getPropertyAsDouble("terrainClass"));
  ASSERT_EQ(6.33, *props.getPropertyAsDouble("buildingHeight"));
  ASSERT_EQ(17.0, *props.getPropertyAsDouble("occupancyHourLast"));
  ASSERT_STREQ("2.1, 234.3, 12.3", (*props.getProperty("wallU")).c_str());

  props.putProperty("a string", "some string");
  ASSERT_STREQ("some string", (*props.getProperty("a string")).c_str());
  props.putProperty("some double", 3.14);
  ASSERT_EQ(3.14, *props.getPropertyAsDouble("some double"));

  // test case insensitivity
  ASSERT_EQ(6.33, *props.getPropertyAsDouble("BUILDINGHEIGHT"));

  std::vector<double> vec;
  props.getPropertyAsDoubleVector("wallU", vec);
  ASSERT_EQ(3, vec.size());
  ASSERT_EQ(2.1, vec[0]);
  ASSERT_EQ(234.3, vec[1]);
  ASSERT_EQ(12.3, vec[2]);
}

TEST_F(ISOModelFixture, PropsMissingValueTests) {
  Properties props(test_data_path + "/test_properties.props");
  
  // Test methods that return boost::optional:
  // Cast to bool to force boost::optional to return a bool like when you do "if (some_optional_val) {..."
  EXPECT_TRUE(bool(props.getProperty("weatherFilePath")));
  EXPECT_TRUE(bool(props.getPropertyAsDouble("buildingHeight")));
  EXPECT_FALSE(bool(props.getProperty("aMissingProperty"))); // Missing.
  EXPECT_FALSE(bool(props.getPropertyAsDouble("aMissingProperty"))); // Missing.
  EXPECT_FALSE(bool(props.getPropertyAsDouble("weatherFilePath"))); // Cannot convert to double.

  // Test methods that return bool
  std::vector<double> vec;
  EXPECT_TRUE(props.getPropertyAsDoubleVector("wallU", vec));
  EXPECT_FALSE(props.getPropertyAsDoubleVector("aMissingProperty", vec)); // Mising
  EXPECT_FALSE(props.getPropertyAsDoubleVector("weatherFilePath", vec)); // Cannot convert to double.
}
