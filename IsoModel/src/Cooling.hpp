#ifndef ISOMODEL_COOLING_HPP
#define ISOMODEL_COOLING_HPP

#include "ISOModelAPI.hpp"

namespace openstudio::isomodel {

class ISOMODEL_API Cooling
{
public:
  Cooling() = default;
  ~Cooling() = default;

  double temperatureSetPointOccupied() const { return m_temperatureSetPointOccupied; }
  void setTemperatureSetPointOccupied(double value) { m_temperatureSetPointOccupied = value; }

  double temperatureSetPointUnoccupied() const { return m_temperatureSetPointUnoccupied; }
  void setTemperatureSetPointUnoccupied(double value) { m_temperatureSetPointUnoccupied = value; }

  double cop() const { return m_cop; }
  void setCop(double value) { m_cop = value; }

  double partialLoadValue() const { return m_partialLoadValue; }
  void setPartialLoadValue(double value) { m_partialLoadValue = value; }

  double hvacLossFactor() const { return m_hvacLossFactor; }
  void setHvacLossFactor(double value) { m_hvacLossFactor = value; }

  double pumpControlReduction() const { return m_pumpControlReduction; }
  void setPumpControlReduction(double value) { m_pumpControlReduction = value; }

  bool forcedAirCooling() const { return m_forcedAirCooling; }
  void setForcedAirCooling(bool value) { m_forcedAirCooling = value; }

  double T_cl_ctrl_flag() const { return m_T_cl_ctrl_flag; }
  void setT_cl_ctrl_flag(double value) { m_T_cl_ctrl_flag = value; }

  double dT_supp_cl() const { return m_dT_supp_cl; }
  void setDT_supp_cl(double value) { m_dT_supp_cl = value; }

  double DC_YesNo() const { return m_DC_YesNo; }
  void setDC_YesNo(double value) { m_DC_YesNo = value; }

  double eta_DC_network() const { return m_eta_DC_network; }
  void setEta_DC_network(double value) { m_eta_DC_network = value; }

  double eta_DC_COP() const { return m_eta_DC_COP; }
  void setEta_DC_COP(double value) { m_eta_DC_COP = value; }

  double eta_DC_frac_abs() const { return m_eta_DC_frac_abs; }
  void setEta_DC_frac_abs(double value) { m_eta_DC_frac_abs = value; }

  double eta_DC_COP_abs() const { return m_eta_DC_COP_abs; }
  void setEta_DC_COP_abs(double value) { m_eta_DC_COP_abs = value; }

  double frac_DC_free() const { return m_frac_DC_free; }
  void setFrac_DC_free(double value) { m_frac_DC_free = value; }

  double E_pumps() const { return m_E_pumps; }
  void setE_pumps(double value) { m_E_pumps = value; }

private:
  double m_temperatureSetPointOccupied = 0.0;
  double m_temperatureSetPointUnoccupied = 0.0;
  double m_cop = 0.0;
  double m_partialLoadValue = 0.0;
  double m_hvacLossFactor = 0.0;
  double m_pumpControlReduction = 0.0;
  
  // Default values moved from old initialization
  bool m_forcedAirCooling = true;
  double m_T_cl_ctrl_flag = 1.0;
  double m_dT_supp_cl = 7.0;
  double m_DC_YesNo = 0.0;
  double m_eta_DC_network = 0.9;
  double m_eta_DC_COP = 5.5;
  double m_eta_DC_frac_abs = 0.0;
  double m_eta_DC_COP_abs = 1.0;
  double m_frac_DC_free = 0.0;
  double m_E_pumps = 0.25;
};

} // namespace openstudio::isomodel
#endif // ISOMODEL_COOLING_HPP