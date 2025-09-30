/*
 * ISOModel_GTest.cpp
 *
 *  Created on: Dec 5, 2014
 *      Author: nick
 */

#include "gtest/gtest.h"

#include "ISOModelFixture.hpp"

#include "../Properties.hpp"
#include "../UserModel.hpp"

using namespace openstudio::isomodel;

// Solar tests. Expected results based on working through the equations in
// ASHRAE Fundamentals, Ch. 14 Climatic Design Information (with the exception of using
// Duffie and Beckman p.14, eq. 1.6.1b for declination). An ipython notebook of
// the hand calculations is saved in the test_data directory.

// Inputs:
// 41.98 N, 87.92 W (matches ORD.EPW)
// Jan 21, 2009, 12:00 (noon).
// GMT -6, Daylight savings: No.
// South facing surface.

// Expected results:
// Rotation: 0.34428412642079925 rad
// Equation of Time: -10.602150196429877 min
// Apparent Solar Time: 11.961964163392835 hours
// Declination: -0.35056553686581415 rad
// Hour angle: -0.009957758738184193 rad
// Altitude: 0.4875023918786105 rad
// sin(azimuth): -0.010585060645453042
// cos(azimuth): 0.9999439766762598
// Azimuth: -0.010585258319975917 rad

TEST_F(ISOModelFixture, SunPositionAndRadiationTests) {
  openstudio::isomodel::UserModel userModel;
  userModel.load(test_data_path + "/SmallOffice_v2_ism.yaml");
  userModel.loadWeather();

  TimeFrame frame;
  SolarRadiation solarRadiation(&frame, userModel.epwData().get());

  auto hourOfYear = 492;
  auto surfaceAzimuth = 0.0; // South facing surface.

  // Confirm that we are testing 12noon, Jan 21.
  EXPECT_EQ(1, frame.Month[hourOfYear]);
  EXPECT_EQ(21, frame.DayOfMonth[hourOfYear]);
  EXPECT_EQ(12, frame.Hour[hourOfYear]);

  // Confirm the solar radiation input variables are what we think they are.
  // The lat, lon and meridian are based on the weather file. The tilt is currently
  // defaulted to pi/2.
  EXPECT_NEAR(41.98 * PI / 180.0, solarRadiation.lat(), 0.0001);
  EXPECT_NEAR(-87.92 * PI / 180.0, solarRadiation.lon(), 0.0001);
  EXPECT_NEAR(-90.0 * PI / 180.0, solarRadiation.localMeridian(), 0.0001);
  EXPECT_NEAR(PI / 2.0, solarRadiation.surfaceTilt(), 0.0001);
  EXPECT_NEAR(0.14, solarRadiation.groundReflectance(), 0.0001);

  // Confirm the radiation values for 12noon, Jan 21 are what we expect them to be:
  auto directBeamIrradiance = userModel.epwData()->data()[EB][hourOfYear];
  auto diffuseIrradiance = userModel.epwData()->data()[ED][hourOfYear];

  EXPECT_NEAR(320.0, directBeamIrradiance, 0.0001);
  EXPECT_NEAR(175.0, diffuseIrradiance, 0.0001);

  // Test the sun position methods.
  auto revolution = solarRadiation.calculateRevolutionAngle(frame.YTD[hourOfYear]);
  EXPECT_NEAR(0.34428412642079925, revolution, 0.0001);

  auto equationOfTime = solarRadiation.calculateEquationOfTime(revolution);
  EXPECT_NEAR(-10.602150196429877, equationOfTime, 0.0001);

  auto apparentSolarTime = solarRadiation.calculateApparentSolarTime(frame.Hour[hourOfYear], equationOfTime);
  EXPECT_NEAR(11.961964163392835, apparentSolarTime, 0.0001);

  auto solarDeclination = solarRadiation.calculateSolarDeclination(revolution);
  EXPECT_NEAR(-0.35056553686581415, solarDeclination, 0.0001);

  auto solarHourAngle = solarRadiation.calculateSolarHourAngle(apparentSolarTime);
  EXPECT_NEAR(-0.009957758738184193, solarHourAngle, 0.0001);

  auto solarAltitudeAngle = solarRadiation.calculateSolarAltitude(solarDeclination, solarHourAngle);
  EXPECT_NEAR(0.4875023918786105, solarAltitudeAngle, 0.0001);

  auto solarAzimuthSin = solarRadiation.calculateSolarAzimuthSin(solarDeclination, solarHourAngle, solarAltitudeAngle);
  EXPECT_NEAR(-0.010585060645453042, solarAzimuthSin, 0.0001);

  auto solarAzimuthCos = solarRadiation.calculateSolarAzimuthCos(solarDeclination, solarHourAngle, solarAltitudeAngle);
  EXPECT_NEAR(0.9999439766762598, solarAzimuthCos, 0.0001);

  auto solarAzimuth = solarRadiation.calculateSolarAzimuth(solarAzimuthSin, solarAzimuthCos);
  EXPECT_NEAR(-0.010585258319975917, solarAzimuth, 0.0001);

  // Test the radiation methods.

  auto groundReflectedIrradiance = solarRadiation.calculateGroundReflectedIrradiance(directBeamIrradiance,
                                                                                     diffuseIrradiance,
                                                                                     solarRadiation.groundReflectance(),
                                                                                     solarAltitudeAngle,
                                                                                     solarRadiation.surfaceTilt());
  EXPECT_NEAR(22.742623699187682, groundReflectedIrradiance, 0.0001);

  auto surfaceSolarAzimuth = solarRadiation.calculateSurfaceSolarAzimuth(solarAzimuth, surfaceAzimuth);
  EXPECT_NEAR(0.010585258319975917, surfaceSolarAzimuth, 0.0001);

  auto angleOfIncidence = solarRadiation.calculateAngleOfIncidence(solarAltitudeAngle, solarAzimuth, solarRadiation.surfaceTilt());
  EXPECT_NEAR(0.4876080490062035, angleOfIncidence, 0.0001);

  auto totalDirectBeamIrradiance = solarRadiation.calculateTotalDirectBeamIrradiance(directBeamIrradiance, angleOfIncidence);
  EXPECT_NEAR(282.7059351987666, totalDirectBeamIrradiance, 0.0001);
  
  auto diffuseAngleOfIncidenceFactor = solarRadiation.calculateDiffuseAngleOfIncidenceFactor(angleOfIncidence);
  EXPECT_NEAR(1.1803650987552168, diffuseAngleOfIncidenceFactor, 0.0001);

  auto totalDiffuseIrradiance = solarRadiation.calculateTotalDiffuseIrradiance(diffuseIrradiance,
                                                                               diffuseAngleOfIncidenceFactor,
                                                                               solarRadiation.surfaceTilt());
  EXPECT_NEAR(206.56389228216293, totalDiffuseIrradiance, 0.0001);

  auto totalIrradiance = solarRadiation.calculateTotalIrradiance(totalDirectBeamIrradiance, totalDiffuseIrradiance, groundReflectedIrradiance);
  EXPECT_NEAR(512.0124511801172, totalIrradiance, 0.0001);
}
