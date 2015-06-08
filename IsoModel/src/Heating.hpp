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
#ifndef ISOMODEL_HEATING_HPP
#define ISOMODEL_HEATING_HPP

namespace openstudio {
namespace isomodel {
class Heating
{
public:
  Heating(void);
  ~Heating(void);

  /**
  * Heating setpoint occupied (C).
  */
  double temperatureSetPointOccupied() const {
    return m_temperatureSetPointOccupied;
  }

  void setTemperatureSetPointOccupied(double value) {
    m_temperatureSetPointOccupied = value;
  }

  /**
  * Heating setpoint unoccupied (C).
  */
  double temperatureSetPointUnoccupied() const {
    return m_temperatureSetPointUnoccupied;
  }

  void setTemperatureSetPointUnoccupied(double value) {
    m_temperatureSetPointUnoccupied = value;
  }

  /**
  * Heating HVAC loss factor, set based on EN 15243 (unitless).
  */
  double hvacLossFactor() const {
    return m_hvacLossFactor;
  }

  void setHvacLossFactor(double value) {
    m_hvacLossFactor = value;
  }

  /**
  * Heating and cooling HVAC waste factor, set based on EN 15243 (unitless).
  */
  double hotcoldWasteFactor() const {
    return m_hotcoldWasteFactor;
  }

  void setHotcoldWasteFactor(double value) {
    m_hotcoldWasteFactor = value;
  }

  /**
  * Heating system efficiency (unitless).
  */
  double efficiency() const {
    return m_efficiency;
  }
  
  void setEfficiency(double value) {
    m_efficiency = value;
  }

  /**
  * Heating system energy type (electric: energyType() == 1, gas: energyType() != 1).
  * XXX TODO: this probably should be an enum.
  */
  double energyType() const {
    return m_energyType;
  }

  void setEnergyType(double value) {
    m_energyType = value;
  }

  /**
  * Heating pump control reduction (pump control 0 = no pump, 0.5 = auto pump controls 
  * for more 50% of pumps, 1.0 = all other cases). See NEN 2914 9.4.3.
  */
  double pumpControlReduction() const {
    return m_pumpControlReduction;
  }

  void setPumpControlReduction(double value) {
    m_pumpControlReduction = value;
  }

  /**
  * Domestic hot water demand (m3/yr). Use 10 m3/yr/person as a default for offices.
  */
  double hotWaterDemand() const {
    return m_hotWaterDemand;
  }

  void setHotWaterDemand(double value) {
    m_hotWaterDemand = value;
  }

  /**
  * Domestic hot water distribution efficiency (all taps within 3m = 1, taps more 
  * than 3m = 0.8, circulation or unknown = 0.6, see NEN 2916 12.6). 
  */
  double hotWaterDistributionEfficiency() const {
    return m_hotWaterDistributionEfficiency;
  }

  void setHotWaterDistributionEfficiency(double value) {
    m_hotWaterDistributionEfficiency = value;
  }

  /**
  * Domestic hot water system efficiency.
  */
  double hotWaterSystemEfficiency() const {
    return m_hotWaterSystemEfficiency;
  }

  void setHotWaterSystemEfficiency(double value) {
    m_hotWaterSystemEfficiency = value;
  }

  /**
  * Domestic hot water system energy type (electric: energyType() == 1, gas: energyType() != 1).
  * XXX TODO: this probably should be an enum.
  */
  double hotWaterEnergyType() const {
    return m_hotWaterEnergyType;
  }

  void setHotWaterEnergyType(double value) {
    m_hotWaterEnergyType = value;
  }

private:
  double m_temperatureSetPointOccupied;
  double m_temperatureSetPointUnoccupied;
  double m_hvacLossFactor;
  double m_efficiency;
  double m_energyType;
  double m_pumpControlReduction;
  double m_hotWaterDemand;
  double m_hotWaterDistributionEfficiency;
  double m_hotWaterSystemEfficiency;
  double m_hotWaterEnergyType;
  double m_hotcoldWasteFactor;

};

} // isomodel
} // openstudio
#endif // ISOMODEL_HEATING_HPP

