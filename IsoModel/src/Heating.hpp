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

  /**
  * Heating temperature difference between room air and supply air (C).
  */
  double dT_supp_ht() const {
    return m_dT_supp_ht;
  }

  void setDT_supp_ht(double dT_supp_ht) {
    m_dT_supp_ht = dT_supp_ht;
  }

  /**
  * Flag to indicate if heating is delivered by air or not, and thus, 
  * whether or not fan power for heating should be calculated.
  */
  bool forcedAirHeating() const {
    return m_forcedAirHeating;
  }

  void setForcedAirHeating(bool forcedAirHeating) {
    m_forcedAirHeating = forcedAirHeating;
  }

  /**
  * Specific power of systems pumps and control systems (W/m2).
  */
  double E_pumps() const {
    return m_E_pumps;
  }

  void setE_pumps(double E_pumps) {
    m_E_pumps = E_pumps;
  }

  /**
  * Flag to signify if we have heating and controls turned on or off. 
  * E.g., might be off for school in summer.
  */
  double T_ht_ctrl_flag() const {
    return m_T_ht_ctrl_flag;
  }

  void setT_ht_ctrl_flag(double T_ht_ctrl_flag) {
    m_T_ht_ctrl_flag = T_ht_ctrl_flag;
  }

  /**
  * Reference dimensionless parameter. (Used to set a_H, the building heating dimensionless constant).
  */
  double a_H0() const {
    return m_a_H0;
  }

  void setA_H0(double a_H0) {
    m_a_H0 = a_H0;
  }

  /**
  * Reference time constant. (Used to set a_H, the building heating dimensionless constant).
  */
  double tau_H0() const {
    return m_tau_H0;
  }

  void setTau_H0(double tau_H0) {
    m_tau_H0 = tau_H0;
  }

  /**
  * Building connected to District Heating (DH) (0=no, 1=yes.  Assume DH is powered by natural gas).
  */
  double DH_YesNo() const {
    return m_DH_YesNo;
  }

  void setDH_YesNo(double DH_YesNo) {
    m_DH_YesNo = DH_YesNo;
  }

  /**
  * Efficiency of DH network. Typical value 0l75-0l9 EN 15316-4-5
  */
  double eta_DH_network() const {
    return m_eta_DH_network;
  }

  void setEta_DH_network(double eta_DH_network) {
    m_eta_DH_network = eta_DH_network;
  }

  /**
  * Efficiency of DH system.
  */
  double eta_DH_sys() const {
    return m_eta_DH_sys;
  }

  void setEta_DH_sys(double eta_DH_sys) {
    m_eta_DH_sys = eta_DH_sys;
  }

  /**
  * Fraction of free heat source to DH (0 to 1).
  */
  double frac_DH_free() const {
    return m_frac_DH_free;
  }

  void setFrac_DH_free(double frac_DH_free) {
    m_frac_DH_free = frac_DH_free;
  }

  /**
  * Water temp set point (C).
  */
  double dhw_tset() const {
    return m_dhw_tset;
  }

  void setDhw_tset(double dhw_tset) {
    m_dhw_tset = dhw_tset;
  }

  /**
  * Water initial temp (C).
  */
  double dhw_tsupply() const {
    return m_dhw_tsupply;
  }

  void setDhw_tsupply(double dhw_tsupply) {
    m_dhw_tsupply = dhw_tsupply;
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
  // Members with default values:
  double m_dT_supp_ht = 7.0;
  bool m_forcedAirHeating = true;
  // Pumps:
  double m_E_pumps = 0.25;
  // Interior temp constants.
  double m_T_ht_ctrl_flag = 1;
  double m_a_H0 = 1;
  double m_tau_H0 = 15;
  double m_DH_YesNo = 0;
  double m_eta_DH_network = 0.9;
  double m_eta_DH_sys = 0.87;
  double m_frac_DH_free = 0.000;
  // Heated water constants
  double m_dhw_tset = 60;
  double m_dhw_tsupply = 20;
};

} // isomodel
} // openstudio
#endif // ISOMODEL_HEATING_HPP

