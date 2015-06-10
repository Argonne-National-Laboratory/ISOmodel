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

  /**
  * Flag to indicate if cooling is delivered by air or not, and thus, 
  * whether or not fan power for cooling should be calculated.
  */
  bool forcedAirCooling() const {
    return m_forcedAirCooling;
  }

  void setForcedAirCooling(bool forcedAirCooling) {
    m_forcedAirCooling = forcedAirCooling;
  }

  /**
  * Flag to signify if we have cooling and controls turned on or off. 
  * E.g., might be off for school in summer.
  */
  double T_cl_ctrl_flag() const {
    return m_T_cl_ctrl_flag;
  }

  void setT_cl_ctrl_flag(double T_cl_ctrl_flag) {
    m_T_cl_ctrl_flag = T_cl_ctrl_flag;
  }

  /**
  * Cooling temperature difference between room air and supply air (C).
  */
  double dT_supp_cl() const {
    return m_dT_supp_cl;
  }

  void setDT_supp_cl(double dT_supp_cl) {
    m_dT_supp_cl = dT_supp_cl;
  }

  /**
  * Building connected to district cooling (DC) (0=no, 1=yes).
  */
  double DC_YesNo() const {
    return m_DC_YesNo;
  }

  void setDC_YesNo(double DC_YesNo) {
    m_DC_YesNo = DC_YesNo;
  }

  /**
  * Efficiency of DC network. Typical value 0l75-0l9 EN 15316-4-5
  */
  double eta_DC_network() const {
    return m_eta_DC_network;
  }

  void setEta_DC_network(double eta_DC_network) {
    m_eta_DC_network = eta_DC_network;
  }

  /**
  * COP of DC electric chillers.
  */
  double eta_DC_COP() const {
    return m_eta_DC_COP;
  }

  void setEta_DC_COP(double eta_DC_COP) {
    m_eta_DC_COP = eta_DC_COP;
  }

  /**
  * Fraction of DC chillers that are absorption.
  */
  double eta_DC_frac_abs() const {
    return m_eta_DC_frac_abs;
  }

  void setEta_DC_frac_abs(double eta_DC_frac_abs) {
    m_eta_DC_frac_abs = eta_DC_frac_abs;
  }

  /**
  * COP of DC absorption chillers.
  */
  double eta_DC_COP_abs() const {
    return m_eta_DC_COP_abs;
  }

  void setEta_DC_COP_abs(double eta_DC_COP_abs) {
    m_eta_DC_COP_abs = eta_DC_COP_abs;
  }

  /**
  * Fraction of free heat source to absorption DC chillers (0 to 1).
  */
  double frac_DC_free() const {
    return m_frac_DC_free;
  }

  void setFrac_DC_free(double frac_DC_free) {
    m_frac_DC_free = frac_DC_free;
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

private:
  double m_temperatureSetPointOccupied;
  double m_temperatureSetPointUnoccupied;
  double m_cop;
  double m_partialLoadValue;
  double m_hvacLossFactor;
  double m_pumpControlReduction;
  // Members with default values:
  bool m_forcedAirCooling = true;
  double m_T_cl_ctrl_flag = 1;
  double m_dT_supp_cl = 7.0;
  double m_DC_YesNo = 0;
  double m_eta_DC_network = 0.9;
  double m_eta_DC_COP = 5.5;
  double m_eta_DC_frac_abs = 0;
  double m_eta_DC_COP_abs = 1;
  double m_frac_DC_free = 0;
  double m_E_pumps = 0.25;
};

} // isomodel
} // openstudio
#endif // ISOMODEL_COOLING_HPP
