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
#ifndef ISOMODEL_VENTILATION_HPP
#define ISOMODEL_VENTILATION_HPP

namespace openstudio {
namespace isomodel {
class Ventilation
{
public:
  Ventilation(void);
  ~Ventilation(void);

  /**
  * Ventilation intake rate occupied (L/s). Use 10 L/s/person as a default.
  */
  double supplyRate() const {
    return m_supplyRate;
  }

  void setSupplyRate(double value) {
    m_supplyRate = value;
  }

  /**
  * Ventilation exhaust rate occupied (L/s).
  */
  double supplyDifference() const {
    return m_supplyDifference;
  }

  void setSupplyDifference(double value) {
    m_supplyDifference = value;
  }

  /**
  * Efficiency of heat recovery (unitless. Use 0.0 for no heat recovery).
  */
  double heatRecoveryEfficiency() const {
    return m_heatRecoveryEfficiency;
  }

  void setHeatRecoveryEfficiency(double value) {
    m_heatRecoveryEfficiency = value;
  }

  /**
  * Fraction of supply air recirculated (unitless).
  */
  double exhaustAirRecirculated() const {
    return m_exhaustAirRecirculated;
  }

  void setExhaustAirRecirculated(double value) {
    m_exhaustAirRecirculated = value;
  }

  /**
  * Ventilation type (mechanical = 1.0, natural = 2.0, combined = 3.0).
  * XXX TODO: change this to an enum.
  */
  double ventType() const {
    return m_ventType;
  }

  void setVentType(double value) {
    m_ventType = value;
  }

  /**
  * Specific fan power (W/(L/s)).
  */
  double fanPower() const {
    return m_fanPower;
  }

  void setFanPower(double value) {
    m_fanPower = value;
  }

  /**
  * Fan flow control factor (unitless). This is the energy reduction from fan control measures.
  * 1 = no control, 0.75 = inlet blade adjuct, 0.65 = variable speed see NEN 2916 7.3.3.4.
  */
  double fanControlFactor() const {
    return m_fanControlFactor;
  }

  void setFanControlFactor(double value) {
    m_fanControlFactor = value;
  }

  /**
  * Ventilation preheat (C).
  */
  double ventPreheatDegC() const {
    return m_ventPreheatDegC;
  }

  void setVentPreheatDegC(double ventPreheatDegC) {
    m_ventPreheatDegC = ventPreheatDegC;
  }

  /**
  * Air leakage at 50 Pa (air-changes/hr). See ISO 15242.
  */
  double n50() const {
    return m_n50;
  }

  void setN50(double n50) {
    m_n50 = n50;
  }

  /**
  * XXX: What is this variable? Wind related, see ISO 15242.
  */
  double hzone() const {
    return m_hzone;
  }

  void setHzone(double hzone) {
    m_hzone = hzone;
  }

  // Ventillation constants.
  // infiltration data from
  // Tamura, (1976), Studies on exterior wall air tightness and air infiltration of tall buildings, ASHRAE Transactions, 82(1), 122-134.
  // Orm (1998), AIVC TN44: Numerical data for air infiltration and natural ventilation calculations, Air Infiltration and Ventilation Centre.
  // Emmerich, (2005), Investigation of the Impact of Commercial Building Envelope Airtightness on HVAC Energy Use.
  // create a different table for different building types
  // n_highrise_inf_table=[4 6 10 15 20];  % infiltration table for high rise buildings as per Tamura, Orm and Emmerich

  /**
  * Assumed floor exponent for infiltration pressure conversion.
  * Tamura, (1976), Studies on exterior wall air tightness and air infiltration of tall buildings, ASHRAE Transactions, 82(1), 122-134.
  * Orm (1998), AIVC TN44: Numerical data for air infiltration and natural ventilation calculations, Air Infiltration and Ventilation Centre.
  * Emmerich, (2005), Investigation of the Impact of Commercial Building Envelope Airtightness on HVAC Energy Use.
  */
  double p_exp() const {
    return m_p_exp;
  }

  void setP_exp(double p_exp) {
    m_p_exp = p_exp;
  }

  /**
  * Fraction that h_stack/zone height. Assume 0.7 as per en 15242.
  * Tamura, (1976), Studies on exterior wall air tightness and air infiltration of tall buildings, ASHRAE Transactions, 82(1), 122-134.
  * Orm (1998), AIVC TN44: Numerical data for air infiltration and natural ventilation calculations, Air Infiltration and Ventilation Centre.
  * Emmerich, (2005), Investigation of the Impact of Commercial Building Envelope Airtightness on HVAC Energy Use.
  */
  double zone_frac() const {
    return m_zone_frac;
  }

  void setZone_frac(double zone_frac) {
    m_zone_frac = zone_frac;
  }

  /**
  * Reset the pressure exponent to 0.667 for this part of the calc.
  * Tamura, (1976), Studies on exterior wall air tightness and air infiltration of tall buildings, ASHRAE Transactions, 82(1), 122-134.
  * Orm (1998), AIVC TN44: Numerical data for air infiltration and natural ventilation calculations, Air Infiltration and Ventilation Centre.
  * Emmerich, (2005), Investigation of the Impact of Commercial Building Envelope Airtightness on HVAC Energy Use.
  */
  double stack_exp() const {
    return m_stack_exp;
  }

  void setStack_exp(double stack_exp) {
    m_stack_exp = stack_exp;
  }

  /**
  * Stack coefficient.
  * Tamura, (1976), Studies on exterior wall air tightness and air infiltration of tall buildings, ASHRAE Transactions, 82(1), 122-134.
  * Orm (1998), AIVC TN44: Numerical data for air infiltration and natural ventilation calculations, Air Infiltration and Ventilation Centre.
  * Emmerich, (2005), Investigation of the Impact of Commercial Building Envelope Airtightness on HVAC Energy Use.
  */
  double stack_coeff() const {
    return m_stack_coeff;
  }

  void setStack_coeff(double stack_coeff) {
    m_stack_coeff = stack_coeff;
  }

  /**
  * Wind exponent.
  * Tamura, (1976), Studies on exterior wall air tightness and air infiltration of tall buildings, ASHRAE Transactions, 82(1), 122-134.
  * Orm (1998), AIVC TN44: Numerical data for air infiltration and natural ventilation calculations, Air Infiltration and Ventilation Centre.
  * Emmerich, (2005), Investigation of the Impact of Commercial Building Envelope Airtightness on HVAC Energy Use.
  */
  double wind_exp() const {
    return m_wind_exp;
  }

  void setWind_exp(double wind_exp) {
    m_wind_exp = wind_exp;
  }

  /**
  * Wind coefficient.
  * Tamura, (1976), Studies on exterior wall air tightness and air infiltration of tall buildings, ASHRAE Transactions, 82(1), 122-134.
  * Orm (1998), AIVC TN44: Numerical data for air infiltration and natural ventilation calculations, Air Infiltration and Ventilation Centre.
  * Emmerich, (2005), Investigation of the Impact of Commercial Building Envelope Airtightness on HVAC Energy Use.
  */
  double wind_coeff() const {
    return m_wind_coeff;
  }

  void setWind_coeff(double wind_coeff) {
    m_wind_coeff = wind_coeff;
  }

  /**
  * Conventional value for cp difference between windward and leeward sides for low rise buildings as per 15242.
  * Tamura, (1976), Studies on exterior wall air tightness and air infiltration of tall buildings, ASHRAE Transactions, 82(1), 122-134.
  * Orm (1998), AIVC TN44: Numerical data for air infiltration and natural ventilation calculations, Air Infiltration and Ventilation Centre.
  * Emmerich, (2005), Investigation of the Impact of Commercial Building Envelope Airtightness on HVAC Energy Use.
  */
  double dCp() const {
    return m_dCp;
  }

  void setDCp(double dCp) {
    m_dCp = dCp;
  }

  /**
  * Vent_rate_flag set to 0 for constant ventilation, 1 if vent off in unoccupied times or 2 if vent rate dropped proportional to population.
  */
  int vent_rate_flag() const {
    return m_vent_rate_flag;
  }

  void setVent_rate_flag(int vent_rate_flag) {
    m_vent_rate_flag = vent_rate_flag;
  }

  /**
  * Heat capacity of air per unit volume, rho*Cp (W/(m3*K)).
  * TODO: Combine this with heating/cooling rhoC_air, which is in MJ/m3/K.
  */
  double rhoc_air() const {
    return m_rhoc_air;
  }

  void setRhoc_air(double rhoc_air) {
    m_rhoc_air = rhoc_air;
  }

  /**
  * Overall heat transfer coefficient by ventilation as per ISO 13790 9.3.
  */
  double H_ve() const {
    return m_H_ve;
  }

  void setH_ve(double H_ve) {
    m_H_ve = H_ve;
  }

private:
  double m_supplyRate;
  double m_supplyDifference;
  double m_heatRecoveryEfficiency;
  double m_exhaustAirRecirculated;
  double m_ventType;
  double m_fanPower;
  double m_fanControlFactor;
  // Members with default values.
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
  double m_rhoc_air = 1200.0; // XXX: This value is also used in heating and cooling. Consilidate these?
  double m_H_ve = 0.0;
  
};
} // isomodel
} // openstudio
#endif // ISOMODEL_VENTILATION_HPP
