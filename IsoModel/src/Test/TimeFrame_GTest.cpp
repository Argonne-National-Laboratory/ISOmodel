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

TEST_F(ISOModelFixture, TimeFrameMonthLengthTest) {
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

TEST_F(ISOModelFixture, TimeFrameHourTests) {
  openstudio::isomodel::TimeFrame frame;
  EXPECT_EQ(0, frame.Hour[0]);
  EXPECT_EQ(23, frame.Hour[23]);
  EXPECT_EQ(0, frame.Hour[24]);
  EXPECT_EQ(23, frame.Hour[8759]);
}

TEST_F(ISOModelFixture, TimeFrameDayOfMonthTests) {
  openstudio::isomodel::TimeFrame frame;
  EXPECT_EQ(1, frame.DayOfMonth[0]);
  EXPECT_EQ(1, frame.DayOfMonth[23]);
  EXPECT_EQ(2, frame.DayOfMonth[24]);
  EXPECT_EQ(31, frame.DayOfMonth[8759]);
}

TEST_F(ISOModelFixture, TimeFrameDayOfWeekTests) {
  openstudio::isomodel::TimeFrame frame;
  EXPECT_EQ(0, frame.DayOfWeek[0]);
  EXPECT_EQ(0, frame.DayOfWeek[23]);
  EXPECT_EQ(1, frame.DayOfWeek[24]);
  EXPECT_EQ(6, frame.DayOfWeek[167]);
  EXPECT_EQ(0, frame.DayOfWeek[168]);
}

TEST_F(ISOModelFixture, TimeFrameMonthTests) {
  openstudio::isomodel::TimeFrame frame;
  EXPECT_EQ(1, frame.Month[0]);
  EXPECT_EQ(1, frame.Month[743]);
  EXPECT_EQ(2, frame.Month[744]);
  EXPECT_EQ(12, frame.Month[8759]);
}

TEST_F(ISOModelFixture, TimeFrameTYDTests) {
  openstudio::isomodel::TimeFrame frame;
  EXPECT_EQ(0, frame.YTD[0]);
  EXPECT_EQ(30, frame.YTD[743]);
  EXPECT_EQ(31, frame.YTD[744]);
  EXPECT_EQ(364, frame.YTD[8759]);
}