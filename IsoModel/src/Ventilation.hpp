#pragma once
#include "Constants.hpp"
#include "ISOModelAPI.hpp"

namespace openstudio::isomodel {

class ISOMODEL_API Ventilation {
public:
  // Use compiler-generated default constructor/destructor
  Ventilation() = default;
  ~Ventilation() = default;

  /**
   * Ventilation intake rate occupied (L/s). Use 10 L/s/person as a default.
   */
  [[nodiscard]] double supplyRate() const noexcept { return m_supplyRate; }
  void setSupplyRate(double value) { m_supplyRate = value; }

  /**
   * Ventilation exhaust rate occupied (L/s).
   */
  [[nodiscard]] double supplyDifference() const noexcept { return m_supplyDifference; }
  void setSupplyDifference(double value) { m_supplyDifference = value; }

  /**
   * Efficiency of heat recovery (unitless. Use 0.0 for no heat recovery).
   */
  [[nodiscard]] double heatRecoveryEfficiency() const noexcept { return m_heatRecoveryEfficiency; }
  void setHeatRecoveryEfficiency(double value) { m_heatRecoveryEfficiency = value; }

  /**
   * Fraction of supply air recirculated (unitless).
   */
  [[nodiscard]] double exhaustAirRecirculated() const noexcept { return m_exhaustAirRecirculated; }
  void setExhaustAirRecirculated(double value) { m_exhaustAirRecirculated = value; }

  /**
   * Ventilation type (mechanical = 1.0, natural = 2.0, combined = 3.0).
   */
  [[nodiscard]] VentilationType ventType() const noexcept { return m_ventType; }
  void setVentType(VentilationType value) { m_ventType = value; }

  /**
   * Specific fan power (W/(L/s)).
   */
  [[nodiscard]] double fanPower() const noexcept { return m_fanPower; }
  void setFanPower(double value) { m_fanPower = value; }

  /**
   * Fan flow control factor (unitless). 1 = no control.
   */
  [[nodiscard]] double fanControlFactor() const noexcept { return m_fanControlFactor; }
  void setFanControlFactor(double value) { m_fanControlFactor = value; }

  /**
   * Ventilation preheat (C).
   */
  [[nodiscard]] double ventPreheatDegC() const noexcept { return m_ventPreheatDegC; }
  void setVentPreheatDegC(double ventPreheatDegC) { m_ventPreheatDegC = ventPreheatDegC; }

  /**
   * Air leakage at 50 Pa (air-changes/hr). See ISO 15242.
   */
  [[nodiscard]] double n50() const noexcept { return m_n50; }
  void setN50(double n50) { m_n50 = n50; }

  /**
   * Wind related, see ISO 15242.
   */
  [[nodiscard]] double hzone() const noexcept { return m_hzone; }
  void setHzone(double hzone) { m_hzone = hzone; }

  /**
   * Assumed floor exponent for infiltration pressure conversion.
   */
  [[nodiscard]] double p_exp() const noexcept { return m_p_exp; }
  void setP_exp(double p_exp) { m_p_exp = p_exp; }

  /**
   * Fraction that h_stack/zone height. Assume 0.7 as per en 15242.
   */
  [[nodiscard]] double zone_frac() const noexcept { return m_zone_frac; }
  void setZone_frac(double zone_frac) { m_zone_frac = zone_frac; }

  /**
   * Reset the pressure exponent to 0.667 for this part of the calc.
   */
  [[nodiscard]] double stack_exp() const noexcept { return m_stack_exp; }
  void setStack_exp(double stack_exp) { m_stack_exp = stack_exp; }

  /**
   * Stack coefficient.
   */
  [[nodiscard]] double stack_coeff() const noexcept { return m_stack_coeff; }
  void setStack_coeff(double stack_coeff) { m_stack_coeff = stack_coeff; }

  /**
   * Wind exponent.
   */
  [[nodiscard]] double wind_exp() const noexcept { return m_wind_exp; }
  void setWind_exp(double wind_exp) { m_wind_exp = wind_exp; }

  /**
   * Wind coefficient.
   */
  [[nodiscard]] double wind_coeff() const noexcept { return m_wind_coeff; }
  void setWind_coeff(double wind_coeff) { m_wind_coeff = wind_coeff; }

  /**
   * Conventional value for cp difference between windward and leeward sides.
   */
  [[nodiscard]] double dCp() const noexcept { return m_dCp; }
  void setDCp(double dCp) { m_dCp = dCp; }

  /**
   * Vent_rate_flag set to 0 for constant ventilation, 1 if vent off in
   * unoccupied times.
   */
  [[nodiscard]] int vent_rate_flag() const noexcept { return m_vent_rate_flag; }
  void setVent_rate_flag(int vent_rate_flag) { m_vent_rate_flag = vent_rate_flag; }

  /**
   * Overall heat transfer coefficient by ventilation as per ISO 13790 9.3.
   */
  [[nodiscard]] double H_ve() const noexcept { return m_H_ve; }
  void setH_ve(double H_ve) { m_H_ve = H_ve; }

  [[nodiscard]] double infiltrationRateUnoccupied() const noexcept {
    return m_infiltrationRateUnoccupied;
  }
  void setInfiltrationRateUnoccupied(double infiltrationRateUnoccupied) noexcept {
    m_infiltrationRateUnoccupied = infiltrationRateUnoccupied;
  }

  [[nodiscard]] double ventilationExhaustRateUnoccupied() const noexcept {
    return m_ventilationExhaustRateUnoccupied;
  }
  void setVentilationExhaustRateUnoccupied(double ventilationExhaustRateUnoccupied) noexcept {
    m_ventilationExhaustRateUnoccupied = ventilationExhaustRateUnoccupied;
  }

  [[nodiscard]] double ventilationIntakeRateUnoccupied() const noexcept {
    return m_ventilationIntakeRateUnoccupied;
  }
  void setVentilationIntakeRateUnoccupied(double ventilationIntakeRateUnoccupied) noexcept {
    m_ventilationIntakeRateUnoccupied = ventilationIntakeRateUnoccupied;
  }

private:
  // In-class initialization
  double m_supplyRate = 0.0;
  double m_supplyDifference = 0.0;
  double m_heatRecoveryEfficiency = 0.0;
  double m_exhaustAirRecirculated = 0.0;
  VentilationType m_ventType = VentilationType::Unspecified;
  double m_fanPower = 0.0;
  double m_fanControlFactor = 1.0; // Default: no control

  // Default values preserved from original
  double m_ventPreheatDegC = -50.0;
  double m_n50 = 2.0;
  double m_hzone = 39.0;
  double m_p_exp = 0.65;
  double m_zone_frac = 0.7;
  double m_stack_exp = 0.667;
  double m_stack_coeff = 0.0146;
  double m_wind_exp = 0.667;
  double m_wind_coeff = 0.0769;
  double m_dCp = 0.75;
  int m_vent_rate_flag = 1;
  double m_H_ve = 0.0;

  double m_infiltrationRateUnoccupied = 0.0;
  double m_ventilationExhaustRateUnoccupied = 0.0;
  double m_ventilationIntakeRateUnoccupied = 0.0;
};

} // namespace openstudio::isomodel
