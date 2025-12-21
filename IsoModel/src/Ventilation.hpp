/**********************************************************************
 * Copyright (c) 2008-2013, Alliance for Sustainable Energy.
 * All rights reserved.
 **********************************************************************/
#ifndef ISOMODEL_VENTILATION_HPP
#define ISOMODEL_VENTILATION_HPP

#include "ISOModelAPI.hpp"

namespace openstudio::isomodel {

class ISOMODEL_API Ventilation
{
public:
  // Use compiler-generated default constructor/destructor
  Ventilation() = default;
  ~Ventilation() = default;

  /**
  * Ventilation intake rate occupied (L/s). Use 10 L/s/person as a default.
  */
  double supplyRate() const { return m_supplyRate; }
  void setSupplyRate(double value) { m_supplyRate = value; }

  /**
  * Ventilation exhaust rate occupied (L/s).
  */
  double supplyDifference() const { return m_supplyDifference; }
  void setSupplyDifference(double value) { m_supplyDifference = value; }

  /**
  * Efficiency of heat recovery (unitless. Use 0.0 for no heat recovery).
  */
  double heatRecoveryEfficiency() const { return m_heatRecoveryEfficiency; }
  void setHeatRecoveryEfficiency(double value) { m_heatRecoveryEfficiency = value; }

  /**
  * Fraction of supply air recirculated (unitless).
  */
  double exhaustAirRecirculated() const { return m_exhaustAirRecirculated; }
  void setExhaustAirRecirculated(double value) { m_exhaustAirRecirculated = value; }

  /**
  * Ventilation type (mechanical = 1.0, natural = 2.0, combined = 3.0).
  */
  double ventType() const { return m_ventType; }
  void setVentType(double value) { m_ventType = value; }

  /**
  * Specific fan power (W/(L/s)).
  */
  double fanPower() const { return m_fanPower; }
  void setFanPower(double value) { m_fanPower = value; }

  /**
  * Fan flow control factor (unitless). 1 = no control.
  */
  double fanControlFactor() const { return m_fanControlFactor; }
  void setFanControlFactor(double value) { m_fanControlFactor = value; }

  /**
  * Ventilation preheat (C).
  */
  double ventPreheatDegC() const { return m_ventPreheatDegC; }
  void setVentPreheatDegC(double ventPreheatDegC) { m_ventPreheatDegC = ventPreheatDegC; }

  /**
  * Air leakage at 50 Pa (air-changes/hr). See ISO 15242.
  */
  double n50() const { return m_n50; }
  void setN50(double n50) { m_n50 = n50; }

  /**
  * Wind related, see ISO 15242.
  */
  double hzone() const { return m_hzone; }
  void setHzone(double hzone) { m_hzone = hzone; }

  /**
  * Assumed floor exponent for infiltration pressure conversion.
  */
  double p_exp() const { return m_p_exp; }
  void setP_exp(double p_exp) { m_p_exp = p_exp; }

  /**
  * Fraction that h_stack/zone height. Assume 0.7 as per en 15242.
  */
  double zone_frac() const { return m_zone_frac; }
  void setZone_frac(double zone_frac) { m_zone_frac = zone_frac; }

  /**
  * Reset the pressure exponent to 0.667 for this part of the calc.
  */
  double stack_exp() const { return m_stack_exp; }
  void setStack_exp(double stack_exp) { m_stack_exp = stack_exp; }

  /**
  * Stack coefficient.
  */
  double stack_coeff() const { return m_stack_coeff; }
  void setStack_coeff(double stack_coeff) { m_stack_coeff = stack_coeff; }

  /**
  * Wind exponent.
  */
  double wind_exp() const { return m_wind_exp; }
  void setWind_exp(double wind_exp) { m_wind_exp = wind_exp; }

  /**
  * Wind coefficient.
  */
  double wind_coeff() const { return m_wind_coeff; }
  void setWind_coeff(double wind_coeff) { m_wind_coeff = wind_coeff; }

  /**
  * Conventional value for cp difference between windward and leeward sides.
  */
  double dCp() const { return m_dCp; }
  void setDCp(double dCp) { m_dCp = dCp; }

  /**
  * Vent_rate_flag set to 0 for constant ventilation, 1 if vent off in unoccupied times.
  */
  int vent_rate_flag() const { return m_vent_rate_flag; }
  void setVent_rate_flag(int vent_rate_flag) { m_vent_rate_flag = vent_rate_flag; }

  /**
  * Overall heat transfer coefficient by ventilation as per ISO 13790 9.3.
  */
  double H_ve() const { return m_H_ve; }
  void setH_ve(double H_ve) { m_H_ve = H_ve; }

  double infiltrationRateUnoccupied() const { return m_infiltrationRateUnoccupied; }
  void setInfiltrationRateUnoccupied(double infiltrationRateUnoccupied) { m_infiltrationRateUnoccupied = infiltrationRateUnoccupied; }

  double ventilationExhaustRateUnoccupied() const { return m_ventilationExhaustRateUnoccupied; }
  void setVentilationExhaustRateUnoccupied(double ventilationExhaustRateUnoccupied) { m_ventilationExhaustRateUnoccupied = ventilationExhaustRateUnoccupied; }

  double ventilationIntakeRateUnoccupied() const { return m_ventilationIntakeRateUnoccupied; }
  void setVentilationIntakeRateUnoccupied(double ventilationIntakeRateUnoccupied) { m_ventilationIntakeRateUnoccupied = ventilationIntakeRateUnoccupied; }

private:
  // In-class initialization
  double m_supplyRate = 0.0;
  double m_supplyDifference = 0.0;
  double m_heatRecoveryEfficiency = 0.0;
  double m_exhaustAirRecirculated = 0.0;
  double m_ventType = 0.0;
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
#endif // ISOMODEL_VENTILATION_HPP