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
#ifndef ISOMODEL_BUILDING_HPP
#define ISOMODEL_BUILDING_HPP

namespace openstudio {
namespace isomodel {
class Building
{
public:
  Building(void);
  ~Building(void);

  /**
  * Lighting occupancy sensor dimming fraction (unitless).
  * Illum controls are set to 1 if there is no control.
  * See iso 15193 Annex F/G for values.
  * XXX: Should this be in the Lighting class?
  */
  double lightingOccupancySensor() const {
    return m_lightingOccupancySensor;
  }

  void setLightingOccupancySensor(double value) {
    m_lightingOccupancySensor = value;
  }

  /**
  * Constant illumination control multiplier (unitless).
  * Illum controls are set to 1 if there is no control.
  * See iso 15193 Annex F/G for values.
  * XXX: Should this be in the Lighting class?
  */
  double constantIllumination() const {
    return m_constantIllumination;
  }

  void setConstantIllumination(double value) {
    m_constantIllumination = value;
  }

  /**
  * Electric appliance power density occupied (W/m2).
  * This value is used for both the electricity consumed and the heat produced by the appliances.
  */
  double electricApplianceHeatGainOccupied() const {
    return m_electricApplianceHeatGainOccupied;
  }

  void setElectricApplianceHeatGainOccupied(double value) {
    m_electricApplianceHeatGainOccupied = value;
  }

  /**
  * Electric appliance power density unoccupied (W/m2).
  * This value is used for both the electricity consumed and the heat produced by the appliances.
  */
  double electricApplianceHeatGainUnoccupied() const {
    return m_electricApplianceHeatGainUnoccupied;
  }

  void setElectricApplianceHeatGainUnoccupied(double value) {
    m_electricApplianceHeatGainUnoccupied = value;
  }

  /**
  * Gas appliance power density occupied (W/m2).
  * This value is used for both the energy consumed and the heat produced by the appliances.
  */
  double gasApplianceHeatGainOccupied() const {
    return m_gasApplianceHeatGainOccupied;
  }

  void setGasApplianceHeatGainOccupied(double value) {
    m_gasApplianceHeatGainOccupied = value;
  }

  /**
  * Gas appliance power density unoccupied (W/m2).
  * This value is used for both the energy consumed and the heat produced by the appliances.
  */
  double gasApplianceHeatGainUnoccupied() const {
    return m_gasApplianceHeatGainUnoccupied;
  }

  void setGasApplianceHeatGainUnoccupied(double value) {
    m_gasApplianceHeatGainUnoccupied = value;
  }

  /**
  * Building energy management type: none (0), simple (1) or advanced (2).
  * Used to adjust the heating and cooling set points in the monthly calculations.
  */
  double buildingEnergyManagement() const {
    return m_buildingEnergyManagement;
  }

  void setBuildingEnergyManagement(double value) {
    m_buildingEnergyManagement = value;
  }

  /**
  * External equipment energy use (W).
  */

  double externalEquipment() const {
    return m_externalEquipment;
  }

  void setExternalEquipment(double externalEquipment) {
    m_externalEquipment = externalEquipment;
  }

private:
  double m_lightingOccupancySensor;
  double m_constantIllumination;
  double m_electricApplianceHeatGainOccupied;
  double m_electricApplianceHeatGainUnoccupied;
  double m_gasApplianceHeatGainOccupied;
  double m_gasApplianceHeatGainUnoccupied;
  double m_buildingEnergyManagement;
  // Members with default values:
  double m_externalEquipment = 0.0;
};

} // isomodel
} // openstudio
#endif // ISOMODEL_BUILDING_HPP
