/// @file Building.hpp
/// @brief Building-level properties for internal gains and controls.
///
/// Holds appliance power densities (electric and gas, occupied and unoccupied),
/// lighting occupancy sensor and constant illumination control multipliers,
/// building energy management (BEM) temperature adjustment, and external
/// equipment energy use. Used by both MonthlyModel and HourlyModel.
///
/// @author Brian Craig
/// @author Nick Collier
/// @author Brendan Albano
/// @author Ralph Muehleisen
/// @date 2013-11-05
/// @copyright Copyright Argonne National Laboratory
#pragma once
#include "ISOModelAPI.hpp"


namespace openstudio::isomodel {

class ISOMODEL_API Building {
public:
  // Use compiler-generated default constructor/destructor
  Building() = default;
  ~Building() = default;

  /// lighting occupancy sensor dimming fraction (unitless).
  /// Illum controls are set to 1 if there is no control.
  /// See iso 15193 Annex F/G for values.
  [[nodiscard]] double lightingOccupancySensor() const noexcept {
    return m_lightingOccupancySensor;
  }
  void setLightingOccupancySensor(double value) { m_lightingOccupancySensor = value; }

  /// Constant illumination control multiplier (unitless).
  /// Illum controls are set to 1 if there is no control.
  [[nodiscard]] double constantIllumination() const noexcept { return m_constantIllumination; }
  void setConstantIllumination(double value) { m_constantIllumination = value; }

  /// Electric appliance power density occupied (W/m2).
  [[nodiscard]] double electricApplianceHeatGainOccupied() const noexcept {
    return m_electricApplianceHeatGainOccupied;
  }
  void setElectricApplianceHeatGainOccupied(double value) noexcept {
    m_electricApplianceHeatGainOccupied = value;
  }

  /// Electric appliance power density unoccupied (W/m2).
  [[nodiscard]] double electricApplianceHeatGainUnoccupied() const noexcept {
    return m_electricApplianceHeatGainUnoccupied;
  }
  void setElectricApplianceHeatGainUnoccupied(double value) noexcept {
    m_electricApplianceHeatGainUnoccupied = value;
  }

  /// Gas appliance power density occupied (W/m2).
  [[nodiscard]] double gasApplianceHeatGainOccupied() const noexcept {
    return m_gasApplianceHeatGainOccupied;
  }
  void setGasApplianceHeatGainOccupied(double value) { m_gasApplianceHeatGainOccupied = value; }

  /// Gas appliance power density unoccupied (W/m2).
  [[nodiscard]] double gasApplianceHeatGainUnoccupied() const noexcept {
    return m_gasApplianceHeatGainUnoccupied;
  }
  void setGasApplianceHeatGainUnoccupied(double value) { m_gasApplianceHeatGainUnoccupied = value; }

  /// Building energy management temperature adjustment (K).
  [[nodiscard]] double buildingEnergyManagement() const noexcept {
    return m_buildingEnergyManagement;
  }
  void setBuildingEnergyManagement(double value) { m_buildingEnergyManagement = value; }

  /// External equipment energy use (W).
  [[nodiscard]] double externalEquipment() const noexcept { return m_externalEquipment; }
  void setExternalEquipment(double externalEquipment) { m_externalEquipment = externalEquipment; }

  // Unused properties preserved for interface compatibility
  [[nodiscard]] double electricAppliancePowerFixedOccupied() const noexcept {
    return m_electricAppliancePowerFixedOccupied;
  }
  void setElectricAppliancePowerFixedOccupied(double value) noexcept {
    m_electricAppliancePowerFixedOccupied = value;
  }

  [[nodiscard]] double electricAppliancePowerFixedUnoccupied() const noexcept {
    return m_electricAppliancePowerFixedUnoccupied;
  }
  void setElectricAppliancePowerFixedUnoccupied(double value) noexcept {
    m_electricAppliancePowerFixedUnoccupied = value;
  }

  [[nodiscard]] double gasAppliancePowerFixedOccupied() const noexcept {
    return m_gasAppliancePowerFixedOccupied;
  }
  void setGasAppliancePowerFixedOccupied(double value) { m_gasAppliancePowerFixedOccupied = value; }

  [[nodiscard]] double gasAppliancePowerFixedUnoccupied() const noexcept {
    return m_gasAppliancePowerFixedUnoccupied;
  }
  void setGasAppliancePowerFixedUnoccupied(double value) noexcept {
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
