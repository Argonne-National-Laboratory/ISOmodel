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
#ifndef ISOMODEL_STRUCTURE_HPP
#define ISOMODEL_STRUCTURE_HPP

#include "Vector.hpp"

namespace openstudio {
namespace isomodel {
class Structure
{
public:
  Structure(void);
  ~Structure(void);

  /**
  * Floor area (m2).
  */
  double floorArea() const {
    return m_floorArea;
  }

  void setFloorArea(double value) {
    m_floorArea = value;
  }

  /**
  * Wall and roof area (m2). The order is S, SE, E, NE, N, NW, W, SW, roof to match
  * conventions for sun angles where south is zero.
  */
  Vector wallArea() const {
    return m_wallArea;
  }

  void setWallArea(const Vector& value) {
    m_wallArea = value;
  }

  void setWallArea(int index, double value) {
    m_wallArea[index] = value;
  }

  /**
  * Window and skylight area (m2). The order is S, SE, E, NE, N, NW, W, SW, roof to match
  * conventions for sun angles where south is zero.
  */
  Vector windowArea() const {
    return m_windowArea;
  }

  void setWindowArea(const Vector& value) {
    m_windowArea = value;
  }
  
  void setWindowArea(int index, double value) {
    m_windowArea[index] = value;
  }

  /**
  * Wall and roof U-values (W/m2/K). The order is S, SE, E, NE, N, NW, W, SW, roof to match
  * conventions for sun angles where south is zero.
  */
  Vector wallUniform() const {
    return m_wallUniform;
  }

  void setWallUniform(const Vector& value) {
    m_wallUniform = value;
  }

  void setWallUniform(int index, double value) {
    m_wallUniform[index] = value;
  }

  /**
  * Window and skylight U-values (W/m2/K). The order is S, SE, E, NE, N, NW, W, SW, roof to match
  * conventions for sun angles where south is zero.
  */
  Vector windowUniform() const {
    return m_windowUniform;
  }

  void setWindowUniform(const Vector& value) {
    m_windowUniform = value;
  }

  void setWindowUniform(int index, double value) {
    m_windowUniform[index] = value;
  }

  /**
  * Wall and roof thermal emissivity (ratio compared to black body, 0 to 1).
  * The order is S, SE, E, NE, N, NW, W, SW, roof to match conventions for sun
  * angles where south is zero.
  */
  Vector wallThermalEmissivity() const {
    return m_wallThermalEmissivity;
  }

  void setWallThermalEmissivity(const Vector& value) {
    m_wallThermalEmissivity = value;
  }

  void setWallThermalEmissivity(int index, double value) {
    m_wallThermalEmissivity[index] = value;
  }

  /**
  * Wall and roof solar absorption coeficient (0 to 1).
  * The order is S, SE, E, NE, N, NW, W, SW, roof to match conventions for sun
  * angles where south is zero.
  */
  Vector wallSolarAbsorption() const {
    return m_wallSolarAbsorbtion;
  }

  void setWallSolarAbsorption(const Vector& value) {
    m_wallSolarAbsorbtion = value;
  }

  void setWallSolarAbsorption(int index, double value) {
    m_wallSolarAbsorbtion[index] = value;
  }

  /**
  * Window shading device factors.
  * The order is S, SE, E, NE, N, NW, W, SW, roof to match conventions for sun
  * angles where south is zero.
  */
  Vector windowShadingDevice() const {
    return m_windowShadingDevice;
  }

  void setWindowShadingDevice(const Vector& value) {
    m_windowShadingDevice = value;
  }

  void setWindowShadingDevice(int index, double value) {
    m_windowShadingDevice[index] = value;
  }
  /**
  * Window solar heat gain coeficcient (0 to 1).
  * The order is S, SE, E, NE, N, NW, W, SW, roof to match conventions for sun
  * angles where south is zero.
  */
  Vector windowNormalIncidenceSolarEnergyTransmittance() const {
    return m_windowNormalIncidenceSolarEnergyTransmittance;
  }

  void setWindowNormalIncidenceSolarEnergyTransmittance(const Vector& value) {
    m_windowNormalIncidenceSolarEnergyTransmittance = value;
  }

  void setWindowNormalIncidenceSolarEnergyTransmittance(int index, double value) {
    m_windowNormalIncidenceSolarEnergyTransmittance[index] = value;
  }

  /**
  * Window solar control factor (external control) (0 to 1).
  */
  Vector windowShadingCorrectionFactor() const {
    return m_windowShadingCorrectionFactor;
  }

  void setWindowShadingCorrectionFactor(const Vector& value) {
    m_windowShadingCorrectionFactor = value;
  }

  void setWindowShadingCorrectionFactor(int index, double value) {
    m_windowShadingCorrectionFactor[index] = value;
  }

  /**
  * Interior surface heat capacity (J/K/m2).
  */
  double interiorHeatCapacity() const {
    return m_interiorHeatCapacity;
  }

  void setInteriorHeatCapacity(double value) {
    m_interiorHeatCapacity = value;
  }

  /**
  * Exterior surface (wall) heat capacity (J/K/m2).
  */
  double wallHeatCapacity() const {
    return m_wallHeatCapacity;
  }

  void setWallHeatCapacity(double value) {
    m_wallHeatCapacity = value;
  }

  /**
  * Building height (m).
  */
  double buildingHeight() const {
    return m_buildingHeight;
  }

  void setBuildingHeight(double value) {
    m_buildingHeight = value;
  }

  /**
  * Infiltration rate occupied (m3/m2/hr, based on surface area).
  */
  double infiltrationRate() const {
    return m_infiltrationRate;
  }

  void setInfiltrationRate(double value) {
    m_infiltrationRate = value;
  }

  /**
  * External thermal surface resistance (m2*k/W).
  */
  double R_se() const {
    return m_R_se;
  }

  void setR_se(double R_se) {
    m_R_se = R_se;
  }

  /**
  * Irradiance at which moveable shading is at maximum use (W). This is used to model movable 
  * shading. ISO 13790 does it by switching between g_{gl} and g_{gl+sh}. The method here allows
  * varying degrees of shading rather than just on or off.
  */
  double irradianceForMaxShadingUse() const {
    return m_irradianceForMaxShadingUse;
  }

  void setIrradianceForMaxShadingUse(double irradianceForMaxShadingUse) {
    m_irradianceForMaxShadingUse = irradianceForMaxShadingUse;
  }

  /**
  * Shading factor at max use of moveable shading (unitless). This is used to model movable 
  * shading. ISO 13790 does it by switching between g_{gl} and g_{gl+sh}. The method here allows
  * varying degrees of shading rather than just on or off.
  */
  double shadingFactorAtMaxUse() const {
    return m_shadingFactorAtMaxUse;
  }

  void setShadingFactorAtMaxUse(double shadingFactorAtMaxUse) {
    m_shadingFactorAtMaxUse = shadingFactorAtMaxUse;
  }

  /**
  * Total interior surface area per floor area (m2/m2).
  */
  double totalAreaPerFloorArea() const {
    return m_totalAreaPerFloorArea;
  }

  void setTotalAreaPerFloorArea(double totalAreaPerFloorArea) {
    m_totalAreaPerFloorArea = totalAreaPerFloorArea;
  }

  /**
  * Window frame factor.
  */
  double win_ff() const {
    return m_win_ff;
  }

  void setWin_ff(double win_ff) {
    m_win_ff = win_ff;
  }

  /**
  * Correction factor for non-scattering window as per ISO 13790 11.4.2.
  */
  double win_F_W() const {
    return m_win_F_W;
  }

  void setWin_F_W(double win_F_W) {
    m_win_F_W = win_F_W;
  }

  /**
  * Vertical wall external convection surface heat resistance as per ISO 6946.
  */
  double R_sc_ext() const {
    return m_R_sc_ext;
  }

  void setR_sc_ext(double R_sc_ext) {
    m_R_sc_ext = R_sc_ext;
  }

private:
  double m_floorArea;
  Vector m_wallArea;
  Vector m_windowArea;
  Vector m_wallUniform;
  Vector m_windowUniform;
  Vector m_wallThermalEmissivity;
  Vector m_wallSolarAbsorbtion;
  Vector m_windowShadingDevice;
  Vector m_windowNormalIncidenceSolarEnergyTransmittance;
  Vector m_windowShadingCorrectionFactor;
  double m_interiorHeatCapacity;
  double m_wallHeatCapacity;
  double m_buildingHeight;
  double m_infiltrationRate;
  // Members with default values:
  double m_R_se = 0.04; // Exterior surface thermal resistance.
  double m_irradianceForMaxShadingUse = 500; // The irradiance at which shading is in full use.
  double m_shadingFactorAtMaxUse = 0.5; // Shading factor of moveable shading when in full use.
  double m_totalAreaPerFloorArea = 4.5; // \Lambda_{at}. Ratio of total interior surface area to floor area.
  double m_win_ff = 0.25; // Window frame factor.
  double m_win_F_W = 0.9; // Correction factor for non-scattering window as per ISO 13790 11.4.2
  double m_R_sc_ext = 0.04; // Vertical wall external convection surface heat resistance as per ISO 6946

};

} // isomodel
} // openstudio
#endif // ISOMODEL_STRUCTURE_HPP
