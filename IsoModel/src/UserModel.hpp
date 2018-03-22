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

#ifndef ISOMODEL_USERMODEL_HPP
#define ISOMODEL_USERMODEL_HPP

#include "ISOModelAPI.hpp"
#include "EpwData.hpp"
#include "MonthlyModel.hpp"
#include "HourlyModel.hpp"
#include "Properties.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>

namespace openstudio {

namespace isomodel {

class MonthlyModel;
class WeatherData;

const std::string GAS = "gas";
const std::string ELECTRIC = "electric";

const std::string MECHANICAL = "mechanical";
const std::string NATURAL = "natural";
const std::string COMBINED = "combined";

const std::string NONE = "none";
const std::string SIMPLE = "simple";
const std::string ADVANCED = "advanced";

struct ISOMODEL_API LatLon {

  double lat, lon;
  bool operator<(const LatLon& rhs) const;

};

class ISOMODEL_API UserModel
{
public:
  UserModel();
  virtual ~UserModel();

  /**
   * Loads an ISO model from the specified .ism file
   */
  void load(std::string buildingFile);
  void UserModel::loadJustBuilding(std::string buildingFile);
  void UserModel::loadJustWeather(std::string buildingFile);
  /**
  * Loads an ISO model file from the specified .ism file and defaults properties from the specified .ism.
  */
  void load(std::string buildingFile, std::string defaultsFile);

  /**
   * Loads the specified weather data from disk.
   * Exposed to allow for separate loading from Ruby Scripts
   * Call setWeatherFilePath(path) then loadWeather() to update
   * the UserModel with a new set of weather data
   */
  void loadWeather();

  /**
   * Loads the weather from the specified array of doubles.
   */
  void loadWeather(int block_size, double* weather_data);

  void loadAndSetWeather();

  /**
   * Generates a MonthlyModel from the properties of the UserModel.
   */
  MonthlyModel toMonthlyModel() const;

  /**
   * Generates an HourlyModel from the properties of the UserModel.
   */
  HourlyModel toHourlyModel() const;

  /**
   * Indicates whether or not the user model loaded in correctly
   * If either the ISO file or the Weather File cannot be found
   * valid will be false
   * userModel.load(<filename>)
   * if(userModel.valid()){
   *     userModel.toMonthlyModel().simulate();
   * }
   */
  bool valid() const  {
    return _valid;
  }

  // Validation
  void setValid(bool val)
  {
    _valid = val;
  }

  // ------------------------------------------------ //
  // Setters and getters for the isomodel properties. //
  // ------------------------------------------------ //

  /// Gets a EpwData property.
  const std::shared_ptr<EpwData> epwData() {
    return _edata;
  }

  /// Gets a WeatherData property.
  const std::shared_ptr<WeatherData> weatherData() {
    return _weather;
  }

  /// Gets a WeatherData property. Property name in .ism file: "weatherfilepath". Property is required.
  std::string weatherFilePath() const {
    return _weatherFilePath;
  }

  /// Sets a WeatherData property. Property name in .ism file: "weatherfilepath". Property is required.
  void setWeatherFilePath(std::string val) {
    _weatherFilePath = val;
  }

  /// Gets a Building property.
  double bemType() const {
    return building.buildingEnergyManagement();
  }

  /// Sets a Building property.
  void setBemType(std::string type) {
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);
    if (type == NONE)
      building.setBuildingEnergyManagement(1.0);
    else if (type == SIMPLE)
      building.setBuildingEnergyManagement(2.0);
    else if (type == ADVANCED)
      building.setBuildingEnergyManagement(3.0);
    else
      throw std::invalid_argument("bemType parameter must be one of 'none', 'simple', or 'advanced'");
  }

  /// Gets a Building property. Property name in .ism file: "infiltrationrateoccupied". Property is required.
  double buildingAirLeakage() const {
    return structure.infiltrationRate();
  }

  /// Gets a Building property. Property name in .ism file: "buildingheight". Property is required.
  double buildingHeight() const {
    return structure.buildingHeight();
  }

  /// Gets a Building property. Property name in .ism file: "occupancydayfirst". Property is required.
  double buildingOccupancyFrom() const {
    return pop.daysStart();
  }

  /// Gets a Building property. Property name in .ism file: "occupancydaylast". Property is required.
  double buildingOccupancyTo() const {
    return pop.daysEnd();
  }

  /// Gets a Building property. Property name in .ism file: "constantilluminationcontrolmultiplier". Property is required.
  double constantIlluminationControl() const {
    return building.constantIllumination();
  }

  /// Sets a Building property. Property name in .ism file: "constantilluminationcontrolmultiplier". Property is required.
  void setConstantIlluminationControl(double val) {
    building.setConstantIllumination(val);
  }

  /// Gets a Building property. Property name in .ism file: "electricappliancepowerdensityoccupied". Property is required.
  double elecPowerAppliancesOccupied() const {
    return building.electricApplianceHeatGainOccupied();
  }

  /// Sets a Building property. Property name in .ism file: "electricappliancepowerdensityoccupied". Property is required.
  void setElecPowerAppliancesOccupied(double val) {
    building.setElectricApplianceHeatGainOccupied(val);
  }

  /// Gets a Building property. Property name in .ism file: "electricappliancepowerdensityunoccupied". Property is required.
  double elecPowerAppliancesUnoccupied() const {
    return building.electricApplianceHeatGainUnoccupied();
  }

  /// Sets a Building property. Property name in .ism file: "electricappliancepowerdensityunoccupied". Property is required.
  void setElecPowerAppliancesUnoccupied(double val) {
    building.setElectricApplianceHeatGainUnoccupied(val);
  }

  /// Gets a Building property. Property name in .ism file: "electricAppliancePowerFixedOccupied". Property is required.
  double electricAppliancePowerFixedOccupied() const {
    return building.electricAppliancePowerFixedOccupied();
  }

  /// Sets a Building property. Property name in .ism file: "electricAppliancePowerFixedOccupied". Property is required.
  void setElectricAppliancePowerFixedOccupied(double electricAppliancePowerFixedOccupied) {
    building.setElectricAppliancePowerFixedOccupied(electricAppliancePowerFixedOccupied);
  }

  /// Gets a Building property. Property name in .ism file: "electricAppliancePowerFixedUnoccupied". Property is required.
  double electricAppliancePowerFixedUnoccupied() const {
    return building.electricAppliancePowerFixedUnoccupied();
  }

  /// Sets a Building property. Property name in .ism file: "electricAppliancePowerFixedUnoccupied". Property is required.
  void setElectricAppliancePowerFixedUnoccupied(double electricAppliancePowerFixedUnoccupied) {
    building.setElectricAppliancePowerFixedUnoccupied(electricAppliancePowerFixedUnoccupied);
  }

  /// Gets a Building property. Property name in .ism file: "externalequipment". Property is optional (has a default).
  double externalEquipment() const {
    return building.externalEquipment();
  }

  /// Sets a Building property. Property name in .ism file: "externalequipment". Property is optional (has a default).
  void setExternalEquipment(double externalEquipment) {
    building.setExternalEquipment(externalEquipment);
  }

  /// Gets a Building property. Property name in .ism file: "gasAppliancePowerFixedOccupied". Property is required.
  double gasAppliancePowerFixedOccupied() const {
    return building.gasAppliancePowerFixedOccupied();
  }

  /// Sets a Building property. Property name in .ism file: "gasAppliancePowerFixedOccupied". Property is required.
  void setGasAppliancePowerFixedOccupied(double gasAppliancePowerFixedOccupied) {
    building.setGasAppliancePowerFixedOccupied(gasAppliancePowerFixedOccupied);
  }

  /// Gets a Building property. Property name in .ism file: "gasAppliancePowerFixedUnoccupied". Property is required.
  double gasAppliancePowerFixedUnoccupied() const {
    return building.gasAppliancePowerFixedUnoccupied();
  }

  /// Sets a Building property. Property name in .ism file: "gasAppliancePowerFixedUnoccupied". Property is required.
  void setGasAppliancePowerFixedUnoccupied(double gasAppliancePowerFixedUnoccupied) {
    building.setGasAppliancePowerFixedUnoccupied(gasAppliancePowerFixedUnoccupied);
  }

  /// Gets a Building property. Property name in .ism file: "gasappliancepowerdensityoccupied". Property is required.
  double gasPowerAppliancesOccupied() const {
    return building.gasApplianceHeatGainOccupied();
  }

  /// Sets a Building property. Property name in .ism file: "gasappliancepowerdensityoccupied". Property is required.
  void setGasPowerAppliancesOccupied(double val) {
    building.setGasApplianceHeatGainOccupied(val);
  }

  /// Gets a Building property. Property name in .ism file: "gasappliancepowerdensityunoccupied". Property is required.
  double gasPowerAppliancesUnoccupied() const {
    return building.gasApplianceHeatGainUnoccupied();
  }

  /// Sets a Building property. Property name in .ism file: "gasappliancepowerdensityunoccupied". Property is required.
  void setGasPowerAppliancesUnoccupied(double val) {
    building.setGasApplianceHeatGainUnoccupied(val);
  }

  /// Gets a Building property. Property name in .ism file: "lightingoccupancysensordimmingfraction". Property is required.
  double lightingOccupancySensorSystem() const {
    return building.lightingOccupancySensor();
  }

  /// Sets a Building property. Property name in .ism file: "lightingoccupancysensordimmingfraction". Property is required.
  void setLightingOccupancySensorSystem(double val) {
    building.setLightingOccupancySensor(val);
  }

  /// Gets a Cooling property.
  double coolingOccupiedSetpoint() const {
    return cooling.temperatureSetPointOccupied();
  }

  /// Sets a Cooling property. Property name in .ism file: "coolingsetpointoccupied". Property is required.
  void setCoolingOccupiedSetpoint(double val) {
    cooling.setTemperatureSetPointOccupied(val);
  }

  /// Gets a Cooling property. Property name in .ism file: "coolingpumpcontrol". Property is required.
  double coolingPumpControl() {
    return cooling.pumpControlReduction();
  }

  /// Sets a Cooling property. Property name in .ism file: "coolingpumpcontrol". Property is required.
  void setCoolingPumpControl(double val) {
    cooling.setPumpControlReduction(val);
  }

  /// Gets a Cooling property. Property name in .ism file: "coolingsystemcop". Property is required.
  double coolingSystemCOP() const {
    return cooling.cop();
  }

  /// Sets a Cooling property. Property name in .ism file: "coolingsystemcop". Property is required.
  void setCoolingSystemCOP(double val) {
    cooling.setCop(val);
  }

  /// Gets a Cooling property. Property name in .ism file: "coolingsystemiplvtocopratio". Property is required.
  double coolingSystemIPLVToCOPRatio() const {
    return cooling.partialLoadValue();
  }

  /// Sets a Cooling property. Property name in .ism file: "coolingsystemiplvtocopratio". Property is required.
  void setCoolingSystemIPLVToCOPRatio(double val) {
    cooling.setPartialLoadValue(val);
  }

  /// Gets a Cooling property.
  double coolingUnoccupiedSetpoint() const {
    return cooling.temperatureSetPointUnoccupied();
  }

  /// Sets a Cooling property. Property name in .ism file: "coolingsetpointunoccupied". Property is required.
  void setCoolingUnoccupiedSetpoint(double val) {
    cooling.setTemperatureSetPointUnoccupied(val);
  }

  /// Gets a Cooling property. Property name in .ism file: "dc_yesno". Property is optional (has a default).
  double DC_YesNo() const {
    return cooling.DC_YesNo();
  }

  /// Sets a Cooling property. Property name in .ism file: "dc_yesno". Property is optional (has a default).
  void setDC_YesNo(double DC_YesNo) {
    cooling.setDC_YesNo(DC_YesNo);
  }

  /// Gets a Cooling property. Property name in .ism file: "dt_supp_cl". Property is optional (has a default).
  double dT_supp_cl() const {
    return cooling.dT_supp_cl();
  }

  /// Sets a Cooling property. Property name in .ism file: "dt_supp_cl". Property is optional (has a default).
  void setDT_supp_cl(double dT_supp_cl) {
    cooling.setDT_supp_cl(dT_supp_cl);
  }

  /// Gets a Cooling property. Property name in .ism file: "e_pumps_cl". Property is optional (has a default).
  double E_pumps_cl() const {
    return cooling.E_pumps();
  }

  /// Sets a Cooling property. Property name in .ism file: "e_pumps_cl". Property is optional (has a default).
  void setE_pumps_cl(double E_pumps) {
    cooling.setE_pumps(E_pumps);
  }

  /// Gets a Cooling property. Property name in .ism file: "eta_dc_cop_abs". Property is optional (has a default).
  double eta_DC_COP_abs() const {
    return cooling.eta_DC_COP_abs();
  }

  /// Sets a Cooling property. Property name in .ism file: "eta_dc_cop_abs". Property is optional (has a default).
  void setEta_DC_COP_abs(double eta_DC_COP_abs) {
    cooling.setEta_DC_COP_abs(eta_DC_COP_abs);
  }

  /// Gets a Cooling property. Property name in .ism file: "eta_dc_cop". Property is optional (has a default).
  double eta_DC_COP() const {
    return cooling.eta_DC_COP();
  }

  /// Sets a Cooling property. Property name in .ism file: "eta_dc_cop". Property is optional (has a default).
  void setEta_DC_COP(double eta_DC_COP) {
    cooling.setEta_DC_COP(eta_DC_COP);
  }

  /// Gets a Cooling property. Property name in .ism file: "eta_dc_frac_abs". Property is optional (has a default).
  double eta_DC_frac_abs() const {
    return cooling.eta_DC_frac_abs();
  }

  /// Sets a Cooling property. Property name in .ism file: "eta_dc_frac_abs". Property is optional (has a default).
  void setEta_DC_frac_abs(double eta_DC_frac_abs) {
    cooling.setEta_DC_frac_abs(eta_DC_frac_abs);
  }

  /// Gets a Cooling property. Property name in .ism file: "eta_dc_network". Property is optional (has a default).
  double eta_DC_network() const {
    return cooling.eta_DC_network();
  }

  /// Sets a Cooling property. Property name in .ism file: "eta_dc_network". Property is optional (has a default).
  void setEta_DC_network(double eta_DC_network) {
    cooling.setEta_DC_network(eta_DC_network);
  }

  /// Gets a Cooling property. Property name in .ism file: "forcedaircooling". Property is optional (has a default).
  bool forcedAirCooling() const {
    return cooling.forcedAirCooling();
  }

  /// Sets a Cooling property. Property name in .ism file: "forcedaircooling". Property is optional (has a default).
  void setForcedAirCooling(bool forcedAirCooling) {
    cooling.setForcedAirCooling(forcedAirCooling);
  }

  /// Gets a Cooling property. Property name in .ism file: "frac_dc_free". Property is optional (has a default).
  double frac_DC_free() const {
    return cooling.frac_DC_free();
  }

  /// Sets a Cooling property. Property name in .ism file: "frac_dc_free". Property is optional (has a default).
  void setFrac_DC_free(double frac_DC_free) {
    cooling.setFrac_DC_free(frac_DC_free);
  }

  /// Gets a Cooling property. Property name in .ism file: "hvaccoolinglossfactor". Property is required.
  double hvacCoolingLossFactor() {
    return cooling.hvacLossFactor();
  }

  /// Sets a Cooling property. Property name in .ism file: "hvaccoolinglossfactor". Property is required.
  void setHvacCoolingLossFactor(double val) {
    cooling.setHvacLossFactor(val);
  }

  /// Gets a Cooling property. Property name in .ism file: "t_cl_ctrl_flag". Property is optional (has a default).
  double T_cl_ctrl_flag() const {
    return cooling.T_cl_ctrl_flag();
  }

  /// Sets a Cooling property. Property name in .ism file: "t_cl_ctrl_flag". Property is optional (has a default).
  void setT_cl_ctrl_flag(double T_cl_ctrl_flag) {
    cooling.setT_cl_ctrl_flag(T_cl_ctrl_flag);
  }

  /// Gets a Heating property. Property name in .ism file: "a_h0". Property is optional (has a default).
  double a_H0() const {
    return heating.a_H0();
  }

  /// Sets a Heating property. Property name in .ism file: "a_h0". Property is optional (has a default).
  void setA_H0(double a_H0) {
    heating.setA_H0(a_H0);
  }

  /// Gets a Heating property. Property name in .ism file: "dh_yesno". Property is optional (has a default).
  double DH_YesNo() const {
    return heating.DH_YesNo();
  }

  /// Sets a Heating property. Property name in .ism file: "dh_yesno". Property is optional (has a default).
  void setDH_YesNo(double DH_YesNo) {
    heating.setDH_YesNo(DH_YesNo);
  }

  /// Gets a Heating property.
  double dhw_tset() const {
    return heating.dhw_tset();
  }

  /// Sets a Heating property. Property name in .ism file: "dhw_tset". Property is optional (has a default).
  void setDhw_tset(double dhw_tset) {
    heating.setDhw_tset(dhw_tset);
  }

  /// Gets a Heating property. Property name in .ism file: "dhw_tsupply". Property is optional (has a default).
  double dhw_tsupply() const {
    return heating.dhw_tsupply();
  }

  /// Sets a Heating property. Property name in .ism file: "dhw_tsupply". Property is optional (has a default).
  void setDhw_tsupply(double dhw_tsupply) {
    heating.setDhw_tsupply(dhw_tsupply);
  }

  /// Gets a Heating property. Property name in .ism file: "dhwdemand". Property is required.
  double dhwDemand() const {
    return heating.hotWaterDemand();
  }

  /// Sets a Heating property. Property name in .ism file: "dhwdemand". Property is required.
  void setDhwDemand(double val) {
    heating.setHotWaterDemand(val);
  }

  /// Gets a Heating property. Property name in .ism file: "dhwdistributionefficiency". Property is required.
  double dhwDistributionEfficiency() {
    return heating.hotWaterDistributionEfficiency();
  }

  /// Sets a Heating property. Property name in .ism file: "dhwdistributionefficiency". Property is required.
  void setDhwDistributionEfficiency(double val) {
    heating.setHotWaterDistributionEfficiency(val);
  }

  /// Sets a Heating property.
  void setDhwDistributionSystem(double val) {
    heating.setHotWaterDistributionEfficiency(val);
  }

  /// Gets a Heating property. Property name in .ism file: "dhwsystemefficiency". Property is required.
  double dhwEfficiency() const {
    return heating.hotWaterSystemEfficiency();
  }

  /// Sets a Heating property. Property name in .ism file: "dhwsystemefficiency". Property is required.
  void setDhwEfficiency(double val) {
    heating.setHotWaterSystemEfficiency(val);
  }

  /// Gets a Heating property.
  double dhwEnergyCarrier() const {
    return heating.hotWaterEnergyType();
  }

  /// Sets a Heating property.
  void setDhwEnergyCarrier(std::string type) {
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);
    if (type == ELECTRIC)
      heating.setHotWaterEnergyType(1.0);
    else if (type == GAS)
      heating.setHotWaterEnergyType(2.0);
    else
      throw std::invalid_argument("dhwFuelType parameter must be one of 'gas' or 'electric'");
  }

  /// Gets a Heating property. Property name in .ism file: "dt_supp_ht". Property is optional (has a default).
  double dT_supp_ht() const {
    return heating.dT_supp_ht();
  }

  /// Sets a Heating property. Property name in .ism file: "dt_supp_ht". Property is optional (has a default).
  void setDT_supp_ht(double dT_supp_ht) {
    heating.setDT_supp_ht(dT_supp_ht);
  }

  /// Gets a Heating property. Property name in .ism file: "e_pumps_ht". Property is optional (has a default).
  double E_pumps_ht() const {
    return heating.E_pumps();
  }

  /// Sets a Heating property. Property name in .ism file: "e_pumps_ht". Property is optional (has a default).
  void setE_pumps_ht(double E_pumps) {
    heating.setE_pumps(E_pumps);
  }

  /// Gets a Heating property. Property name in .ism file: "eta_dh_network". Property is optional (has a default).
  double eta_DH_network() const {
    return heating.eta_DH_network();
  }

  /// Sets a Heating property. Property name in .ism file: "eta_dh_network". Property is optional (has a default).
  void setEta_DH_network(double eta_DH_network) {
    heating.setEta_DH_network(eta_DH_network);
  }

  /// Gets a Heating property. Property name in .ism file: "eta_dh_sys". Property is optional (has a default).
  double eta_DH_sys() const {
    return heating.eta_DH_sys();
  }

  /// Sets a Heating property. Property name in .ism file: "eta_dh_sys". Property is optional (has a default).
  void setEta_DH_sys(double eta_DH_sys) {
    heating.setEta_DH_sys(eta_DH_sys);
  }

  /// Gets a Heating property. Property name in .ism file: "forcedairheating". Property is optional (has a default).
  bool forcedAirHeating() const {
    return heating.forcedAirHeating();
  }

  /// Sets a Heating property. Property name in .ism file: "forcedairheating". Property is optional (has a default).
  void setForcedAirHeating(bool forcedAirHeating) {
    heating.setForcedAirHeating(forcedAirHeating);
  }

  /// Gets a Heating property. Property name in .ism file: "frac_dh_free". Property is optional (has a default).
  double frac_DH_free() const {
    return heating.frac_DH_free();
  }

  /// Sets a Heating property. Property name in .ism file: "frac_dh_free". Property is optional (has a default).
  void setFrac_DH_free(double frac_DH_free) {
    heating.setFrac_DH_free(frac_DH_free);
  }

  /// Gets a Heating property.
  double heatingEnergyCarrier() const {
    return heating.energyType();
  }

  /// Sets a Heating property.
  void setHeatingEnergyCarrier(std::string type) {
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);
    if (type == ELECTRIC)
      heating.setEnergyType(1.0);
    else if (type == GAS)
      heating.setEnergyType(2.0);
    else
      throw std::invalid_argument("heatingFuelType parameter must be one of 'gas' or 'electric'");
  }

  /// Gets a Heating property.
  double heatingOccupiedSetpoint() const {
    return heating.temperatureSetPointOccupied();
  }

  /// Sets a Heating property. Property name in .ism file: "heatingsetpointoccupied". Property is required.
  void setHeatingOccupiedSetpoint(double val) {
    heating.setTemperatureSetPointOccupied(val);
  }

  /// Gets a Heating property. Property name in .ism file: "heatingpumpcontrol". Property is required.
  double heatingPumpControl() {
    return heating.pumpControlReduction();
  }

  /// Sets a Heating property. Property name in .ism file: "heatingpumpcontrol". Property is required.
  void setHeatingPumpControl(double val) {
    heating.setPumpControlReduction(val);
  }

  /// Gets a Heating property. Property name in .ism file: "heatingsystemefficiency". Property is required.
  double heatingSystemEfficiency() const {
    return heating.efficiency();
  }

  /// Sets a Heating property. Property name in .ism file: "heatingsystemefficiency". Property is required.
  void setHeatingSystemEfficiency(double val) {
    heating.setEfficiency(val);
  }

  /// Gets a Heating property.
  double heatingUnoccupiedSetpoint() const {
    return heating.temperatureSetPointUnoccupied();
  }

  /// Sets a Heating property. Property name in .ism file: "heatingsetpointunoccupied". Property is required.
  void setHeatingUnoccupiedSetpoint(double val) {
    heating.setTemperatureSetPointUnoccupied(val);
  }

  /// Gets a Heating property. Property name in .ism file: "hvacheatinglossfactor". Property is required.
  double hvacHeatingLossFactor() {
    return heating.hvacLossFactor();
  }

  /// Sets a Heating property. Property name in .ism file: "hvacheatinglossfactor". Property is required.
  void setHvacHeatingLossFactor(double val) {
    heating.setHvacLossFactor(val);
  }

  /// Gets a Heating property. Property name in .ism file: "hvacwastefactor". Property is required.
  double hvacWasteFactor() {
    return heating.hotcoldWasteFactor();
  }

  /// Sets a Heating property. Property name in .ism file: "hvacwastefactor". Property is required.
  void setHvacWasteFactor(double val) {
    heating.setHotcoldWasteFactor(val);
  }

  /// Gets a Heating property. Property name in .ism file: "t_ht_ctrl_flag". Property is optional (has a default).
  double T_ht_ctrl_flag() const {
    return heating.T_ht_ctrl_flag();
  }

  /// Sets a Heating property. Property name in .ism file: "t_ht_ctrl_flag". Property is optional (has a default).
  void setT_ht_ctrl_flag(double T_ht_ctrl_flag) {
    heating.setT_ht_ctrl_flag(T_ht_ctrl_flag);
  }

  /// Gets a Heating property. Property name in .ism file: "tau_h0". Property is optional (has a default).
  double tau_H0() const {
    return heating.tau_H0();
  }

  /// Sets a Heating property. Property name in .ism file: "tau_h0". Property is optional (has a default).
  void setTau_H0(double tau_H0) {
    heating.setTau_H0(tau_H0);
  }

  /// Gets a Lights property. Property name in .ism file: "automaticad". Property is optional (has a default).
  double automaticAd() const {
    return lights.automaticAd();
  }

  /// Sets a Lights property. Property name in .ism file: "automaticad". Property is optional (has a default).
  void setAutomaticAd(double automaticAd) {
    lights.setAutomaticAd(automaticAd);
  }

  /// Gets a Lights property. Property name in .ism file: "automaticlux". Property is optional (has a default).
  double automaticLux() const {
    return lights.automaticLux();
  }

  /// Sets a Lights property. Property name in .ism file: "automaticlux". Property is optional (has a default).
  void setAutomaticLux(double automaticLux) {
    lights.setAutomaticLux(automaticLux);
  }

  /// Gets a Lights property. Property name in .ism file: "daylightsensordimmingfraction". Property is required.
  double daylightSensorSystem() const {
    return lights.dimmingFraction();
  }

  /// Sets a Lights property. Property name in .ism file: "daylightsensordimmingfraction". Property is required.
  void setDaylightSensorSystem(double val) {
    lights.setDimmingFraction(val);
  }

  /// Gets a Lights property. Property name in .ism file: "elecinternalgains". Property is optional (has a default).
  double elecInternalGains() const {
    return lights.elecInternalGains();
  }

  /// Sets a Lights property. Property name in .ism file: "elecinternalgains". Property is optional (has a default).
  void setElecInternalGains(double elecInternalGains) {
    lights.setElecInternalGains(elecInternalGains);
  }

  /// Gets a Lights property. Property name in .ism file: "exteriorlightingpower". Property is required.
  double exteriorLightingPower() const {
    return lights.exteriorEnergy();
  }

  /// Sets a Lights property. Property name in .ism file: "exteriorlightingpower". Property is required.
  void setExteriorLightingPower(double val) {
    lights.setExteriorEnergy(val);
  }

  /// Gets a Lights property. Property name in .ism file: "lightingPowerFixedOccupied". Property is required.
  double lightingPowerFixedOccupied() const {
    return lights.lightingPowerFixedOccupied();
  }

  /// Sets a Lights property. Property name in .ism file: "lightingPowerFixedOccupied". Property is required.
  void setLightingPowerFixedOccupied(double lightingPowerFixedOccupied) {
    lights.setLightingPowerFixedOccupied(lightingPowerFixedOccupied);
  }

  /// Gets a Lights property. Property name in .ism file: "lightingPowerFixedUnoccupied". Property is required.
  double lightingPowerFixedUnoccupied() const {
    return lights.lightingPowerFixedUnoccupied();
  }

  /// Sets a Lights property. Property name in .ism file: "lightingPowerFixedUnoccupied". Property is required.
  void setLightingPowerFixedUnoccupied(double lightingPowerFixedUnoccupied) {
    lights.setLightingPowerFixedUnoccupied(lightingPowerFixedUnoccupied);
  }

  /// Gets a Lights property. Property name in .ism file: "lightingpowerdensityoccupied". Property is required.
  double lightingPowerIntensityOccupied() const {
    return lights.powerDensityOccupied();
  }

  /// Sets a Lights property. Property name in .ism file: "lightingpowerdensityoccupied". Property is required.
  void setLightingPowerIntensityOccupied(double val) {
    lights.setPowerDensityOccupied(val);
  }

  /// Gets a Lights property. Property name in .ism file: "lightingpowerdensityunoccupied". Property is required.
  double lightingPowerIntensityUnoccupied() const {
    return lights.powerDensityUnoccupied();
  }

  /// Sets a Lights property. Property name in .ism file: "lightingpowerdensityunoccupied". Property is required.
  void setLightingPowerIntensityUnoccupied(double val) {
    lights.setPowerDensityUnoccupied(val);
  }

  /// Gets a Lights property. Property name in .ism file: "manualswitchad". Property is optional (has a default).
  double manualSwitchAd() const {
    return lights.manualSwitchAd();
  }

  /// Sets a Lights property. Property name in .ism file: "manualswitchad". Property is optional (has a default).
  void setManualSwitchAd(double manualSwitchAd) {
    lights.setManualSwitchAd(manualSwitchAd);
  }

  /// Gets a Lights property. Property name in .ism file: "manualswitchlux". Property is optional (has a default).
  double manualSwitchLux() const {
    return lights.manualSwitchLux();
  }

  /// Sets a Lights property. Property name in .ism file: "manualswitchlux". Property is optional (has a default).
  void setManualSwitchLux(double manualSwitchLux) {
    lights.setManualSwitchLux(manualSwitchLux);
  }

  /// Gets a Lights property. Property name in .ism file: "n_day_end". Property is optional (has a default).
  double n_day_end() const {
    return lights.n_day_end();
  }

  /// Sets a Lights property. Property name in .ism file: "n_day_end". Property is optional (has a default).
  void setN_day_end(double n_day_end) {
    lights.setN_day_end(n_day_end);
  }

  /// Gets a Lights property. Property name in .ism file: "n_day_start". Property is optional (has a default).
  double n_day_start() const {
    return lights.n_day_start();
  }

  /// Sets a Lights property. Property name in .ism file: "n_day_start". Property is optional (has a default).
  void setN_day_start(double n_day_start) {
    lights.setN_day_start(n_day_start);
  }

  /// Gets a Lights property. Property name in .ism file: "n_weeks". Property is optional (has a default).
  double n_weeks() const {
    return lights.n_weeks();
  }

  /// Sets a Lights property. Property name in .ism file: "n_weeks". Property is optional (has a default).
  void setN_weeks(double n_weeks) {
    lights.setN_weeks(n_weeks);
  }

  /// Gets a Lights property. Property name in .ism file: "naturallylightedarea". Property is optional (has a default).
  double naturallyLightedArea() const {
    return lights.naturallyLightedArea();
  }

  /// Sets a Lights property. Property name in .ism file: "naturallylightedarea". Property is optional (has a default).
  void setNaturallyLightedArea(double naturallyLightedArea) {
    lights.setNaturallyLightedArea(naturallyLightedArea);
  }

  /// Gets a Lights property. Property name in .ism file: "permlightpowerdensity". Property is optional (has a default).
  double permLightPowerDensity() const {
    return lights.permLightPowerDensity();
  }

  /// Sets a Lights property. Property name in .ism file: "permlightpowerdensity". Property is optional (has a default).
  void setPermLightPowerDensity(double permLightPowerDensity) {
    lights.setPermLightPowerDensity(permLightPowerDensity);
  }

  /// Gets a Lights property. Property name in .ism file: "presenceautoad". Property is optional (has a default).
  double presenceAutoAd() const {
    return lights.presenceAutoAd();
  }

  /// Sets a Lights property. Property name in .ism file: "presenceautoad". Property is optional (has a default).
  void setPresenceAutoAd(double presenceAutoAd) {
    lights.setPresenceAutoAd(presenceAutoAd);
  }

  /// Gets a Lights property. Property name in .ism file: "presenceautolux". Property is optional (has a default).
  double presenceAutoLux() const {
    return lights.presenceAutoLux();
  }

  /// Sets a Lights property. Property name in .ism file: "presenceautolux". Property is optional (has a default).
  void setPresenceAutoLux(double presenceAutoLux) {
    lights.setPresenceAutoLux(presenceAutoLux);
  }

  /// Gets a Lights property. Property name in .ism file: "presencesensorad". Property is optional (has a default).
  double presenceSensorAd() const {
    return lights.presenceSensorAd();
  }

  /// Sets a Lights property. Property name in .ism file: "presencesensorad". Property is optional (has a default).
  void setPresenceSensorAd(double presenceSensorAd) {
    lights.setPresenceSensorAd(presenceSensorAd);
  }

  /// Gets a Lights property. Property name in .ism file: "presencesensorlux". Property is optional (has a default).
  double presenceSensorLux() const {
    return lights.presenceSensorLux();
  }

  /// Sets a Lights property. Property name in .ism file: "presencesensorlux". Property is optional (has a default).
  void setPresenceSensorLux(double presenceSensorLux) {
    lights.setPresenceSensorLux(presenceSensorLux);
  }

  /// Gets a Location property. Property name in .ism file: "terrainclass". Property is required.
  double terrainClass() const {
    return location.terrain();
  }

  /// Sets a Location property. Property name in .ism file: "terrainclass". Property is required.
  void setTerrainClass(double val) {
    location.setTerrain(val);
  }

  /// Gets a PhysicalQuantities property. Property name in .ism file: "rhocpair". Property is optional (has a default).
  double rhoCpAir() const {
    return phys.rhoCpAir();
  }

  /// Sets a PhysicalQuantities property. Property name in .ism file: "rhocpair". Property is optional (has a default).
  void setRhoCpAir(double rhoCpAir) {
    phys.setRhoCpAir(rhoCpAir);
  }

  /// Gets a PhysicalQuantities property. Property name in .ism file: "rhocpwater". Property is optional (has a default).
  double rhoCpWater() const {
    return phys.rhoCpWater();
  }

  /// Sets a PhysicalQuantities property. Property name in .ism file: "rhocpwater". Property is optional (has a default).
  void setRhoCpWater(double rhoCpWater) {
    phys.setRhoCpWater(rhoCpWater);
  }

  /// Sets a Population property. Property name in .ism file: "occupancydayfirst". Property is required.
  void setBuildingOccupancyFrom(double val) {
    pop.setDaysStart(val);
  }

  /// Sets a Population property. Property name in .ism file: "occupancydaylast". Property is required.
  void setBuildingOccupancyTo(double val) {
    pop.setDaysEnd(val);
  }

  /// Gets a Population property. Property name in .ism file: "occupancyhourfirst". Property is required.
  double equivFullLoadOccupancyFrom() const {
    return pop.hoursStart();
  }

  /// Sets a Population property. Property name in .ism file: "occupancyhourfirst". Property is required.
  void setEquivFullLoadOccupancyFrom(double val) {
    pop.setHoursStart(val);
  }

  /// Gets a Population property. Property name in .ism file: "occupancyhourlast". Property is required.
  double equivFullLoadOccupancyTo() const {
    return pop.hoursEnd();
  }

  /// Sets a Population property. Property name in .ism file: "occupancyhourlast". Property is required.
  void setEquivFullLoadOccupancyTo(double val) {
    pop.setHoursEnd(val);
  }

  /// Gets a Population property. Property name in .ism file: "heatgainperperson". Property is required.
  double heatGainPerPerson() {
    return pop.heatGainPerPerson();
  }

  /// Sets a Population property. Property name in .ism file: "heatgainperperson". Property is required.
  void setHeatGainPerPerson(double val) {
    pop.setHeatGainPerPerson(val);
  }

  /// Gets a Population property. Property name in .ism file: "peopledensityoccupied". Property is required.
  double peopleDensityOccupied() const {
    return pop.densityOccupied();
  }

  /// Sets a Population property. Property name in .ism file: "peopledensityoccupied". Property is required.
  void setPeopleDensityOccupied(double val) {
    pop.setDensityOccupied(val);
  }

  /// Gets a Population property. Property name in .ism file: "peopledensityunoccupied". Property is required.
  double peopleDensityUnoccupied() const {
    return pop.densityUnoccupied();
  }

  /// Sets a Population property. Property name in .ism file: "peopledensityunoccupied". Property is required.
  void setPeopleDensityUnoccupied(double val) {
    pop.setDensityUnoccupied(val);
  }

  /// Gets a Population property. Property name in .ism file: "schedulefilepath". Property is required.
  std::string scheduleFilePath() const {
    // TODO: This property isn't used by the simulations yet -BAA@2015-06-18
    return pop.scheduleFilePath();
  }

  /// Sets a Population property. Property name in .ism file: "schedulefilepath". Property is required.
  void setScheduleFilePath(std::string scheduleFilePath) {
    // TODO: This property isn't used by the simulations yet -BAA@2015-06-18
    pop.setScheduleFilePath(scheduleFilePath);
  }

  /// Gets a SimulationSettings property. Property name in .ism file: "hci". Property is optional (has a default).
  double hci() const {
    return simSettings.hci();
  }

  /// Sets a SimulationSettings property. Property name in .ism file: "hci". Property is optional (has a default).
  void setHci(double hci) {
    simSettings.setHci(hci);
  }

  /// Gets a SimulationSettings property. Property name in .ism file: "hri". Property is optional (has a default).
  double hri() const {
    return simSettings.hri();
  }

  /// Sets a SimulationSettings property. Property name in .ism file: "hri". Property is optional (has a default).
  void setHri(double hri) {
    simSettings.setHri(hri);
  }

  /// Gets a SimulationSettings property. Property name in .ism file: "phiintfractiontoairnode". Property is optional (has a default).
  double phiIntFractionToAirNode() const {
    return simSettings.phiIntFractionToAirNode();
  }

  /// Sets a SimulationSettings property. Property name in .ism file: "phiintfractiontoairnode". Property is optional (has a default).
  void setPhiIntFractionToAirNode(double phiIntFractionToAirNode) {
    simSettings.setPhiIntFractionToAirNode(phiIntFractionToAirNode);
  }

  /// Gets a SimulationSettings property. Property name in .ism file: "phisolfractiontoairnode". Property is optional (has a default).
  double phiSolFractionToAirNode() const {
    return simSettings.phiSolFractionToAirNode();
  }

  /// Sets a SimulationSettings property. Property name in .ism file: "phisolfractiontoairnode". Property is optional (has a default).
  void setPhiSolFractionToAirNode(double phiSolFractionToAirNode) {
    simSettings.setPhiSolFractionToAirNode(phiSolFractionToAirNode);
  }

  /// Sets a Structure property. Property name in .ism file: "infiltrationrateoccupied". Property is required.
  void setBuildingAirLeakage(double val) {
    structure.setInfiltrationRate(val);
  }

  /// Sets a Structure property. Property name in .ism file: "buildingheight". Property is required.
  void setBuildingHeight(double val) {
    structure.setBuildingHeight(val);
  }

  /// Gets a Structure property. Property name in .ism file: "exteriorheatcapacity". Property is required.
  double exteriorHeatCapacity() {
    return structure.wallHeatCapacity();
  }

  /// Sets a Structure property. Property name in .ism file: "exteriorheatcapacity". Property is required.
  void setExteriorHeatCapacity(double val) {
    structure.setWallHeatCapacity(val);
  }

  /// Gets a Structure property. Property name in .ism file: "floorarea". Property is required.
  double floorArea() const {
    return structure.floorArea();
  }

  /// Sets a Structure property. Property name in .ism file: "floorarea". Property is required.
  void setFloorArea(double val) {
    structure.setFloorArea(val);
  }

  /// Gets a Structure property. Property name in .ism file: "interiorheatcapacity". Property is required.
  double interiorHeatCapacity() const {
    return structure.interiorHeatCapacity();
  }

  /// Sets a Structure property. Property name in .ism file: "interiorheatcapacity". Property is required.
  void setInteriorHeatCapacity(double val) {
    structure.setInteriorHeatCapacity(val);
  }

  /// Gets a Structure property. Property name in .ism file: "irradianceformaxshadinguse". Property is optional (has a default).
  double irradianceForMaxShadingUse() const {
    return structure.irradianceForMaxShadingUse();
  }

  /// Sets a Structure property. Property name in .ism file: "irradianceformaxshadinguse". Property is optional (has a default).
  void setIrradianceForMaxShadingUse(double irradianceForMaxShadingUse) {
    structure.setIrradianceForMaxShadingUse(irradianceForMaxShadingUse);
  }

  /// Gets a Structure property. Property name in .ism file: "r_sc_ext". Property is optional (has a default).
  double R_sc_ext() const {
    return structure.R_sc_ext();
  }

  /// Sets a Structure property. Property name in .ism file: "r_sc_ext". Property is optional (has a default).
  void setR_sc_ext(double R_sc_ext) {
    structure.setR_sc_ext(R_sc_ext);
  }

  /// Gets a Structure property. Property name in .ism file: "r_se". Property is optional (has a default).
  double R_se() const {
    return structure.R_se();
  }

  /// Sets a Structure property. Property name in .ism file: "r_se". Property is optional (has a default).
  void setR_se(double R_se) {
    structure.setR_se(R_se);
  }

  /// Gets a Structure property.
  double roofArea() {
    return structure.wallArea()[8];
  }

  /// Sets a Structure property.
  void setRoofArea(double val) {
    structure.setWallArea(8, val);
  }

  /// Gets a Structure property.
  double roofSolarAbsorption() const {
    return structure.wallSolarAbsorption()[8];
  }

  /// Sets a Structure property.
  void setRoofSolarAbsorption(double val) {
    structure.setWallSolarAbsorption(8, val);
  }

  /// Gets a Structure property.
  double roofThermalEmissivity() const {
    return structure.wallThermalEmissivity()[8];
  }

  /// Sets a Structure property.
  void setRoofThermalEmissivity(double val) {
    structure.setWallThermalEmissivity(8, val);
  }

  /// Gets a Structure property.
  double roofUValue() const {
    return structure.wallUniform()[8];
  }

  /// Sets a Structure property.
  void setRoofUValue(double val) {
    structure.setWallUniform(8, val);
  }

  /// Gets a Structure property. Property name in .ism file: "shadingfactoratmaxuse". Property is optional (has a default).
  double shadingFactorAtMaxUse() const {
    return structure.shadingFactorAtMaxUse();
  }

  /// Sets a Structure property. Property name in .ism file: "shadingfactoratmaxuse". Property is optional (has a default).
  void setShadingFactorAtMaxUse(double shadingFactorAtMaxUse) {
    structure.setShadingFactorAtMaxUse(shadingFactorAtMaxUse);
  }

  /// Gets a Structure property.
  double skylightArea() {
    return structure.windowArea()[8];
  }

  /// Sets a Structure property.
  void setSkylightArea(double val) {
    structure.setWindowArea(8, val);
  }

  /// Gets a Structure property.
  double skylightSCF() {
    return structure.windowShadingCorrectionFactor()[8];
  }

  /// Sets a Structure property.
  void setSkylightSCF(double val) {
    structure.setWindowShadingCorrectionFactor(8, val);
  }

  /// Gets a Structure property.
  double skylightSDF() const {
    return structure.windowShadingDevice()[8];
  }

  /// Sets a Structure property.
  void setSkylightSDF(double val) {
    structure.setWindowShadingDevice(8, val);
  }

  /// Gets a Structure property.
  double skylightSHGC() {
    return structure.windowNormalIncidenceSolarEnergyTransmittance()[8];
  }

  /// Sets a Structure property.
  void setSkylightSHGC(double val) {
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(8, val);
  }

  /// Gets a Structure property.
  double skylightUvalue() {
    return structure.windowUniform()[8];
  }

  /// Sets a Structure property.
  void setSkylightUvalue(double val) {
    structure.setWindowUniform(8, val);
  }

  /// Gets a Structure property. Property name in .ism file: "totalareaperfloorarea". Property is optional (has a default).
  double totalAreaPerFloorArea() const {
    return structure.totalAreaPerFloorArea();
  }

  /// Sets a Structure property. Property name in .ism file: "totalareaperfloorarea". Property is optional (has a default).
  void setTotalAreaPerFloorArea(double totalAreaPerFloorArea) {
    structure.setTotalAreaPerFloorArea(totalAreaPerFloorArea);
  }

  /// Sets a Structure property. Property name in .ism file: "wallArea". Property is required.
  void setWallArea(const Vector& vec) {
    if (vec.size() != 9) {
      throw std::invalid_argument("Invalid number of values for WallArea parameter. It must have 9.");
    }
    structure.setWallArea(vec);
  }

  /// Gets a Structure property.
  double wallAreaE() {
    return structure.wallArea()[2];
  }

  /// Sets a Structure property.
  void setWallAreaE(double val) {
    structure.setWallArea(2, val);
  }

  /// Gets a Structure property.
  double wallAreaN() {
    return structure.wallArea()[4];
  }

  /// Sets a Structure property.
  void setWallAreaN(double val) {
    structure.setWallArea(4, val);
  }

  /// Gets a Structure property.
  double wallAreaNE() {
    return structure.wallArea()[3];
  }

  /// Sets a Structure property.
  void setWallAreaNE(double val) {
    structure.setWallArea(3, val);
  }

  /// Gets a Structure property.
  double wallAreaNW() {
    return structure.wallArea()[5];
  }

  /// Sets a Structure property.
  void setWallAreaNW(double val) {
    structure.setWallArea(5, val);
  }

  /// Gets a Structure property.
  double wallAreaS() {
    return structure.wallArea()[0];
  }

  /// Sets a Structure property.
  void setWallAreaS(double val) {
    structure.setWallArea(0, val);
  }

  /// Gets a Structure property.
  double wallAreaSE() {
    return structure.wallArea()[1];
  }

  /// Sets a Structure property.
  void setWallAreaSE(double val) {
    structure.setWallArea(1, val);
  }

  /// Gets a Structure property.
  double wallAreaSW() {
    return structure.wallArea()[7];
  }

  /// Sets a Structure property.
  void setWallAreaSW(double val) {
    structure.setWallArea(7, val);
  }

  /// Gets a Structure property.
  double wallAreaW() {
    return structure.wallArea()[6];
  }

  /// Sets a Structure property.
  void setWallAreaW(double val) {
    structure.setWallArea(6, val);
  }

  /// Sets a Structure property. Property name in .ism file: "wallAbsorption". Property is required.
  void setWallSolarAbsorption(const Vector& vec) {
    if (vec.size() != 9) {
      throw std::invalid_argument("Invalid number of values for WallSolarAbsorption parameter. It must have 9.");
    }
    structure.setWallSolarAbsorption(vec);
  }

  /// Gets a Structure property.
  double wallSolarAbsorptionE() const {
    return structure.wallSolarAbsorption()[2];
  }

  /// Sets a Structure property.
  void setWallSolarAbsorptionE(double val) {
    structure.setWallSolarAbsorption(2, val);
  }

  /// Gets a Structure property.
  double wallSolarAbsorptionN() const {
    return structure.wallSolarAbsorption()[4];
  }

  /// Sets a Structure property.
  void setWallSolarAbsorptionN(double val) {
    structure.setWallSolarAbsorption(4, val);
  }

  /// Gets a Structure property.
  double wallSolarAbsorptionNE() const {
    return structure.wallSolarAbsorption()[3];
  }

  /// Sets a Structure property.
  void setWallSolarAbsorptionNE(double val) {
    structure.setWallSolarAbsorption(3, val);
  }

  /// Gets a Structure property.
  double wallSolarAbsorptionNW() const {
    return structure.wallSolarAbsorption()[5];
  }

  /// Sets a Structure property.
  void setWallSolarAbsorptionNW(double val) {
    structure.setWallSolarAbsorption(5, val);
  }

  /// Gets a Structure property.
  double wallSolarAbsorptionS() const {
    return structure.wallSolarAbsorption()[0];
  }

  /// Sets a Structure property.
  void setWallSolarAbsorptionS(double val) {
    structure.setWallSolarAbsorption(0, val);
  }

  /// Gets a Structure property.
  double wallSolarAbsorptionSE() const {
    return structure.wallSolarAbsorption()[1];
  }

  /// Sets a Structure property.
  void setWallSolarAbsorptionSE(double val) {
    structure.setWallSolarAbsorption(1, val);
  }

  /// Gets a Structure property.
  double wallSolarAbsorptionSW() const {
    return structure.wallSolarAbsorption()[7];
  }

  /// Sets a Structure property.
  void setWallSolarAbsorptionSW(double val) {
    structure.setWallSolarAbsorption(7, val);
  }

  /// Gets a Structure property.
  double wallSolarAbsorptionW() const {
    return structure.wallSolarAbsorption()[6];
  }

  /// Sets a Structure property.
  void setWallSolarAbsorptionW(double val) {
    structure.setWallSolarAbsorption(6, val);
  }

  /// Sets a Structure property. Property name in .ism file: "wallEmissivity". Property is required.
  void setWallThermalEmissivity(const Vector& vec) {
    if (vec.size() != 9) {
      throw std::invalid_argument("Invalid number of values for WallThermalEmissivity parameter. It must have 9.");
    }
    structure.setWallThermalEmissivity(vec);
  }

  /// Gets a Structure property.
  double wallThermalEmissivityE() const {
    return structure.wallThermalEmissivity()[2];
  }

  /// Sets a Structure property.
  void setWallThermalEmissivityE(double val) {
    structure.setWallThermalEmissivity(2, val);
  }

  /// Gets a Structure property.
  double wallThermalEmissivityN() const {
    return structure.wallThermalEmissivity()[4];
  }

  /// Sets a Structure property.
  void setWallThermalEmissivityN(double val) {
    structure.setWallThermalEmissivity(4, val);
  }

  /// Gets a Structure property.
  double wallThermalEmissivityNE() const {
    return structure.wallThermalEmissivity()[3];
  }

  /// Sets a Structure property.
  void setWallThermalEmissivityNE(double val) {
    structure.setWallThermalEmissivity(3, val);
  }

  /// Gets a Structure property.
  double wallThermalEmissivityNW() const {
    return structure.wallThermalEmissivity()[5];
  }

  /// Sets a Structure property.
  void setWallThermalEmissivityNW(double val) {
    structure.setWallThermalEmissivity(5, val);
  }

  /// Gets a Structure property.
  double wallThermalEmissivityS() const {
    return structure.wallThermalEmissivity()[0];
  }

  /// Sets a Structure property.
  void setWallThermalEmissivityS(double val) {
    structure.setWallThermalEmissivity(0, val);
  }

  /// Gets a Structure property.
  double wallThermalEmissivitySE() const {
    return structure.wallThermalEmissivity()[1];
  }

  /// Sets a Structure property.
  void setWallThermalEmissivitySE(double val) {
    structure.setWallThermalEmissivity(1, val);
  }

  /// Gets a Structure property.
  double wallThermalEmissivitySW() const {
    return structure.wallThermalEmissivity()[7];
  }

  /// Sets a Structure property.
  void setWallThermalEmissivitySW(double val) {
    structure.setWallThermalEmissivity(7, val);
  }

  /// Gets a Structure property.
  double wallThermalEmissivityW() const {
    return structure.wallThermalEmissivity()[6];
  }

  /// Sets a Structure property.
  void setWallThermalEmissivityW(double val) {
    structure.setWallThermalEmissivity(6, val);
  }

  /// Sets a Structure property. Property name in .ism file: "wallU". Property is required.
  void setWallU(const Vector& vec) {
    if (vec.size() != 9) {
      throw std::invalid_argument("Invalid number of values for WallU parameter. It must have 9.");
    }
    structure.setWallUniform(vec);
  }

  /// Gets a Structure property.
  double wallUvalueE() const {
    return structure.wallUniform()[2];
  }

  /// Sets a Structure property.
  void setWallUvalueE(double val) {
    structure.setWallUniform(2, val);
  }

  /// Gets a Structure property.
  double wallUvalueN() const {
    return structure.wallUniform()[4];
  }

  /// Sets a Structure property.
  void setWallUvalueN(double val) {
    structure.setWallUniform(4, val);
  }

  /// Gets a Structure property.
  double wallUvalueNE() const {
    return structure.wallUniform()[3];
  }

  /// Sets a Structure property.
  void setWallUvalueNE(double val) {
    structure.setWallUniform(3, val);
  }

  /// Gets a Structure property.
  double wallUvalueNW() const {
    return structure.wallUniform()[5];
  }

  /// Sets a Structure property.
  void setWallUvalueNW(double val) {
    structure.setWallUniform(5, val);
  }

  /// Gets a Structure property.
  double wallUvalueS() const {
    return structure.wallUniform()[0];
  }

  /// Sets a Structure property.
  void setWallUvalueS(double val) {
    structure.setWallUniform(0, val);
  }

  /// Gets a Structure property.
  double wallUvalueSE() const {
    return structure.wallUniform()[1];
  }

  /// Sets a Structure property.
  void setWallUvalueSE(double val) {
    structure.setWallUniform(1, val);
  }

  /// Gets a Structure property.
  double wallUvalueSW() const {
    return structure.wallUniform()[7];
  }

  /// Sets a Structure property.
  void setWallUvalueSW(double val) {
    structure.setWallUniform(7, val);
  }

  /// Gets a Structure property.
  double wallUvalueW() const {
    return structure.wallUniform()[6];
  }

  /// Sets a Structure property.
  void setWallUvalueW(double val) {
    structure.setWallUniform(6, val);
  }

  /// Gets a Structure property. Property name in .ism file: "win_f_w". Property is optional (has a default).
  double win_F_W() const {
    return structure.win_F_W();
  }

  /// Sets a Structure property. Property name in .ism file: "win_f_w". Property is optional (has a default).
  void setWin_F_W(double win_F_W) {
    structure.setWin_F_W(win_F_W);
  }

  /// Gets a Structure property. Property name in .ism file: "win_ff". Property is optional (has a default).
  double win_ff() const {
    return structure.win_ff();
  }

  /// Sets a Structure property. Property name in .ism file: "win_ff". Property is optional (has a default).
  void setWin_ff(double win_ff) {
    structure.setWin_ff(win_ff);
  }

  /// Sets a Structure property. Property name in .ism file: "windowArea". Property is required.
  void setWindowArea(const Vector& vec) {
    if (vec.size() != 9) {
      throw std::invalid_argument("Invalid number of values for WindowArea parameter. It must have 9.");
    }
    structure.setWindowArea(vec);
  }

  /// Gets a Structure property.
  double windowAreaE() {
    return structure.windowArea()[2];
  }

  /// Sets a Structure property.
  void setWindowAreaE(double val) {
    structure.setWindowArea(2, val);
  }

  /// Gets a Structure property.
  double windowAreaN() {
    return structure.windowArea()[4];
  }

  /// Sets a Structure property.
  void setWindowAreaN(double val) {
    structure.setWindowArea(4, val);
  }

  /// Gets a Structure property.
  double windowAreaNE() {
    return structure.windowArea()[3];
  }

  /// Sets a Structure property.
  void setWindowAreaNE(double val) {
    structure.setWindowArea(3, val);
  }

  /// Gets a Structure property.
  double windowAreaNW() {
    return structure.windowArea()[5];
  }

  /// Sets a Structure property.
  void setWindowAreaNW(double val) {
    structure.setWindowArea(5, val);
  }

  /// Gets a Structure property.
  double windowAreaS() {
    return structure.windowArea()[0];
  }

  /// Sets a Structure property.
  void setWindowAreaS(double val) {
    structure.setWindowArea(0, val);
  }

  /// Gets a Structure property.
  double windowAreaSE() {
    return structure.windowArea()[1];
  }

  /// Sets a Structure property.
  void setWindowAreaSE(double val) {
    structure.setWindowArea(1, val);
  }

  /// Gets a Structure property.
  double windowAreaSW() {
    return structure.windowArea()[7];
  }

  /// Sets a Structure property.
  void setWindowAreaSW(double val) {
    structure.setWindowArea(7, val);
  }

  /// Gets a Structure property.
  double windowAreaW() {
    return structure.windowArea()[6];
  }

  /// Sets a Structure property.
  void setWindowAreaW(double val) {
    structure.setWindowArea(6, val);
  }

  /// Sets a Structure property. Property name in .ism file: "windowSCF". Property is required.
  void setWindowSCF(const Vector& vec) {
    if (vec.size() != 9) {
      throw std::invalid_argument("Invalid number of values for WindowSCF parameter. It must have 9.");
    }
    structure.setWindowShadingCorrectionFactor(vec);
  }

  /// Gets a Structure property.
  double windowSCFE() const {
    return structure.windowShadingCorrectionFactor()[2];
  }

  /// Sets a Structure property.
  void setWindowSCFE(double val) {
    structure.setWindowShadingCorrectionFactor(2, val);
  }

  /// Gets a Structure property.
  double windowSCFN() const {
    return structure.windowShadingCorrectionFactor()[4];
  }

  /// Sets a Structure property.
  void setWindowSCFN(double val) {
    structure.setWindowShadingCorrectionFactor(4, val);
  }

  /// Gets a Structure property.
  double windowSCFNE() const {
    return structure.windowShadingCorrectionFactor()[3];
  }

  /// Sets a Structure property.
  void setWindowSCFNE(double val) {
    structure.setWindowShadingCorrectionFactor(3, val);
  }

  /// Gets a Structure property.
  double windowSCFNW() const {
    return structure.windowShadingCorrectionFactor()[5];
  }

  /// Sets a Structure property.
  void setWindowSCFNW(double val) {
    structure.setWindowShadingCorrectionFactor(5, val);
  }

  /// Gets a Structure property.
  double windowSCFS() const {
    return structure.windowShadingCorrectionFactor()[0];
  }

  /// Sets a Structure property.
  void setWindowSCFS(double val) {
    structure.setWindowShadingCorrectionFactor(0, val);
  }

  /// Gets a Structure property.
  double windowSCFSE() const {
    return structure.windowShadingCorrectionFactor()[1];
  }

  /// Sets a Structure property.
  void setWindowSCFSE(double val) {
    structure.setWindowShadingCorrectionFactor(1, val);
  }

  /// Gets a Structure property.
  double windowSCFSW() const {
    return structure.windowShadingCorrectionFactor()[7];
  }

  /// Sets a Structure property.
  void setWindowSCFSW(double val) {
    structure.setWindowShadingCorrectionFactor(7, val);
  }

  /// Gets a Structure property.
  double windowSCFW() const {
    return structure.windowShadingCorrectionFactor()[6];
  }

  /// Sets a Structure property.
  void setWindowSCFW(double val) {
    structure.setWindowShadingCorrectionFactor(6, val);
  }

  /// Sets a Structure property. Property name in .ism file: "windowSDF". Property is required.
  void setWindowSDF(const Vector& vec) {
    if (vec.size() != 9) {
      throw std::invalid_argument("Invalid number of values for WindowSDF parameter. It must have 9.");
    }
    structure.setWindowShadingDevice(vec);
  }

  /// Gets a Structure property.
  double windowSDFE() const {
    return structure.windowShadingDevice()[2];
  }

  /// Sets a Structure property.
  void setWindowSDFE(double val) {
    structure.setWindowShadingDevice(2, val);
  }

  /// Gets a Structure property.
  double windowSDFN() const {
    return structure.windowShadingDevice()[4];
  }

  /// Sets a Structure property.
  void setWindowSDFN(double val) {
    structure.setWindowShadingDevice(4, val);
  }

  /// Gets a Structure property.
  double windowSDFNE() const {
    return structure.windowShadingDevice()[3];
  }

  /// Sets a Structure property.
  void setWindowSDFNE(double val) {
    structure.setWindowShadingDevice(3, val);
  }

  /// Gets a Structure property.
  double windowSDFNW() const {
    return structure.windowShadingDevice()[5];
  }

  /// Sets a Structure property.
  void setWindowSDFNW(double val) {
    structure.setWindowShadingDevice(5, val);
  }

  /// Gets a Structure property.
  double windowSDFS() const {
    return structure.windowShadingDevice()[0];
  }

  /// Sets a Structure property.
  void setWindowSDFS(double val) {
    structure.setWindowShadingDevice(0, val);
  }

  /// Gets a Structure property.
  double windowSDFSE() const {
    return structure.windowShadingDevice()[1];
  }

  /// Sets a Structure property.
  void setWindowSDFSE(double val) {
    structure.setWindowShadingDevice(1, val);
  }

  /// Gets a Structure property.
  double windowSDFSW() const {
    return structure.windowShadingDevice()[7];
  }

  /// Sets a Structure property.
  void setWindowSDFSW(double val) {
    structure.setWindowShadingDevice(7, val);
  }

  /// Gets a Structure property.
  double windowSDFW() const {
    return structure.windowShadingDevice()[6];
  }

  /// Sets a Structure property.
  void setWindowSDFW(double val) {
    structure.setWindowShadingDevice(6, val);
  }

  /// Sets a Structure property. Property name in .ism file: "windowSHGC". Property is required.
  void setWindowSHGC(const Vector& vec) {
    if (vec.size() != 9) {
      throw std::invalid_argument("Invalid number of values for WindowSHGC parameter. It must have 9.");
    }
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(vec);
  }

  /// Gets a Structure property.
  double windowSHGCE() const {
    return structure.windowNormalIncidenceSolarEnergyTransmittance()[2];
  }

  /// Sets a Structure property.
  void setWindowSHGCE(double val) {
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(2, val);
  }

  /// Gets a Structure property.
  double windowSHGCN() const {
    return structure.windowNormalIncidenceSolarEnergyTransmittance()[4];
  }

  /// Sets a Structure property.
  void setWindowSHGCN(double val) {
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(4, val);
  }

  /// Gets a Structure property.
  double windowSHGCNE() const {
    return structure.windowNormalIncidenceSolarEnergyTransmittance()[3];
  }

  /// Sets a Structure property.
  void setWindowSHGCNE(double val) {
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(3, val);
  }

  /// Gets a Structure property.
  double windowSHGCNW() const {
    return structure.windowNormalIncidenceSolarEnergyTransmittance()[5];
  }

  /// Sets a Structure property.
  void setWindowSHGCNW(double val) {
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(5, val);
  }

  /// Gets a Structure property.
  double windowSHGCS() const {
    return structure.windowNormalIncidenceSolarEnergyTransmittance()[0];
  }

  /// Sets a Structure property.
  void setWindowSHGCS(double val) {
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(0, val);
  }

  /// Gets a Structure property.
  double windowSHGCSE() const {
    return structure.windowNormalIncidenceSolarEnergyTransmittance()[1];
  }

  /// Sets a Structure property.
  void setWindowSHGCSE(double val) {
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(1, val);
  }

  /// Gets a Structure property.
  double windowSHGCSW() const {
    return structure.windowNormalIncidenceSolarEnergyTransmittance()[7];
  }

  /// Sets a Structure property.
  void setWindowSHGCSW(double val) {
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(7, val);
  }

  /// Gets a Structure property.
  double windowSHGCW() const {
    return structure.windowNormalIncidenceSolarEnergyTransmittance()[6];
  }

  /// Sets a Structure property.
  void setWindowSHGCW(double val) {
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(6, val);
  }

  /// Sets a Structure property. Property name in .ism file: "windowU". Property is required.
  void setWindowU(const Vector& vec) {
    if (vec.size() != 9) {
      throw std::invalid_argument("Invalid number of values for WindowU parameter. It must have 9.");
    }
    structure.setWindowUniform(vec);
  }

  /// Gets a Structure property.
  double windowUvalueE() const {
    return structure.windowUniform()[2];
  }

  /// Sets a Structure property.
  void setWindowUvalueE(double val) {
    structure.setWindowUniform(2, val);
  }

  /// Gets a Structure property.
  double windowUvalueN() const {
    return structure.windowUniform()[4];
  }

  /// Sets a Structure property.
  void setWindowUvalueN(double val) {
    structure.setWindowUniform(4, val);
  }

  /// Gets a Structure property.
  double windowUvalueNE() const {
    return structure.windowUniform()[3];
  }

  /// Sets a Structure property.
  void setWindowUvalueNE(double val) {
    structure.setWindowUniform(3, val);
  }

  /// Gets a Structure property.
  double windowUvalueNW() const {
    return structure.windowUniform()[5];
  }

  /// Sets a Structure property.
  void setWindowUvalueNW(double val) {
    structure.setWindowUniform(5, val);
  }

  /// Gets a Structure property.
  double windowUvalueS() const {
    return structure.windowUniform()[0];
  }

  /// Sets a Structure property.
  void setWindowUvalueS(double val) {
    structure.setWindowUniform(0, val);
  }

  /// Gets a Structure property.
  double windowUvalueSE() const {
    return structure.windowUniform()[1];
  }

  /// Sets a Structure property.
  void setWindowUvalueSE(double val) {
    structure.setWindowUniform(1, val);
  }

  /// Gets a Structure property.
  double windowUvalueSW() const {
    return structure.windowUniform()[7];
  }

  /// Sets a Structure property.
  void setWindowUvalueSW(double val) {
    structure.setWindowUniform(7, val);
  }

  /// Gets a Structure property.
  double windowUvalueW() const {
    return structure.windowUniform()[6];
  }

  /// Sets a Structure property.
  void setWindowUvalueW(double val) {
    structure.setWindowUniform(6, val);
  }

  /// Gets a Ventilation property. Property name in .ism file: "dcp". Property is optional (has a default).
  double dCp() const {
    return ventilation.dCp();
  }

  /// Sets a Ventilation property. Property name in .ism file: "dcp". Property is optional (has a default).
  void setDCp(double dCp) {
    ventilation.setDCp(dCp);
  }

  /// Gets a Ventilation property. Property name in .ism file: "exhaustairrecirculation". Property is required.
  double exhaustAirRecirclation() const {
    return ventilation.exhaustAirRecirculated();
  }

  /// Sets a Ventilation property. Property name in .ism file: "exhaustairrecirculation". Property is required.
  void setExhaustAirRecirclation(double val) {
    ventilation.setExhaustAirRecirculated(val);
  }

  /// Gets a Ventilation property. Property name in .ism file: "fanflowcontrolfactor". Property is required.
  double fanFlowControlFactor() const {
    return ventilation.fanControlFactor();
  }

  /// Sets a Ventilation property. Property name in .ism file: "fanflowcontrolfactor". Property is required.
  void setFanFlowControlFactor(double val) {
    ventilation.setFanControlFactor(val);
  }

  /// Gets a Ventilation property. Property name in .ism file: "ventilationintakerateoccupied". Property is required.
  double freshAirFlowRate() const {
    return ventilation.supplyRate();
  }

  /// Sets a Ventilation property. Property name in .ism file: "ventilationintakerateoccupied". Property is required.
  void setFreshAirFlowRate(double val) {
    ventilation.setSupplyRate(val);
  }

  /// Gets a Ventilation property. Property name in .ism file: "h_ve". Property is optional (has a default).
  double H_ve() const {
    return ventilation.H_ve();
  }

  /// Sets a Ventilation property. Property name in .ism file: "h_ve". Property is optional (has a default).
  void setH_ve(double H_ve) {
    ventilation.setH_ve(H_ve);
  }

  /// Gets a Ventilation property. Property name in .ism file: "heatrecovery". Property is required.
  double heatRecovery() const {
    return ventilation.heatRecoveryEfficiency();
  }

  /// Sets a Ventilation property. Property name in .ism file: "heatrecovery". Property is required.
  void setHeatRecovery(double val) {
    ventilation.setHeatRecoveryEfficiency(val);
  }

  /// Gets a Ventilation property. Property name in .ism file: "hzone". Property is optional (has a default).
  double hzone() const {
    return ventilation.hzone();
  }

  /// Sets a Ventilation property. Property name in .ism file: "hzone". Property is optional (has a default).
  void setHzone(double hzone) {
    ventilation.setHzone(hzone);
  }

  /// Gets a Ventilation property. Property name in .ism file: "infiltrationRateUnoccupied". Property is required.
  double infiltrationRateUnoccupied() const {
    return ventilation.infiltrationRateUnoccupied();
  }

  /// Sets a Ventilation property. Property name in .ism file: "infiltrationRateUnoccupied". Property is required.
  void setInfiltrationRateUnoccupied(double infiltrationRateUnoccupied) {
    ventilation.setInfiltrationRateUnoccupied(infiltrationRateUnoccupied);
  }

  /// Gets a Ventilation property. Property name in .ism file: "n50". Property is optional (has a default).
  double n50() const {
    return ventilation.n50();
  }

  /// Sets a Ventilation property. Property name in .ism file: "n50". Property is optional (has a default).
  void setN50(double n50) {
    ventilation.setN50(n50);
  }

  /// Gets a Ventilation property. Property name in .ism file: "p_exp". Property is optional (has a default).
  double p_exp() const {
    return ventilation.p_exp();
  }

  /// Sets a Ventilation property. Property name in .ism file: "p_exp". Property is optional (has a default).
  void setP_exp(double p_exp) {
    ventilation.setP_exp(p_exp);
  }

  /// Gets a Ventilation property. Property name in .ism file: "specificfanpower". Property is required.
  double specificFanPower() const {
    return ventilation.fanPower();
  }

  /// Sets a Ventilation property. Property name in .ism file: "specificfanpower". Property is required.
  void setSpecificFanPower(double val) {
    ventilation.setFanPower(val);
  }

  /// Gets a Ventilation property. Property name in .ism file: "stack_coeff". Property is optional (has a default).
  double stack_coeff() const {
    return ventilation.stack_coeff();
  }

  /// Sets a Ventilation property. Property name in .ism file: "stack_coeff". Property is optional (has a default).
  void setStack_coeff(double stack_coeff) {
    ventilation.setStack_coeff(stack_coeff);
  }

  /// Gets a Ventilation property. Property name in .ism file: "stack_exp". Property is optional (has a default).
  double stack_exp() const {
    return ventilation.stack_exp();
  }

  /// Sets a Ventilation property. Property name in .ism file: "stack_exp". Property is optional (has a default).
  void setStack_exp(double stack_exp) {
    ventilation.setStack_exp(stack_exp);
  }

  /// Gets a Ventilation property. Property name in .ism file: "ventilationExhaustRateOccupied". Property is required.
  double supplyExhaustRate() const {
    return ventilation.supplyDifference();
  }

  /// Sets a Ventilation property. Property name in .ism file: "ventilationExhaustRateOccupied". Property is required.
  void setSupplyExhaustRate(double val) {
    ventilation.setSupplyDifference(val);
  }

  /// Gets a Ventilation property. Property name in .ism file: "vent_rate_flag". Property is optional (has a default).
  double vent_rate_flag() const {
    return ventilation.vent_rate_flag();
  }

  /// Sets a Ventilation property. Property name in .ism file: "vent_rate_flag". Property is optional (has a default).
  void setVent_rate_flag(int vent_rate_flag) {
    ventilation.setVent_rate_flag(vent_rate_flag);
  }

  /// Gets a Ventilation property. Property name in .ism file: "ventilationExhaustRateUnoccupied". Property is required.
  double ventilationExhaustRateUnoccupied() const {
    return ventilation.ventilationExhaustRateUnoccupied();
  }

  /// Sets a Ventilation property. Property name in .ism file: "ventilationExhaustRateUnoccupied". Property is required.
  void setVentilationExhaustRateUnoccupied(double ventilationExhaustRateUnoccupied) {
    ventilation.setVentilationExhaustRateUnoccupied(ventilationExhaustRateUnoccupied);
  }

  /// Gets a Ventilation property. Property name in .ism file: "ventilationIntakeRateUnoccupied". Property is required.
  double ventilationIntakeRateUnoccupied() const {
    return ventilation.ventilationIntakeRateUnoccupied();
  }

  /// Sets a Ventilation property. Property name in .ism file: "ventilationIntakeRateUnoccupied". Property is required.
  void setVentilationIntakeRateUnoccupied(double ventilationIntakeRateUnoccupied) {
    ventilation.setVentilationIntakeRateUnoccupied(ventilationIntakeRateUnoccupied);
  }

  /// Gets a Ventilation property.
  double ventilationType() const {
    return ventilation.ventType();
  }

  /// Sets a Ventilation property.
  void setVentilationType(std::string type) {
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);
    if (type == MECHANICAL)
      ventilation.setVentType(1.0);
    else if (type == COMBINED)
      ventilation.setVentType(2.0);
    else if (type == NATURAL)
      ventilation.setVentType(3.0);
    else
      throw std::invalid_argument("ventilationType parameter must be one of 'mechanical', 'natural', or 'combined'");
  }

  /// Gets a Ventilation property. Property name in .ism file: "ventpreheatdegc". Property is optional (has a default).
  double ventPreheatDegC() const {
    return ventilation.ventPreheatDegC();
  }

  /// Sets a Ventilation property. Property name in .ism file: "ventpreheatdegc". Property is optional (has a default).
  void setVentPreheatDegC(double ventPreheatDegC) {
    ventilation.setVentPreheatDegC(ventPreheatDegC);
  }

  /// Gets a Ventilation property. Property name in .ism file: "wind_coeff". Property is optional (has a default).
  double wind_coeff() const {
    return ventilation.wind_coeff();
  }

  /// Sets a Ventilation property. Property name in .ism file: "wind_coeff". Property is optional (has a default).
  void setWind_coeff(double wind_coeff) {
    ventilation.setWind_coeff(wind_coeff);
  }

  /// Gets a Ventilation property. Property name in .ism file: "wind_exp". Property is optional (has a default).
  double wind_exp() const {
    return ventilation.wind_exp();
  }

  /// Sets a Ventilation property. Property name in .ism file: "wind_exp". Property is optional (has a default).
  void setWind_exp(double wind_exp) {
    ventilation.setWind_exp(wind_exp);
  }

  /// Gets a Ventilation property. Property name in .ism file: "zone_frac". Property is optional (has a default).
  double zone_frac() const {
    return ventilation.zone_frac();
  }

  /// Sets a Ventilation property. Property name in .ism file: "zone_frac". Property is optional (has a default).
  void setZone_frac(double zone_frac) {
    ventilation.setZone_frac(zone_frac);
  }

private:

  void setCoreSimulationProperties(Simulation& sim) const;

  std::string resolveFilename(std::string baseFile, std::string relativeFile);
  void initializeStructure(const Properties& buildingParams);

  std::map<LatLon, std::shared_ptr<WeatherData>> _weather_cache;

  std::shared_ptr<WeatherData> _weather;
  std::shared_ptr<EpwData> _edata;

  Population pop;
  Location location;
  Lighting lights;
  Building building;
  Structure structure;
  Heating heating;
  Cooling cooling;
  Ventilation ventilation;
  // EpwData epwData; // XXX: Currently a shared_ptr already.
  PhysicalQuantities phys;
  SimulationSettings simSettings;

  bool _valid;

  std::string _weatherFilePath, _scheduleFilePath;
  std::string dataFile;

  void initializeParameters(const Properties& props);

  /**
   * Sets an .ism property in the usermodel to a value gotten from a Properties object.
   * Takes a pointer to the appropriate UserModel setter function, the Properties object,
   * the name of the property, and a boolean indicating if the property is required or
   * if it has a hardcoded fallback default. The overloads attempt to get the property by name
   * as the appropriate type from the Properties object depending on the argument type
   * needed by the setter function. If the call to initializeParameter has required=true then
   * it throws invalid_argument if it gets boost::none when calling props.getPropertyAs..., if it
   * is optional (required=false), then it does nothing if it gets boost::none (the UserModel
   * setter is not called).
   */
  void initializeParameter(void(UserModel::*setProp)(double), const Properties& props, std::string propertyName, bool required);
  void initializeParameter(void(UserModel::*setProp)(int), const Properties& props, std::string propertyName, bool required);
  void initializeParameter(void(UserModel::*setProp)(bool), const Properties& props, std::string propertyName, bool required);
  void initializeParameter(void(UserModel::*setProp)(const Vector&), const Properties& props, std::string propertyName, bool required);
  void initializeParameter(void(UserModel::*setProp)(std::string), const Properties& props, std::string propertyName, bool required);

  /**
   * .ism file is N, NE, E, SE, S, SW, W, NW, Roof.
   * Structure is S, SE, E, NE, N, NW, W, SW, Roof.
   * This reorders a vector from the .ism format to the Structure format.
   */
  void northToSouth(Vector& vec);

  void loadBuilding(std::string buildingFile);
  void loadBuilding(std::string buildingFile, std::string defaultsFile);
  int weatherState(std::string header);
  void initializeSolar();

};

} // isomodel
} // openstudio

#endif // ISOMODEL_USERMODEL_HPP
