#ifndef ISOMODEL_HEATING_HPP
#define ISOMODEL_HEATING_HPP

#include "ISOModelAPI.hpp"

namespace openstudio::isomodel {

class ISOMODEL_API Heating
{
public:
  Heating() = default;
  ~Heating() = default;

  double temperatureSetPointOccupied() const { return m_temperatureSetPointOccupied; }
  void setTemperatureSetPointOccupied(double value) { m_temperatureSetPointOccupied = value; }

  double temperatureSetPointUnoccupied() const { return m_temperatureSetPointUnoccupied; }
  void setTemperatureSetPointUnoccupied(double value) { m_temperatureSetPointUnoccupied = value; }

  double hvacLossFactor() const { return m_hvacLossFactor; }
  void setHvacLossFactor(double value) { m_hvacLossFactor = value; }

  double hotcoldWasteFactor() const { return m_hotcoldWasteFactor; }
  void setHotcoldWasteFactor(double value) { m_hotcoldWasteFactor = value; }

  double efficiency() const { return m_efficiency; }
  void setEfficiency(double value) { m_efficiency = value; }

  double energyType() const { return m_energyType; }
  void setEnergyType(double value) { m_energyType = value; }

  double pumpControlReduction() const { return m_pumpControlReduction; }
  void setPumpControlReduction(double value) { m_pumpControlReduction = value; }

  double hotWaterDemand() const { return m_hotWaterDemand; }
  void setHotWaterDemand(double value) { m_hotWaterDemand = value; }

  double hotWaterDistributionEfficiency() const { return m_hotWaterDistributionEfficiency; }
  void setHotWaterDistributionEfficiency(double value) { m_hotWaterDistributionEfficiency = value; }

  double hotWaterSystemEfficiency() const { return m_hotWaterSystemEfficiency; }
  void setHotWaterSystemEfficiency(double value) { m_hotWaterSystemEfficiency = value; }

  double hotWaterEnergyType() const { return m_hotWaterEnergyType; }
  void setHotWaterEnergyType(double value) { m_hotWaterEnergyType = value; }

  double dT_supp_ht() const { return m_dT_supp_ht; }
  void setDT_supp_ht(double value) { m_dT_supp_ht = value; }

  bool forcedAirHeating() const { return m_forcedAirHeating; }
  void setForcedAirHeating(bool value) { m_forcedAirHeating = value; }

  double E_pumps() const { return m_E_pumps; }
  void setE_pumps(double value) { m_E_pumps = value; }

  double T_ht_ctrl_flag() const { return m_T_ht_ctrl_flag; }
  void setT_ht_ctrl_flag(double value) { m_T_ht_ctrl_flag = value; }

  double a_H0() const { return m_a_H0; }
  void setA_H0(double value) { m_a_H0 = value; }

  double tau_H0() const { return m_tau_H0; }
  void setTau_H0(double value) { m_tau_H0 = value; }

  double DH_YesNo() const { return m_DH_YesNo; }
  void setDH_YesNo(double value) { m_DH_YesNo = value; }

  double eta_DH_network() const { return m_eta_DH_network; }
  void setEta_DH_network(double value) { m_eta_DH_network = value; }

  double eta_DH_sys() const { return m_eta_DH_sys; }
  void setEta_DH_sys(double value) { m_eta_DH_sys = value; }

  double frac_DH_free() const { return m_frac_DH_free; }
  void setFrac_DH_free(double value) { m_frac_DH_free = value; }

  double dhw_tset() const { return m_dhw_tset; }
  void setDhw_tset(double value) { m_dhw_tset = value; }

  double dhw_tsupply() const { return m_dhw_tsupply; }
  void setDhw_tsupply(double value) { m_dhw_tsupply = value; }

private:
  double m_temperatureSetPointOccupied = 0.0;
  double m_temperatureSetPointUnoccupied = 0.0;
  double m_hvacLossFactor = 0.0;
  double m_efficiency = 0.0;
  double m_energyType = 0.0;
  double m_pumpControlReduction = 0.0;
  double m_hotWaterDemand = 0.0;
  double m_hotWaterDistributionEfficiency = 0.0;
  double m_hotWaterSystemEfficiency = 0.0;
  double m_hotWaterEnergyType = 0.0;
  double m_hotcoldWasteFactor = 0.0;
  
  // Default Initializers
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
#endif // ISOMODEL_HEATING_HPP