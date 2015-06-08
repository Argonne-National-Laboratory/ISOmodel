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
#ifndef ISOMODEL_COOLING_HPP
#define ISOMODEL_COOLING_HPP

namespace openstudio {
namespace isomodel {
class Cooling
{
public:
  Cooling(void);
  ~Cooling(void);

  /**
  * Cooling setpoint occupied (C).
  */
  double temperatureSetPointOccupied() const {
    return m_temperatureSetPointOccupied;
  }
  
  void setTemperatureSetPointOccupied(double value) {
    m_temperatureSetPointOccupied = value;
  }

  /**
  * Cooling setpoint unoccupied (C).
  */
  double temperatureSetPointUnoccupied() const {
    return m_temperatureSetPointUnoccupied;
  }

  void setTemperatureSetPointUnoccupied(double value) {
    m_temperatureSetPointUnoccupied = value;
  }

  /**
  * Coefficient of performance (W/W).
  */
  double cop() const {
    return m_cop;
  }

  void setCop(double value) {
    m_cop = value;
  }

  /**
  * Cooling system IPLV (integrated part load value) to COP ratio (unitless).
  */
  double partialLoadValue() const {
    return m_partialLoadValue;
  }
  
  void setPartialLoadValue(double value) {
    m_partialLoadValue = value;
  }

  /**
  * Cooling HVAC loss factor, set based on EN 15243 (unitless).
  */
  double hvacLossFactor() const {
    return m_hvacLossFactor;
  }

  void setHvacLossFactor(double value) {
    m_hvacLossFactor = value;
  }

  /**
  * Cooling pump control reduction (pump control 0 = no pump, 0.5 = auto pump controls 
  * for more 50% of pumps, 1.0 = all other cases). See NEN 2914 9.4.3.
  */
  double pumpControlReduction() const {
    return m_pumpControlReduction;
  }

  void setPumpControlReduction(double value) {
    m_pumpControlReduction = value;
  }

private:
  double m_temperatureSetPointOccupied;
  double m_temperatureSetPointUnoccupied;
  double m_cop;
  double m_partialLoadValue;
  double m_hvacLossFactor;
  double m_pumpControlReduction;
};

} // isomodel
} // openstudio
#endif // ISOMODEL_COOLING_HPP
