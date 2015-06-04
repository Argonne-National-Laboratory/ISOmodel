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

  // Getters
  double temperatureSetPointOccupied() const {
    return m_temperatureSetPointOccupied;
  }

  double temperatureSetPointUnoccupied() const {
    return m_temperatureSetPointUnoccupied;
  }

  double hvacLossFactor() const {
    return m_hvacLossFactor;
  }

  double hotcoldWasteFactor() const {
    return m_hotcoldWasteFactor;
  }

  double efficiency() const {
    return m_efficiency;
  }

  double energyType() const {
    return m_energyType;
  }

  double pumpControlReduction() const {
    return m_pumpControlReduction;
  }

  double hotWaterDemand() const {
    return m_hotWaterDemand;
  }

  double hotWaterDistributionEfficiency() const {
    return m_hotWaterDistributionEfficiency;
  }

  double hotWaterSystemEfficiency() const {
    return m_hotWaterSystemEfficiency;
  }

  double hotWaterEnergyType() const {
    return m_hotWaterEnergyType;
  }

  // Setters
  void setTemperatureSetPointOccupied(double value) {
    m_temperatureSetPointOccupied = value;
  }

  void setTemperatureSetPointUnoccupied(double value) {
    m_temperatureSetPointUnoccupied = value;
  }

  void setHvacLossFactor(double value) {
    m_hvacLossFactor = value;
  }

  void setHotcoldWasteFactor(double value) {
    m_hotcoldWasteFactor = value;
  }

  void setEfficiency(double value) {
    m_efficiency = value;
  }

  void setEnergyType(double value) {
    m_energyType = value;
  }

  void setPumpControlReduction(double value) {
    m_pumpControlReduction = value;
  }

  void setHotWaterDemand(double value) {
    m_hotWaterDemand = value;
  }

  void setHotWaterDistributionEfficiency(double value) {
    m_hotWaterDistributionEfficiency = value;
  }

  void setHotWaterSystemEfficiency(double value) {
    m_hotWaterSystemEfficiency = value;
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

