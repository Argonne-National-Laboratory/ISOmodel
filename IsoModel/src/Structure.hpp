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
  * Wall and roof area (m2).
  */
  Vector wallArea() const {
    return m_wallArea;
  }

  void setWallArea(Vector value) {
    m_wallArea = value;
  }

  /**
  *
  */
  Vector windowArea() const {
    return m_windowArea;
  }

  void setWindowArea(Vector value) {
    m_windowArea = value;
  }

  /**
  *
  */
  Vector wallUniform() const {
    return m_wallUniform;
  }

  void setWallUniform(Vector value) {
    m_wallUniform = value;
  }

  /**
  *
  */
  Vector windowUniform() const {
    return m_windowUniform;
  }

  void setWindowUniform(Vector value) {
    m_windowUniform = value;
  }

  /**
  *
  */
  Vector wallThermalEmissivity() const {
    return m_wallThermalEmissivity;
  }

  void setWallThermalEmissivity(Vector value) {
    m_wallThermalEmissivity = value;
  }

  /**
  *
  */
  Vector wallSolarAbsorbtion() const {
    return m_wallSolarAbsorbtion;
  }

  void setWallSolarAbsorbtion(Vector value) {
    m_wallSolarAbsorbtion = value;
  }

  /**
  *
  */
  double windowShadingDevice() const {
    return m_windowShadingDevice;
  }

  void setWindowShadingDevice(double value) {
    m_windowShadingDevice = value;
  }

  /**
  *
  */
  Vector windowNormalIncidenceSolarEnergyTransmittance() const {
    return m_windowNormalIncidenceSolarEnergyTransmittance;
  }

  void setWindowNormalIncidenceSolarEnergyTransmittance(Vector value) {
    m_windowNormalIncidenceSolarEnergyTransmittance = value;
  }

  /**
  *
  */
  Vector windowShadingCorrectionFactor() const {
    return m_windowShadingCorrectionFactor;
  }

  void setWindowShadingCorrectionFactor(Vector value) {
    m_windowShadingCorrectionFactor = value;
  }

  /**
  *
  */
  double interiorHeatCapacity() const {
    return m_interiorHeatCapacity;
  }

  void setInteriorHeatCapacity(double value) {
    m_interiorHeatCapacity = value;
  }

  /**
  *
  */
  double wallHeatCapacity() const {
    return m_wallHeatCapacity;
  }

  void setWallHeatCapacity(double value) {
    m_wallHeatCapacity = value;
  }

  /**
  *
  */
  double buildingHeight() const {
    return m_buildingHeight;
  }

  void setBuildingHeight(double value) {
    m_buildingHeight = value;
  }

  /**
  *
  */
  double infiltrationRate() const {
    return m_infiltrationRate;
  }

  void setInfiltrationRate(double value) {
    m_infiltrationRate = value;
  }

private:
  double m_floorArea;
  Vector m_wallArea;
  Vector m_windowArea;
  Vector m_wallUniform;
  Vector m_windowUniform;
  Vector m_wallThermalEmissivity;
  Vector m_wallSolarAbsorbtion;
  double m_windowShadingDevice;
  Vector m_windowNormalIncidenceSolarEnergyTransmittance;
  Vector m_windowShadingCorrectionFactor;
  double m_interiorHeatCapacity;
  double m_wallHeatCapacity;
  double m_buildingHeight;
  double m_infiltrationRate;

};

} // isomodel
} // openstudio
#endif // ISOMODEL_STRUCTURE_HPP
