#pragma once
#include "ISOModelAPI.hpp"


namespace openstudio::isomodel {

class ISOMODEL_API Building {
public:
  // Use compiler-generated default constructor/destructor
  Building() = default;
  ~Building() = default;

  /**
   * lighting occupancy sensor dimming fraction (unitless).
   * Illum controls are set to 1 if there is no control.
   * See iso 15193 Annex F/G for values.
   */
  [[nodiscard]] double lightingOccupancySensor() const { return m_lightingOccupancySensor; }
  void setLightingOccupancySensor(double value) { m_lightingOccupancySensor = value; }

  /**
   * Constant illumination control multiplier (unitless).
   * Illum controls are set to 1 if there is no control.
   */
  [[nodiscard]] double constantIllumination() const { return m_constantIllumination; }
  void setConstantIllumination(double value) { m_constantIllumination = value; }

  /**
   * Electric appliance power density occupied (W/m2).
   */
  [[nodiscard]] double electricApplianceHeatGainOccupied() const { return m_electricApplianceHeatGainOccupied; }
  void setElectricApplianceHeatGainOccupied(double value) {
    m_electricApplianceHeatGainOccupied = value;
  }

  /**
   * Electric appliance power density unoccupied (W/m2).
   */
  [[nodiscard]] double electricApplianceHeatGainUnoccupied() const {
    return m_electricApplianceHeatGainUnoccupied;
  }
  void setElectricApplianceHeatGainUnoccupied(double value) {
    m_electricApplianceHeatGainUnoccupied = value;
  }

  /**
   * Gas appliance power density occupied (W/m2).
   */
  [[nodiscard]] double gasApplianceHeatGainOccupied() const { return m_gasApplianceHeatGainOccupied; }
  void setGasApplianceHeatGainOccupied(double value) { m_gasApplianceHeatGainOccupied = value; }

  /**
   * Gas appliance power density unoccupied (W/m2).
   */
  [[nodiscard]] double gasApplianceHeatGainUnoccupied() const { return m_gasApplianceHeatGainUnoccupied; }
  void setGasApplianceHeatGainUnoccupied(double value) { m_gasApplianceHeatGainUnoccupied = value; }

  /**
   * Building energy management temperature adjustment (K).
   */
  [[nodiscard]] double buildingEnergyManagement() const { return m_buildingEnergyManagement; }
  void setBuildingEnergyManagement(double value) { m_buildingEnergyManagement = value; }

  /**
   * External equipment energy use (W).
   */
  [[nodiscard]] double externalEquipment() const { return m_externalEquipment; }
  void setExternalEquipment(double externalEquipment) { m_externalEquipment = externalEquipment; }

  // Unused properties preserved for interface compatibility
  [[nodiscard]] double electricAppliancePowerFixedOccupied() const {
    return m_electricAppliancePowerFixedOccupied;
  }
  void setElectricAppliancePowerFixedOccupied(double value) {
    m_electricAppliancePowerFixedOccupied = value;
  }

  [[nodiscard]] double electricAppliancePowerFixedUnoccupied() const {
    return m_electricAppliancePowerFixedUnoccupied;
  }
  void setElectricAppliancePowerFixedUnoccupied(double value) {
    m_electricAppliancePowerFixedUnoccupied = value;
  }

  [[nodiscard]] double gasAppliancePowerFixedOccupied() const { return m_gasAppliancePowerFixedOccupied; }
  void setGasAppliancePowerFixedOccupied(double value) { m_gasAppliancePowerFixedOccupied = value; }

  [[nodiscard]] double gasAppliancePowerFixedUnoccupied() const { return m_gasAppliancePowerFixedUnoccupied; }
  void setGasAppliancePowerFixedUnoccupied(double value) {
    m_gasAppliancePowerFixedUnoccupied = value;
  }

private:
  // In-class initialization ensures safer default state
  double m_lightingOccupancySensor = 0.0;
  double m_constantIllumination = 0.0;
  double m_electricApplianceHeatGainOccupied = 0.0;
  double m_electricApplianceHeatGainUnoccupied = 0.0;
  double m_gasApplianceHeatGainOccupied = 0.0;
  double m_gasApplianceHeatGainUnoccupied = 0.0;
  double m_buildingEnergyManagement = 0.0;
  double m_externalEquipment = 0.0;

  double m_electricAppliancePowerFixedOccupied = 0.0;
  double m_electricAppliancePowerFixedUnoccupied = 0.0;
  double m_gasAppliancePowerFixedOccupied = 0.0;
  double m_gasAppliancePowerFixedUnoccupied = 0.0;
};

} // namespace openstudio::isomodel
