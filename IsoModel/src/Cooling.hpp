/// @file Cooling.hpp
/// @brief Cooling system properties and HVAC distribution parameters.
///
/// Stores COP, partial load value, temperature setpoints (occupied and
/// unoccupied), HVAC loss factors, pump power, and district cooling
/// parameters. Properties map to ISO 13790 and EN 15243 cooling calculations.
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

class ISOMODEL_API Cooling {
public:
  // Use compiler-generated default constructor/destructor
  Cooling() = default;
  ~Cooling() = default;

  [[nodiscard]] double temperatureSetPointOccupied() const noexcept {
    return m_temperatureSetPointOccupied;
  }
  void setTemperatureSetPointOccupied(double value) { m_temperatureSetPointOccupied = value; }

  [[nodiscard]] double temperatureSetPointUnoccupied() const noexcept {
    return m_temperatureSetPointUnoccupied;
  }
  void setTemperatureSetPointUnoccupied(double value) { m_temperatureSetPointUnoccupied = value; }

  [[nodiscard]] double cop() const noexcept { return m_cop; }
  void setCop(double value) { m_cop = value; }

  [[nodiscard]] double partialLoadValue() const noexcept { return m_partialLoadValue; }
  void setPartialLoadValue(double value) { m_partialLoadValue = value; }

  [[nodiscard]] double hvacLossFactor() const noexcept { return m_hvacLossFactor; }
  void setHvacLossFactor(double value) { m_hvacLossFactor = value; }

  [[nodiscard]] double pumpControlReduction() const noexcept { return m_pumpControlReduction; }
  void setPumpControlReduction(double value) { m_pumpControlReduction = value; }

  [[nodiscard]] bool forcedAirCooling() const noexcept { return m_forcedAirCooling; }
  void setForcedAirCooling(bool value) { m_forcedAirCooling = value; }

  [[nodiscard]] double T_cl_ctrl_flag() const noexcept { return m_T_cl_ctrl_flag; }
  void setT_cl_ctrl_flag(double value) { m_T_cl_ctrl_flag = value; }

  [[nodiscard]] double dT_supp_cl() const noexcept { return m_dT_supp_cl; }
  void setDT_supp_cl(double value) { m_dT_supp_cl = value; }

  [[nodiscard]] double DC_YesNo() const noexcept { return m_DC_YesNo; }
  void setDC_YesNo(double value) { m_DC_YesNo = value; }

  [[nodiscard]] double eta_DC_network() const noexcept { return m_eta_DC_network; }
  void setEta_DC_network(double value) { m_eta_DC_network = value; }

  [[nodiscard]] double eta_DC_COP() const noexcept { return m_eta_DC_COP; }
  void setEta_DC_COP(double value) { m_eta_DC_COP = value; }

  [[nodiscard]] double eta_DC_frac_abs() const noexcept { return m_eta_DC_frac_abs; }
  void setEta_DC_frac_abs(double value) { m_eta_DC_frac_abs = value; }

  [[nodiscard]] double eta_DC_COP_abs() const noexcept { return m_eta_DC_COP_abs; }
  void setEta_DC_COP_abs(double value) { m_eta_DC_COP_abs = value; }

  [[nodiscard]] double frac_DC_free() const noexcept { return m_frac_DC_free; }
  void setFrac_DC_free(double value) { m_frac_DC_free = value; }

  [[nodiscard]] double E_pumps() const noexcept { return m_E_pumps; }
  void setE_pumps(double value) { m_E_pumps = value; }

private:
  // In-class initialization ensures safer default state
  double m_temperatureSetPointOccupied = 0.0;
  double m_temperatureSetPointUnoccupied = 0.0;
  double m_cop = 0.0;
  double m_partialLoadValue = 0.0;
  double m_hvacLossFactor = 0.0;
  double m_pumpControlReduction = 0.0;

  // Defaults preserved from original logic
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
