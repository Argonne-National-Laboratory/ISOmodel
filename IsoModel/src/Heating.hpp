/// @file Heating.hpp
/// @brief Heating system properties and fuel type parameters.
///
/// Stores heating efficiency, temperature setpoints (occupied and unoccupied),
/// HVAC loss and waste factors, pump power, hot water demand, and district
/// heating parameters. Includes fuel type selection for primary energy
/// calculations per ISO 13790.
///
/// @author Brian Craig
/// @author Nick Collier
/// @author Brendan Albano
/// @author Ralph Muehleisen
/// @date 2013-11-05
/// @copyright Copyright Argonne National Laboratory
#pragma once
#include "Constants.hpp"
#include "ISOModelAPI.hpp"

namespace openstudio::isomodel {

class ISOMODEL_API Heating {
public:
  // Use compiler-generated default constructor/destructor
  Heating() = default;
  ~Heating() = default;

  [[nodiscard]] double temperatureSetPointOccupied() const noexcept {
    return m_temperatureSetPointOccupied;
  }
  void setTemperatureSetPointOccupied(double value) { m_temperatureSetPointOccupied = value; }

  [[nodiscard]] double temperatureSetPointUnoccupied() const noexcept {
    return m_temperatureSetPointUnoccupied;
  }
  void setTemperatureSetPointUnoccupied(double value) { m_temperatureSetPointUnoccupied = value; }

  [[nodiscard]] double hvacLossFactor() const noexcept { return m_hvacLossFactor; }
  void setHvacLossFactor(double value) { m_hvacLossFactor = value; }

  [[nodiscard]] double hotcoldWasteFactor() const noexcept { return m_hotcoldWasteFactor; }
  void setHotcoldWasteFactor(double value) { m_hotcoldWasteFactor = value; }

  [[nodiscard]] double efficiency() const noexcept { return m_efficiency; }
  void setEfficiency(double value) { m_efficiency = value; }

  [[nodiscard]] FuelType energyType() const noexcept { return m_energyType; }
  void setEnergyType(FuelType value) { m_energyType = value; }

  [[nodiscard]] double pumpControlReduction() const noexcept { return m_pumpControlReduction; }
  void setPumpControlReduction(double value) { m_pumpControlReduction = value; }

  [[nodiscard]] double hotWaterDemand() const noexcept { return m_hotWaterDemand; }
  void setHotWaterDemand(double value) { m_hotWaterDemand = value; }

  [[nodiscard]] double hotWaterDistributionEfficiency() const noexcept {
    return m_hotWaterDistributionEfficiency;
  }
  void setHotWaterDistributionEfficiency(double value) { m_hotWaterDistributionEfficiency = value; }

  [[nodiscard]] double hotWaterSystemEfficiency() const noexcept {
    return m_hotWaterSystemEfficiency;
  }
  void setHotWaterSystemEfficiency(double value) { m_hotWaterSystemEfficiency = value; }

  [[nodiscard]] FuelType hotWaterEnergyType() const noexcept { return m_hotWaterEnergyType; }
  void setHotWaterEnergyType(FuelType value) { m_hotWaterEnergyType = value; }

  [[nodiscard]] double dT_supp_ht() const noexcept { return m_dT_supp_ht; }
  void setDT_supp_ht(double value) { m_dT_supp_ht = value; }

  [[nodiscard]] bool forcedAirHeating() const noexcept { return m_forcedAirHeating; }
  void setForcedAirHeating(bool value) { m_forcedAirHeating = value; }

  [[nodiscard]] double E_pumps() const noexcept { return m_E_pumps; }
  void setE_pumps(double value) { m_E_pumps = value; }

  [[nodiscard]] double T_ht_ctrl_flag() const noexcept { return m_T_ht_ctrl_flag; }
  void setT_ht_ctrl_flag(double value) { m_T_ht_ctrl_flag = value; }

  [[nodiscard]] double a_H0() const noexcept { return m_a_H0; }
  void setA_H0(double value) { m_a_H0 = value; }

  [[nodiscard]] double tau_H0() const noexcept { return m_tau_H0; }
  void setTau_H0(double value) { m_tau_H0 = value; }

  [[nodiscard]] double DH_YesNo() const noexcept { return m_DH_YesNo; }
  void setDH_YesNo(double value) { m_DH_YesNo = value; }

  [[nodiscard]] double eta_DH_network() const noexcept { return m_eta_DH_network; }
  void setEta_DH_network(double value) { m_eta_DH_network = value; }

  [[nodiscard]] double eta_DH_sys() const noexcept { return m_eta_DH_sys; }
  void setEta_DH_sys(double value) { m_eta_DH_sys = value; }

  [[nodiscard]] double frac_DH_free() const noexcept { return m_frac_DH_free; }
  void setFrac_DH_free(double value) { m_frac_DH_free = value; }

  [[nodiscard]] double dhw_tset() const noexcept { return m_dhw_tset; }
  void setDhw_tset(double value) { m_dhw_tset = value; }

  [[nodiscard]] double dhw_tsupply() const noexcept { return m_dhw_tsupply; }
  void setDhw_tsupply(double value) { m_dhw_tsupply = value; }

private:
  // In-class initialization ensures safer default state
  double m_temperatureSetPointOccupied = 0.0;
  double m_temperatureSetPointUnoccupied = 0.0;
  double m_hvacLossFactor = 0.0;
  double m_efficiency = 0.0;
  FuelType m_energyType = FuelType::Unspecified;
  double m_pumpControlReduction = 0.0;
  double m_hotWaterDemand = 0.0;
  double m_hotWaterDistributionEfficiency = 0.0;
  double m_hotWaterSystemEfficiency = 0.0;
  FuelType m_hotWaterEnergyType = FuelType::Unspecified;
  double m_hotcoldWasteFactor = 0.0;

  // Default values preserved
  bool m_forcedAirHeating = true;
  double m_dT_supp_ht = 7.0;
  double m_E_pumps = 0.25;
  double m_T_ht_ctrl_flag = 1.0;
  double m_a_H0 = 1.0;
  double m_tau_H0 = 15.0;
  double m_DH_YesNo = 0.0;
  double m_eta_DH_network = 0.9;
  double m_eta_DH_sys = 0.87;
  double m_frac_DH_free = 0.0;
  double m_dhw_tset = 60.0;
  double m_dhw_tsupply = 20.0;
};

} // namespace openstudio::isomodel
