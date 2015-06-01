/**********************************************************************
 *  Copyright (c) 2008-2013, Alliance for Sustainable Energy.
 *  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 **********************************************************************/
#ifndef ISOMODEL_SOLAR_RADIATION_HPP
#define ISOMODEL_SOLAR_RADIATION_HPP

#include <math.h>
#include <algorithm>
#include <vector>
#include "TimeFrame.hpp"
#include "EpwData.hpp"

namespace openstudio {
namespace isomodel {
const double PI = 3.1415926535897;
const int NUM_SURFACES = 8;
const int MONTHS = 12;
const int HOURS = 24;

class EpwData;
class SolarRadiation
{
protected:
  openstudio::isomodel::TimeFrame* frame;
  openstudio::isomodel::EpwData* epwData;

  //inputs
  double m_surfaceTilt;
  double m_localMeridian; //LSM
  double m_longitude;
  double m_latitude; //latitude in radians
  double m_groundReflectance; // rho_g

  //outputs
  std::vector<std::vector<double> > m_eglobe; //total solar radiation from direct beam, ground reflect and diffuse
  //averages
  std::vector<double> m_monthlyDryBulbTemp;
  std::vector<double> m_monthlyDewPointTemp;
  std::vector<double> m_monthlyRelativeHumidity;
  std::vector<double> m_monthlyWindspeed;
  std::vector<double> m_monthlyGlobalHorizontalRadiation;
  std::vector<std::vector<double> > m_monthlySolarRadiation;
  std::vector<std::vector<double> > m_hourlyDryBulbTemp;
  std::vector<std::vector<double> > m_hourlyDewPointTemp;
  std::vector<std::vector<double> > m_hourlyGlobalHorizontalRadiation;

public:
  void Calculate();

  SolarRadiation(TimeFrame* frame, EpwData* wdata, double tilt = PI);
  ~SolarRadiation(void);

  void calculateSurfaceSolarRadiation();
  void calculateAverages();
  void calculateMonthAvg(int midx, int cnt);
  void clearMonthlyAvg(int midx);

  // Sun position equations.

  /**
  * Calculates the revolution angle in radians of the earth around the sun.
  * ASHRAE2013 Fundamentals, Ch. 14, eq. 6.
  */
  double calculateRevolutionAngle(int dayOfYear) {
    return 2.0 * PI * dayOfYear / 365.0;	//should be .25? <- BAA@2015-05-16: I'm not sure where the previous comment about .25 is from but am leaving it for now to be investigated further.
  }

  /**
  * Calculates the difference between the apparent solar time and mean solar time (the equation of time).
  * ASHRAE2013 Fundamentals, Ch. 14, eq. 5.
  */
  double calculateEquationOfTime(double revolutionAngle) {
    return 2.2918 * (0.0075 + 0.1868 * cos(revolutionAngle) - 3.2077 * sin(revolutionAngle)
           - 1.4615 * cos(2 * revolutionAngle) - 4.089 * sin(2 * revolutionAngle));
  }

  /**
  * Calculates the apparent Solar Time in hours.
  * ASHRAE2013 Fundamentals, Ch. 14, eq. 7. Note that because we use radians, we divide by 15*pi/180,
  * or pi / 12.
  */
  double calculateApparentSolarTime(int hourOfDay, double equationOfTime) {
    return hourOfDay + equationOfTime / 60.0 + (m_longitude - m_localMeridian) / (PI / 12.0);
  }

  /**
  * Calculates the solar declination in radians. The following is a more accurate formula
  * for declination as taken from "Solar Engineering of Thermal Processes," 
  * Duffie and Beckman p. 14, eq. 1.6.1b.
  */
  double calculateSolarDeclination(double revolutionAngle) {
    return 0.006918 - 0.399912 * cos(revolutionAngle) + 0.070257 * sin(revolutionAngle) - 0.006758 * cos(2.0 * revolutionAngle)
           + 0.000907 * sin(2.0 * revolutionAngle) - 0.002697 * cos(3.0 * revolutionAngle) + 0.00148 * sin(3.0 * revolutionAngle);
  }

  /**
  * Calculates the solar hour angle in radians.
  * ASHRAE2013 Fundamentals, Ch. 14, eq. 11.
  */
  double calculateSolarHourAngle(double apparentSolarTime) {
    return 15 * (apparentSolarTime - 12) * PI / 180.0;
  }

  /**
  * Calculates the solar altitude angle in radians.
  * ASHRAE2013 Fundamentals, Ch. 14, eq. 12.
  */
  double calculateSolarAltitude(double solarDeclination, double solarHourAngles) {
    return asin(cos(this->m_latitude) * cos(solarDeclination) * cos(solarHourAngles) + sin(this->m_latitude) * sin(solarDeclination));
  }

  /**
  * Calculates the sin of the solar azimuth.
  * ASHRAE2013 Fundamentals, Ch. 14, eq. 14.
  */
  double calculateSolarAzimuthSin(double solarDeclination, double solarHourAngle, double solarAltitude) {
    return sin(solarHourAngle) * cos(solarDeclination) / cos(solarAltitude);
  }

  /**
  * Calculates the cosine of the solar azimuth.
  * ASHRAE2013 Fundamentals, Ch. 14, eq. 15.
  */
  double calculateSolarAzimuthCos(double solarDeclination, double solarHourAngle, double solarAltitude) {
    return (cos(solarHourAngle) * cos(solarDeclination) * sin(this->m_latitude) - sin(solarDeclination) * cos(this->m_latitude))
           / cos(solarAltitude);
  }

  /**
  * Calculates the solar azimuth from its sin and cos.
  */
  double calculateSolarAzimuth(double solarAzimuthSin, double solarAzimuthCos) {
    return atan2(solarAzimuthSin, solarAzimuthCos);
  }

  // Radiation equations.

  /**
  * Calculates the total ground reflected radiation.
  * ASHRAE2013 Fundamentals, Ch. 14, eq. 31.
  */
  double calculateGroundReflectedIrradiance(double directBeamIrradiance, 
                                           double diffuseIrradiance,
                                           double groundReflectance,
                                           double solarAltitude,
                                           double surfaceTilt) {
    return (directBeamIrradiance * sin(solarAltitude) + diffuseIrradiance)
           * groundReflectance * (1 - cos(surfaceTilt)) / 2;
  }

  /**
  * Calculates the surface solar azimuth (the difference between the surface and solar azimuths). 
  * solarAzimuth and surfaceAzimuth should be in radians. Result in radians.
  * ASHRAE2013 Fundamentals, Ch. 14, eq. 21.
  */
  double calculateSurfaceSolarAzimuth(double solarAzimuth, double surfaceAzimuth) {
    return fabs(solarAzimuth - surfaceAzimuth);
  }

  /**
  * Calculates the angle of incidence of the sun on the surface.
  * ASHRAE2013 Fundamentals, Ch. 14, eq. 22.
  */
  double calculateAngleOfIncidence(double solarAltitude, double surfaceSolarAzimuth, double surfaceTilt) {
    return acos(cos(solarAltitude) * cos(surfaceSolarAzimuth) * sin(surfaceTilt)
           + sin(solarAltitude) * cos(surfaceTilt));
  }

  /** 
  * Calculates the total direct beam irradiance on a surface.
  * ASHRAE2013 Fundamentals, Ch. 14, eq. 26.
  */
  double calculateTotalDirectBeamIrradiance(double directBeamIrradiance, double angleOfIncidence) {
    return directBeamIrradiance * std::max(cos(angleOfIncidence), 0.0);
  }

  /**
  * Calculates the ratio of clear-sky diffuse irradiance on a vertical surface to that on a horizontal surface.
  * ASHRAE2013 Fundamentals, Ch. 14, eq. 28.
  */
  double calculateDiffuseAngleOfIncidenceFactor(double angleOfIncidence) {
    return std::max(0.45, 0.55 + 0.437 * cos(angleOfIncidence) + 0.313 * std::pow(cos(angleOfIncidence), 2.0));
  }

  /**
  * Calculates the total diffuse irradiance on the surface.
  * ASHRAE2013 Fundamentals, Ch. 14, eq. 29, 30.
  */
  double calculateTotalDiffuseIrradiance(double diffuseIrradiance,
                                         double diffuseAngleOfIncidenceFactor,
                                         double surfaceTilt) {
    if (surfaceTilt > PI / 2) {
      // Wall tilted outward.
      return diffuseIrradiance * diffuseAngleOfIncidenceFactor * sin(surfaceTilt);
    } else {
      // Wall vertical or tilted inward.
      return diffuseIrradiance * (diffuseAngleOfIncidenceFactor * sin(surfaceTilt) + cos(surfaceTilt));
    }
  }

  /** 
  * Calculates the total irradiance reaching a surface.
  * ASHRAE2013 Fundamentals, Ch. 14, eq. 25.
  */
  double calculateTotalIrradiance(double totalDirectBeamIrradiance,
                                  double totalDiffuseIrradiance,
                                  double totalGroundReflectedIrradiance) {
    return totalDirectBeamIrradiance + totalDiffuseIrradiance + totalGroundReflectedIrradiance;
  }

  // Getter methods

  double surfaceTilt() {
    return m_surfaceTilt;
  }

  double localMeridian() {
    return m_localMeridian; // meridian of the local time zone, in radians.
  }

  double lon() {
    return m_longitude; // in radians.
  }

  double lat() {
    return m_latitude; // in radians
  }
  
  double groundReflectance() {
    return m_groundReflectance;
  }

  // Outputs
  std::vector<std::vector<double> > eglobe() {
    return m_eglobe;
  }	//total solar radiation from direct beam, ground reflect and diffuse

  // Averages
  std::vector<double> monthlyDryBulbTemp() {
    return m_monthlyDryBulbTemp;
  }

  std::vector<double> monthlyDewPointTemp() {
    return m_monthlyDewPointTemp;
  }

  std::vector<double> monthlyRelativeHumidity() {
    return m_monthlyRelativeHumidity;
  }

  std::vector<double> monthlyWindspeed() {
    return m_monthlyWindspeed;
  }

  std::vector<double> monthlyGlobalHorizontalRadiation() {
    return m_monthlyGlobalHorizontalRadiation;
  }

  std::vector<std::vector<double> > monthlySolarRadiation() {
    return m_monthlySolarRadiation;
  }

  std::vector<std::vector<double> > hourlyDryBulbTemp() {
    return m_hourlyDryBulbTemp;
  }

  std::vector<std::vector<double> > hourlyDewPointTemp() {
    return m_hourlyDewPointTemp;
  }

  std::vector<std::vector<double> > hourlyGlobalHorizontalRadiation() {
    return m_hourlyGlobalHorizontalRadiation;
  }

};

}
}
#endif
