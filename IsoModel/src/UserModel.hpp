/// @file UserModel.hpp
/// @brief High-level facade for loading building models and creating simulations.
///
/// Parses ISM (legacy) and YAML configuration files to populate all
/// component property objects (Structure, Heating, Cooling, Ventilation,
/// Lighting, Building, Population, Location, etc.). Provides factory
/// methods to create configured MonthlyModel and HourlyModel instances.
/// Supports optional default values and property overrides.
///
/// @author Brian Craig
/// @author Nick Collier
/// @author Brendan Albano
/// @author Ralph Muehleisen
/// @date 2013-11-05
/// @copyright Copyright Argonne National Laboratory
#pragma once
#include "Constants.hpp" // Added to access ConfigStrings and Physics constants
#include "EpwData.hpp"
#include "HourlyModel.hpp"
#include "ISOModelAPI.hpp"
#include "MonthlyModel.hpp"

#include "yaml-cpp/yaml.h"

#include <algorithm>
#include <filesystem>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

namespace openstudio::isomodel {

class MonthlyModel;
class WeatherData;

// NOTE: String constants (GAS, ELECTRIC, etc.) are now defined in Constants.hpp
// inside the openstudio::isomodel namespace.

struct ISOMODEL_API LatLon {
  double lat, lon;
  bool operator<(const LatLon &rhs) const;
};

class ISOMODEL_API UserModel {
public:
  // Use compiler-generated default constructor/destructor
  UserModel() = default;
  virtual ~UserModel() = default;

  /// Loads an ISO model from the specified .ism file
  void load(std::string buildingFile);

  /// Loads an ISO model file from the specified .ism file and defaults
  /// properties from the specified .ism.
  void load(std::string buildingFile, std::string defaultsFile);

  /// Loads the specified weather data from disk.
  /// Exposed to allow for separate loading from Ruby Scripts
  /// Call setWeatherFilePath(path) then loadWeather() to update
  /// the UserModel with a new set of weather data
  void loadWeather();

  /// Loads the weather from the specified array of doubles.
  void loadWeather(int block_size, double *weather_data);

  void loadAndSetWeather();

  /// Generates a MonthlyModel from the properties of the UserModel.
  [[nodiscard]] MonthlyModel toMonthlyModel() const;

  /// Generates an HourlyModel from the properties of the UserModel.
  [[nodiscard]] HourlyModel toHourlyModel() const;

  /// Indicates whether or not the user model loaded in correctly.
  [[nodiscard]] bool valid() const noexcept { return _valid; }

  // Validation
  void setValid(bool val) { _valid = val; }

  // ------------------------------------------------ //
  // Setters and getters for the isomodel properties. //
  // ------------------------------------------------ //

  /// Gets a EpwData property.
  [[nodiscard]] const std::shared_ptr<EpwData> epwData() const noexcept { return m_edata; }

  /// Gets a WeatherData property.
  [[nodiscard]] const std::shared_ptr<WeatherData> weatherData() const noexcept {
    return m_weather;
  }

  /// Gets a WeatherData property. Property name in .ism file:
  /// "weatherfilepath". Property is required.
  [[nodiscard]] std::string weatherFilePath() const { return m_weatherFilePath; }

  /// Sets a WeatherData property. Property name in .ism file:
  /// "weatherfilepath". Property is required.
  void setWeatherFilePath(std::string val) { m_weatherFilePath = std::move(val); }

  /// Gets a Building property.
  [[nodiscard]] std::string bemType() const {
    double val = building.buildingEnergyManagement();
    for (const auto &[key, adj] : BEM_TYPE_TO_ADJUSTMENT) {
      if (std::abs(val - adj) < 0.001)
        return key;
    }
    return "none";
  }

  /// Sets a Building property.
  void setBemType(std::string type) {
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);

    if (auto it = BEM_TYPE_TO_ADJUSTMENT.find(type); it != BEM_TYPE_TO_ADJUSTMENT.end()) {
      building.setBuildingEnergyManagement(it->second);
    } else {
      throw std::invalid_argument(
          "bemType parameter must be one of 'none', 'simple', or 'advanced'");
    }
  }

  /// Gets a Building property.
  [[nodiscard]] double buildingAirLeakage() const noexcept { return structure.infiltrationRate(); }

  /// Gets a Building property.
  [[nodiscard]] double buildingHeight() const noexcept { return structure.buildingHeight(); }

  /// Gets a Building property.
  [[nodiscard]] double buildingOccupancyFrom() const noexcept { return pop.daysStart(); }

  /// Gets a Building property.
  [[nodiscard]] double buildingOccupancyTo() const noexcept { return pop.daysEnd(); }

  /// Gets a Building property.
  [[nodiscard]] double constantIlluminationControl() const noexcept {
    return building.constantIllumination();
  }

  /// Sets a Building property.
  void setConstantIlluminationControl(double val) { building.setConstantIllumination(val); }

  /// Gets a Building property.
  [[nodiscard]] double elecPowerAppliancesOccupied() const noexcept {
    return building.electricApplianceHeatGainOccupied();
  }

  /// Sets a Building property.
  void setElecPowerAppliancesOccupied(double val) {
    building.setElectricApplianceHeatGainOccupied(val);
  }

  /// Gets a Building property.
  [[nodiscard]] double elecPowerAppliancesUnoccupied() const noexcept {
    return building.electricApplianceHeatGainUnoccupied();
  }

  /// Sets a Building property.
  void setElecPowerAppliancesUnoccupied(double val) {
    building.setElectricApplianceHeatGainUnoccupied(val);
  }

  /// Gets a Building property.
  [[nodiscard]] double electricAppliancePowerFixedOccupied() const noexcept {
    return building.electricAppliancePowerFixedOccupied();
  }

  /// Sets a Building property.
  void setElectricAppliancePowerFixedOccupied(double electricAppliancePowerFixedOccupied) {
    building.setElectricAppliancePowerFixedOccupied(electricAppliancePowerFixedOccupied);
  }

  /// Gets a Building property.
  [[nodiscard]] double electricAppliancePowerFixedUnoccupied() const noexcept {
    return building.electricAppliancePowerFixedUnoccupied();
  }

  /// Sets a Building property.
  void setElectricAppliancePowerFixedUnoccupied(double electricAppliancePowerFixedUnoccupied) {
    building.setElectricAppliancePowerFixedUnoccupied(electricAppliancePowerFixedUnoccupied);
  }

  /// Gets a Building property.
  [[nodiscard]] double externalEquipment() const noexcept { return building.externalEquipment(); }

  /// Sets a Building property.
  void setExternalEquipment(double externalEquipment) {
    building.setExternalEquipment(externalEquipment);
  }

  /// Gets a Building property.
  [[nodiscard]] double gasAppliancePowerFixedOccupied() const noexcept {
    return building.gasAppliancePowerFixedOccupied();
  }

  /// Sets a Building property.
  void setGasAppliancePowerFixedOccupied(double gasAppliancePowerFixedOccupied) {
    building.setGasAppliancePowerFixedOccupied(gasAppliancePowerFixedOccupied);
  }

  /// Gets a Building property.
  [[nodiscard]] double gasAppliancePowerFixedUnoccupied() const noexcept {
    return building.gasAppliancePowerFixedUnoccupied();
  }

  /// Sets a Building property.
  void setGasAppliancePowerFixedUnoccupied(double gasAppliancePowerFixedUnoccupied) {
    building.setGasAppliancePowerFixedUnoccupied(gasAppliancePowerFixedUnoccupied);
  }

  /// Gets a Building property.
  [[nodiscard]] double gasPowerAppliancesOccupied() const noexcept {
    return building.gasApplianceHeatGainOccupied();
  }

  /// Sets a Building property.
  void setGasPowerAppliancesOccupied(double val) { building.setGasApplianceHeatGainOccupied(val); }

  /// Gets a Building property.
  [[nodiscard]] double gasPowerAppliancesUnoccupied() const noexcept {
    return building.gasApplianceHeatGainUnoccupied();
  }

  /// Sets a Building property.
  void setGasPowerAppliancesUnoccupied(double val) {
    building.setGasApplianceHeatGainUnoccupied(val);
  }

  /// Gets a Building property.
  [[nodiscard]] double lightingOccupancySensorSystem() const noexcept {
    return building.lightingOccupancySensor();
  }

  /// Sets a Building property.
  void setLightingOccupancySensorSystem(double val) { building.setLightingOccupancySensor(val); }

  /// Gets a Cooling property.
  [[nodiscard]] double coolingOccupiedSetpoint() const noexcept {
    return cooling.temperatureSetPointOccupied();
  }

  /// Sets a Cooling property.
  void setCoolingOccupiedSetpoint(double val) { cooling.setTemperatureSetPointOccupied(val); }

  /// Gets a Cooling property.
  double coolingPumpControl() { return cooling.pumpControlReduction(); }

  /// Sets a Cooling property.
  void setCoolingPumpControl(double val) { cooling.setPumpControlReduction(val); }

  /// Gets a Cooling property.
  [[nodiscard]] double coolingSystemCOP() const noexcept { return cooling.cop(); }

  /// Sets a Cooling property.
  void setCoolingSystemCOP(double val) { cooling.setCop(val); }

  /// Gets a Cooling property.
  [[nodiscard]] double coolingSystemIPLVToCOPRatio() const noexcept {
    return cooling.partialLoadValue();
  }

  /// Sets a Cooling property.
  void setCoolingSystemIPLVToCOPRatio(double val) { cooling.setPartialLoadValue(val); }

  /// Gets a Cooling property.
  [[nodiscard]] double coolingUnoccupiedSetpoint() const noexcept {
    return cooling.temperatureSetPointUnoccupied();
  }

  /// Sets a Cooling property.
  void setCoolingUnoccupiedSetpoint(double val) { cooling.setTemperatureSetPointUnoccupied(val); }

  /// Gets a Cooling property.
  [[nodiscard]] double DC_YesNo() const noexcept { return cooling.DC_YesNo(); }

  /// Sets a Cooling property.
  void setDC_YesNo(double DC_YesNo) { cooling.setDC_YesNo(DC_YesNo); }

  /// Gets a Cooling property.
  [[nodiscard]] double dT_supp_cl() const noexcept { return cooling.dT_supp_cl(); }

  /// Sets a Cooling property.
  void setDT_supp_cl(double dT_supp_cl) { cooling.setDT_supp_cl(dT_supp_cl); }

  /// Gets a Cooling property.
  [[nodiscard]] double E_pumps_cl() const noexcept { return cooling.E_pumps(); }

  /// Sets a Cooling property.
  void setE_pumps_cl(double E_pumps) { cooling.setE_pumps(E_pumps); }

  /// Gets a Cooling property.
  [[nodiscard]] double eta_DC_COP_abs() const noexcept { return cooling.eta_DC_COP_abs(); }

  /// Sets a Cooling property.
  void setEta_DC_COP_abs(double eta_DC_COP_abs) { cooling.setEta_DC_COP_abs(eta_DC_COP_abs); }

  /// Gets a Cooling property.
  [[nodiscard]] double eta_DC_COP() const noexcept { return cooling.eta_DC_COP(); }

  /// Sets a Cooling property.
  void setEta_DC_COP(double eta_DC_COP) { cooling.setEta_DC_COP(eta_DC_COP); }

  /// Gets a Cooling property.
  [[nodiscard]] double eta_DC_frac_abs() const noexcept { return cooling.eta_DC_frac_abs(); }

  /// Sets a Cooling property.
  void setEta_DC_frac_abs(double eta_DC_frac_abs) { cooling.setEta_DC_frac_abs(eta_DC_frac_abs); }

  /// Gets a Cooling property.
  [[nodiscard]] double eta_DC_network() const noexcept { return cooling.eta_DC_network(); }

  /// Sets a Cooling property.
  void setEta_DC_network(double eta_DC_network) { cooling.setEta_DC_network(eta_DC_network); }

  /// Gets a Cooling property.
  [[nodiscard]] bool forcedAirCooling() const noexcept { return cooling.forcedAirCooling(); }

  /// Sets a Cooling property.
  void setForcedAirCooling(bool forcedAirCooling) { cooling.setForcedAirCooling(forcedAirCooling); }

  /// Gets a Cooling property.
  [[nodiscard]] double frac_DC_free() const noexcept { return cooling.frac_DC_free(); }

  /// Sets a Cooling property.
  void setFrac_DC_free(double frac_DC_free) { cooling.setFrac_DC_free(frac_DC_free); }

  /// Gets a Cooling property.
  double hvacCoolingLossFactor() { return cooling.hvacLossFactor(); }

  /// Sets a Cooling property.
  void setHvacCoolingLossFactor(double val) { cooling.setHvacLossFactor(val); }

  /// Gets a Cooling property.
  [[nodiscard]] double T_cl_ctrl_flag() const noexcept { return cooling.T_cl_ctrl_flag(); }

  /// Sets a Cooling property.
  void setT_cl_ctrl_flag(double T_cl_ctrl_flag) { cooling.setT_cl_ctrl_flag(T_cl_ctrl_flag); }

  /// Gets a Heating property.
  [[nodiscard]] double a_H0() const noexcept { return heating.a_H0(); }

  /// Sets a Heating property.
  void setA_H0(double a_H0) { heating.setA_H0(a_H0); }

  /// Gets a Heating property.
  [[nodiscard]] double DH_YesNo() const noexcept { return heating.DH_YesNo(); }

  /// Sets a Heating property.
  void setDH_YesNo(double DH_YesNo) { heating.setDH_YesNo(DH_YesNo); }

  /// Gets a Heating property.
  [[nodiscard]] double dhw_tset() const noexcept { return heating.dhw_tset(); }

  /// Sets a Heating property.
  void setDhw_tset(double dhw_tset) { heating.setDhw_tset(dhw_tset); }

  /// Gets a Heating property.
  [[nodiscard]] double dhw_tsupply() const noexcept { return heating.dhw_tsupply(); }

  /// Sets a Heating property.
  void setDhw_tsupply(double dhw_tsupply) { heating.setDhw_tsupply(dhw_tsupply); }

  /// Gets a Heating property.
  [[nodiscard]] double dhwDemand() const noexcept { return heating.hotWaterDemand(); }

  /// Sets a Heating property.
  void setDhwDemand(double val) { heating.setHotWaterDemand(val); }

  /// Gets a Heating property.
  double dhwDistributionEfficiency() { return heating.hotWaterDistributionEfficiency(); }

  /// Sets a Heating property.
  void setDhwDistributionEfficiency(double val) { heating.setHotWaterDistributionEfficiency(val); }

  /// Sets a Heating property.
  void setDhwDistributionSystem(double val) { heating.setHotWaterDistributionEfficiency(val); }

  /// Gets a Heating property.
  [[nodiscard]] double dhwEfficiency() const noexcept { return heating.hotWaterSystemEfficiency(); }

  /// Sets a Heating property.
  void setDhwEfficiency(double val) { heating.setHotWaterSystemEfficiency(val); }

  /// Gets a Heating property.
  [[nodiscard]] double dhwEnergyCarrier() const noexcept {
    return static_cast<double>(heating.hotWaterEnergyType());
  }

  /// Sets a Heating property.
  void setDhwEnergyCarrier(std::string type) {
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);
    if (auto it = STRING_TO_FUEL_TYPE.find(type); it != STRING_TO_FUEL_TYPE.end()) {
      heating.setHotWaterEnergyType(it->second);
    } else {
      throw std::invalid_argument("dhwFuelType parameter must be one of 'gas' or 'electric'");
    }
  }

  /// Gets a Heating property.
  [[nodiscard]] double dT_supp_ht() const noexcept { return heating.dT_supp_ht(); }

  /// Sets a Heating property.
  void setDT_supp_ht(double dT_supp_ht) { heating.setDT_supp_ht(dT_supp_ht); }

  /// Gets a Heating property.
  [[nodiscard]] double E_pumps_ht() const noexcept { return heating.E_pumps(); }

  /// Sets a Heating property.
  void setE_pumps_ht(double E_pumps) { heating.setE_pumps(E_pumps); }

  /// Gets a Heating property.
  [[nodiscard]] double eta_DH_network() const noexcept { return heating.eta_DH_network(); }

  /// Sets a Heating property.
  void setEta_DH_network(double eta_DH_network) { heating.setEta_DH_network(eta_DH_network); }

  /// Gets a Heating property.
  [[nodiscard]] double eta_DH_sys() const noexcept { return heating.eta_DH_sys(); }

  /// Sets a Heating property.
  void setEta_DH_sys(double eta_DH_sys) { heating.setEta_DH_sys(eta_DH_sys); }

  /// Gets a Heating property.
  [[nodiscard]] bool forcedAirHeating() const noexcept { return heating.forcedAirHeating(); }

  /// Sets a Heating property.
  void setForcedAirHeating(bool forcedAirHeating) { heating.setForcedAirHeating(forcedAirHeating); }

  /// Gets a Heating property.
  [[nodiscard]] double frac_DH_free() const noexcept { return heating.frac_DH_free(); }

  /// Sets a Heating property.
  void setFrac_DH_free(double frac_DH_free) { heating.setFrac_DH_free(frac_DH_free); }

  /// Gets a Heating property.
  [[nodiscard]] double heatingEnergyCarrier() const noexcept {
    return static_cast<double>(heating.energyType());
  }

  /// Sets a Heating property.
  void setHeatingEnergyCarrier(std::string type) {
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);
    if (auto it = STRING_TO_FUEL_TYPE.find(type); it != STRING_TO_FUEL_TYPE.end()) {
      heating.setEnergyType(it->second);
    } else {
      throw std::invalid_argument("heatingFuelType parameter must be one of 'gas' or 'electric'");
    }
  }

  /// Gets a Heating property.
  [[nodiscard]] double heatingOccupiedSetpoint() const noexcept {
    return heating.temperatureSetPointOccupied();
  }

  /// Sets a Heating property.
  void setHeatingOccupiedSetpoint(double val) { heating.setTemperatureSetPointOccupied(val); }

  /// Gets a Heating property.
  double heatingPumpControl() { return heating.pumpControlReduction(); }

  /// Sets a Heating property.
  void setHeatingPumpControl(double val) { heating.setPumpControlReduction(val); }

  /// Gets a Heating property.
  [[nodiscard]] double heatingSystemEfficiency() const noexcept { return heating.efficiency(); }

  /// Sets a Heating property.
  void setHeatingSystemEfficiency(double val) { heating.setEfficiency(val); }

  /// Gets a Heating property.
  [[nodiscard]] double heatingUnoccupiedSetpoint() const noexcept {
    return heating.temperatureSetPointUnoccupied();
  }

  /// Sets a Heating property.
  void setHeatingUnoccupiedSetpoint(double val) { heating.setTemperatureSetPointUnoccupied(val); }

  /// Gets a Heating property.
  double hvacHeatingLossFactor() { return heating.hvacLossFactor(); }

  /// Sets a Heating property.
  void setHvacHeatingLossFactor(double val) { heating.setHvacLossFactor(val); }

  /// Gets a Heating property.
  double hvacWasteFactor() { return heating.hotcoldWasteFactor(); }

  /// Sets a Heating property.
  void setHvacWasteFactor(double val) { heating.setHotcoldWasteFactor(val); }

  /// Gets a Heating property.
  [[nodiscard]] double T_ht_ctrl_flag() const noexcept { return heating.T_ht_ctrl_flag(); }

  /// Sets a Heating property.
  void setT_ht_ctrl_flag(double T_ht_ctrl_flag) { heating.setT_ht_ctrl_flag(T_ht_ctrl_flag); }

  /// Gets a Heating property.
  [[nodiscard]] double tau_H0() const noexcept { return heating.tau_H0(); }

  /// Sets a Heating property.
  void setTau_H0(double tau_H0) { heating.setTau_H0(tau_H0); }

  /// Gets a Lights property.
  [[nodiscard]] double automaticAd() const noexcept { return lights.automaticAd(); }

  /// Sets a Lights property.
  void setAutomaticAd(double automaticAd) { lights.setAutomaticAd(automaticAd); }

  /// Gets a Lights property.
  [[nodiscard]] double automaticLux() const noexcept { return lights.automaticLux(); }

  /// Sets a Lights property.
  void setAutomaticLux(double automaticLux) { lights.setAutomaticLux(automaticLux); }

  /// Gets a Lights property.
  [[nodiscard]] double daylightSensorSystem() const noexcept { return lights.dimmingFraction(); }

  /// Sets a Lights property.
  void setDaylightSensorSystem(double val) { lights.setDimmingFraction(val); }

  /// Gets a Lights property.
  [[nodiscard]] double elecInternalGains() const noexcept { return lights.elecInternalGains(); }

  /// Sets a Lights property.
  void setElecInternalGains(double elecInternalGains) {
    lights.setElecInternalGains(elecInternalGains);
  }

  /// Gets a Lights property.
  [[nodiscard]] double exteriorLightingPower() const noexcept { return lights.exteriorEnergy(); }

  /// Sets a Lights property.
  void setExteriorLightingPower(double val) { lights.setExteriorEnergy(val); }

  /// Gets a Lights property.
  [[nodiscard]] double lightingPowerFixedOccupied() const noexcept {
    return lights.lightingPowerFixedOccupied();
  }

  /// Sets a Lights property.
  void setLightingPowerFixedOccupied(double lightingPowerFixedOccupied) {
    lights.setLightingPowerFixedOccupied(lightingPowerFixedOccupied);
  }

  /// Gets a Lights property.
  [[nodiscard]] double lightingPowerFixedUnoccupied() const noexcept {
    return lights.lightingPowerFixedUnoccupied();
  }

  /// Sets a Lights property.
  void setLightingPowerFixedUnoccupied(double lightingPowerFixedUnoccupied) {
    lights.setLightingPowerFixedUnoccupied(lightingPowerFixedUnoccupied);
  }

  /// Gets a Lights property.
  [[nodiscard]] double lightingPowerIntensityOccupied() const noexcept {
    return lights.powerDensityOccupied();
  }

  /// Sets a Lights property.
  void setLightingPowerIntensityOccupied(double val) { lights.setPowerDensityOccupied(val); }

  /// Gets a Lights property.
  [[nodiscard]] double lightingPowerIntensityUnoccupied() const noexcept {
    return lights.powerDensityUnoccupied();
  }

  /// Sets a Lights property.
  void setLightingPowerIntensityUnoccupied(double val) { lights.setPowerDensityUnoccupied(val); }

  /// Gets a Lights property.
  [[nodiscard]] double manualSwitchAd() const noexcept { return lights.manualSwitchAd(); }

  /// Sets a Lights property.
  void setManualSwitchAd(double manualSwitchAd) { lights.setManualSwitchAd(manualSwitchAd); }

  /// Gets a Lights property.
  [[nodiscard]] double manualSwitchLux() const noexcept { return lights.manualSwitchLux(); }

  /// Sets a Lights property.
  void setManualSwitchLux(double manualSwitchLux) { lights.setManualSwitchLux(manualSwitchLux); }

  /// Gets a Lights property.
  [[nodiscard]] double n_day_end() const noexcept { return lights.n_day_end(); }

  /// Sets a Lights property.
  void setN_day_end(double n_day_end) { lights.setN_day_end(n_day_end); }

  /// Gets a Lights property.
  [[nodiscard]] double n_day_start() const noexcept { return lights.n_day_start(); }

  /// Sets a Lights property.
  void setN_day_start(double n_day_start) { lights.setN_day_start(n_day_start); }

  /// Gets a Lights property.
  [[nodiscard]] double n_weeks() const noexcept { return lights.n_weeks(); }

  /// Sets a Lights property.
  void setN_weeks(double n_weeks) { lights.setN_weeks(n_weeks); }

  /// Gets a Lights property.
  [[nodiscard]] double naturallyLightedArea() const noexcept {
    return lights.naturallyLightedArea();
  }

  /// Sets a Lights property.
  void setNaturallyLightedArea(double naturallyLightedArea) {
    lights.setNaturallyLightedArea(naturallyLightedArea);
  }

  /// Gets a Lights property.
  [[nodiscard]] double permLightPowerDensity() const noexcept {
    return lights.permLightPowerDensity();
  }

  /// Sets a Lights property.
  void setPermLightPowerDensity(double permLightPowerDensity) {
    lights.setPermLightPowerDensity(permLightPowerDensity);
  }

  /// Gets a Lights property.
  [[nodiscard]] double presenceAutoAd() const noexcept { return lights.presenceAutoAd(); }

  /// Sets a Lights property.
  void setPresenceAutoAd(double presenceAutoAd) { lights.setPresenceAutoAd(presenceAutoAd); }

  /// Gets a Lights property.
  [[nodiscard]] double presenceAutoLux() const noexcept { return lights.presenceAutoLux(); }

  /// Sets a Lights property.
  void setPresenceAutoLux(double presenceAutoLux) { lights.setPresenceAutoLux(presenceAutoLux); }

  /// Gets a Lights property.
  [[nodiscard]] double presenceSensorAd() const noexcept { return lights.presenceSensorAd(); }

  /// Sets a Lights property.
  void setPresenceSensorAd(double presenceSensorAd) {
    lights.setPresenceSensorAd(presenceSensorAd);
  }

  /// Gets a Lights property.
  [[nodiscard]] double presenceSensorLux() const noexcept { return lights.presenceSensorLux(); }

  /// Sets a Lights property.
  void setPresenceSensorLux(double presenceSensorLux) {
    lights.setPresenceSensorLux(presenceSensorLux);
  }

  /// Gets a Location property.
  [[nodiscard]] double terrainClass() const noexcept { return location.terrain(); }

  /// Sets a Location property.
  void setTerrainClass(double val) { location.setTerrain(val); }

  /// Gets a PhysicalQuantities property (now constants)
  // Removed accessors to PhysicalQuantities.hpp members as they are constants

  /// Sets a Population property.
  void setBuildingOccupancyFrom(double val) { pop.setDaysStart(val); }

  /// Sets a Population property.
  void setBuildingOccupancyTo(double val) { pop.setDaysEnd(val); }

  /// Gets a Population property.
  [[nodiscard]] double equivFullLoadOccupancyFrom() const noexcept { return pop.hoursStart(); }

  /// Sets a Population property.
  void setEquivFullLoadOccupancyFrom(double val) { pop.setHoursStart(val); }

  /// Gets a Population property.
  [[nodiscard]] double equivFullLoadOccupancyTo() const noexcept { return pop.hoursEnd(); }

  /// Sets a Population property.
  void setEquivFullLoadOccupancyTo(double val) { pop.setHoursEnd(val); }

  /// Gets a Population property.
  double heatGainPerPerson() { return pop.heatGainPerPerson(); }

  /// Sets a Population property.
  void setHeatGainPerPerson(double val) { pop.setHeatGainPerPerson(val); }

  /// Gets a Population property.
  [[nodiscard]] double peopleDensityOccupied() const noexcept { return pop.densityOccupied(); }

  /// Sets a Population property.
  void setPeopleDensityOccupied(double val) { pop.setDensityOccupied(val); }

  /// Gets a Population property.
  [[nodiscard]] double peopleDensityUnoccupied() const noexcept { return pop.densityUnoccupied(); }

  /// Sets a Population property.
  void setPeopleDensityUnoccupied(double val) { pop.setDensityUnoccupied(val); }

  /// Gets a Population property.
  [[nodiscard]] std::string scheduleFilePath() const { return pop.scheduleFilePath(); }

  /// Sets a Population property.
  void setScheduleFilePath(std::string scheduleFilePath) {
    pop.setScheduleFilePath(std::move(scheduleFilePath));
  }

  /// Gets a SimulationSettings property.
  [[nodiscard]] double hci() const noexcept { return simSettings.hci(); }

  /// Sets a SimulationSettings property.
  void setHci(double hci) { simSettings.setHci(hci); }

  /// Gets a SimulationSettings property.
  [[nodiscard]] double hri() const noexcept { return simSettings.hri(); }

  /// Sets a SimulationSettings property.
  void setHri(double hri) { simSettings.setHri(hri); }

  /// Gets a SimulationSettings property.
  [[nodiscard]] double phiIntFractionToAirNode() const noexcept {
    return simSettings.phiIntFractionToAirNode();
  }

  /// Sets a SimulationSettings property.
  void setPhiIntFractionToAirNode(double phiIntFractionToAirNode) {
    simSettings.setPhiIntFractionToAirNode(phiIntFractionToAirNode);
  }

  /// Gets a SimulationSettings property.
  [[nodiscard]] double phiSolFractionToAirNode() const noexcept {
    return simSettings.phiSolFractionToAirNode();
  }

  /// Sets a SimulationSettings property.
  void setPhiSolFractionToAirNode(double phiSolFractionToAirNode) {
    simSettings.setPhiSolFractionToAirNode(phiSolFractionToAirNode);
  }

  /// Sets a Structure property.
  void setBuildingAirLeakage(double val) { structure.setInfiltrationRate(val); }

  /// Sets a Structure property.
  void setBuildingHeight(double val) { structure.setBuildingHeight(val); }

  /// Gets a Structure property.
  double exteriorHeatCapacity() { return structure.wallHeatCapacity(); }

  /// Sets a Structure property.
  void setExteriorHeatCapacity(double val) { structure.setWallHeatCapacity(val); }

  /// Gets a Structure property.
  [[nodiscard]] double floorArea() const noexcept { return structure.floorArea(); }

  /// Sets a Structure property.
  void setFloorArea(double val) { structure.setFloorArea(val); }

  /// Gets a Structure property.
  [[nodiscard]] double interiorHeatCapacity() const noexcept {
    return structure.interiorHeatCapacity();
  }

  /// Sets a Structure property.
  void setInteriorHeatCapacity(double val) { structure.setInteriorHeatCapacity(val); }

  /// Gets a Structure property.
  [[nodiscard]] double irradianceForMaxShadingUse() const noexcept {
    return structure.irradianceForMaxShadingUse();
  }

  /// Sets a Structure property.
  void setIrradianceForMaxShadingUse(double irradianceForMaxShadingUse) {
    structure.setIrradianceForMaxShadingUse(irradianceForMaxShadingUse);
  }

  /// Gets a Structure property.
  [[nodiscard]] double R_sc_ext() const noexcept { return structure.R_sc_ext(); }

  /// Sets a Structure property.
  void setR_sc_ext(double R_sc_ext) { structure.setR_sc_ext(R_sc_ext); }

  /// Gets a Structure property.
  [[nodiscard]] double R_se() const noexcept { return structure.R_se(); }

  /// Sets a Structure property.
  void setR_se(double R_se) { structure.setR_se(R_se); }

  /// Gets a Structure property.
  double roofArea() { return structure.wallArea()[8]; }

  /// Sets a Structure property.
  void setRoofArea(double val) { structure.setWallArea(8, val); }

  /// Gets a Structure property.
  [[nodiscard]] double roofSolarAbsorption() const noexcept {
    return structure.wallSolarAbsorption()[8];
  }

  /// Sets a Structure property.
  void setRoofSolarAbsorption(double val) { structure.setWallSolarAbsorption(8, val); }

  /// Gets a Structure property.
  [[nodiscard]] double roofThermalEmissivity() const noexcept {
    return structure.wallThermalEmissivity()[8];
  }

  /// Sets a Structure property.
  void setRoofThermalEmissivity(double val) { structure.setWallThermalEmissivity(8, val); }

  /// Gets a Structure property.
  [[nodiscard]] double roofUValue() const noexcept { return structure.wallUniform()[8]; }

  /// Sets a Structure property.
  void setRoofUValue(double val) { structure.setWallUniform(8, val); }

  /// Gets a Structure property.
  [[nodiscard]] double shadingFactorAtMaxUse() const noexcept {
    return structure.shadingFactorAtMaxUse();
  }

  /// Sets a Structure property.
  void setShadingFactorAtMaxUse(double shadingFactorAtMaxUse) {
    structure.setShadingFactorAtMaxUse(shadingFactorAtMaxUse);
  }

  /// Gets a Structure property.
  double skylightArea() { return structure.windowArea()[8]; }

  /// Sets a Structure property.
  void setSkylightArea(double val) { structure.setWindowArea(8, val); }

  /// Gets a Structure property.
  double skylightSCF() { return structure.windowShadingCorrectionFactor()[8]; }

  /// Sets a Structure property.
  void setSkylightSCF(double val) { structure.setWindowShadingCorrectionFactor(8, val); }

  /// Gets a Structure property.
  [[nodiscard]] double skylightSDF() const noexcept { return structure.windowShadingDevice()[8]; }

  /// Sets a Structure property.
  void setSkylightSDF(double val) { structure.setWindowShadingDevice(8, val); }

  /// Gets a Structure property.
  double skylightSHGC() { return structure.windowNormalIncidenceSolarEnergyTransmittance()[8]; }

  /// Sets a Structure property.
  void setSkylightSHGC(double val) {
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(8, val);
  }

  /// Gets a Structure property.
  double skylightUvalue() { return structure.windowUniform()[8]; }

  /// Sets a Structure property.
  void setSkylightUvalue(double val) { structure.setWindowUniform(8, val); }

  /// Gets a Structure property.
  [[nodiscard]] double totalAreaPerFloorArea() const noexcept {
    return structure.totalAreaPerFloorArea();
  }

  /// Sets a Structure property.
  void setTotalAreaPerFloorArea(double totalAreaPerFloorArea) {
    structure.setTotalAreaPerFloorArea(totalAreaPerFloorArea);
  }

  /// Sets a Structure property.
  void setWallArea(const Vector &vec) {
    if (vec.size() != 9) {
      throw std::invalid_argument(
          "Invalid number of values for WallArea parameter. It must have 9.");
    }
    structure.setWallArea(vec);
  }

  /// Gets a Structure property.
  double wallAreaE() { return structure.wallArea()[2]; }

  /// Sets a Structure property.
  void setWallAreaE(double val) { structure.setWallArea(2, val); }

  /// Gets a Structure property.
  double wallAreaN() { return structure.wallArea()[4]; }

  /// Sets a Structure property.
  void setWallAreaN(double val) { structure.setWallArea(4, val); }

  /// Gets a Structure property.
  double wallAreaNE() { return structure.wallArea()[3]; }

  /// Sets a Structure property.
  void setWallAreaNE(double val) { structure.setWallArea(3, val); }

  /// Gets a Structure property.
  double wallAreaNW() { return structure.wallArea()[5]; }

  /// Sets a Structure property.
  void setWallAreaNW(double val) { structure.setWallArea(5, val); }

  /// Gets a Structure property.
  double wallAreaS() { return structure.wallArea()[0]; }

  /// Sets a Structure property.
  void setWallAreaS(double val) { structure.setWallArea(0, val); }

  /// Gets a Structure property.
  double wallAreaSE() { return structure.wallArea()[1]; }

  /// Sets a Structure property.
  void setWallAreaSE(double val) { structure.setWallArea(1, val); }

  /// Gets a Structure property.
  double wallAreaSW() { return structure.wallArea()[7]; }

  /// Sets a Structure property.
  void setWallAreaSW(double val) { structure.setWallArea(7, val); }

  /// Gets a Structure property.
  double wallAreaW() { return structure.wallArea()[6]; }

  /// Sets a Structure property.
  void setWallAreaW(double val) { structure.setWallArea(6, val); }

  /// Sets a Structure property.
  void setWallSolarAbsorption(const Vector &vec) {
    if (vec.size() != 9) {
      throw std::invalid_argument(
          "Invalid number of values for WallSolarAbsorption parameter. It must "
          "have 9.");
    }
    structure.setWallSolarAbsorption(vec);
  }

  /// Gets a Structure property.
  [[nodiscard]] double wallSolarAbsorptionE() const noexcept {
    return structure.wallSolarAbsorption()[2];
  }

  /// Sets a Structure property.
  void setWallSolarAbsorptionE(double val) { structure.setWallSolarAbsorption(2, val); }

  /// Gets a Structure property.
  [[nodiscard]] double wallSolarAbsorptionN() const noexcept {
    return structure.wallSolarAbsorption()[4];
  }

  /// Sets a Structure property.
  void setWallSolarAbsorptionN(double val) { structure.setWallSolarAbsorption(4, val); }

  /// Gets a Structure property.
  [[nodiscard]] double wallSolarAbsorptionNE() const noexcept {
    return structure.wallSolarAbsorption()[3];
  }

  /// Sets a Structure property.
  void setWallSolarAbsorptionNE(double val) { structure.setWallSolarAbsorption(3, val); }

  /// Gets a Structure property.
  [[nodiscard]] double wallSolarAbsorptionNW() const noexcept {
    return structure.wallSolarAbsorption()[5];
  }

  /// Sets a Structure property.
  void setWallSolarAbsorptionNW(double val) { structure.setWallSolarAbsorption(5, val); }

  /// Gets a Structure property.
  [[nodiscard]] double wallSolarAbsorptionS() const noexcept {
    return structure.wallSolarAbsorption()[0];
  }

  /// Sets a Structure property.
  void setWallSolarAbsorptionS(double val) { structure.setWallSolarAbsorption(0, val); }

  /// Gets a Structure property.
  [[nodiscard]] double wallSolarAbsorptionSE() const noexcept {
    return structure.wallSolarAbsorption()[1];
  }

  /// Sets a Structure property.
  void setWallSolarAbsorptionSE(double val) { structure.setWallSolarAbsorption(1, val); }

  /// Gets a Structure property.
  [[nodiscard]] double wallSolarAbsorptionSW() const noexcept {
    return structure.wallSolarAbsorption()[7];
  }

  /// Sets a Structure property.
  void setWallSolarAbsorptionSW(double val) { structure.setWallSolarAbsorption(7, val); }

  /// Gets a Structure property.
  [[nodiscard]] double wallSolarAbsorptionW() const noexcept {
    return structure.wallSolarAbsorption()[6];
  }

  /// Sets a Structure property.
  void setWallSolarAbsorptionW(double val) { structure.setWallSolarAbsorption(6, val); }

  /// Sets a Structure property.
  void setWallThermalEmissivity(const Vector &vec) {
    if (vec.size() != 9) {
      throw std::invalid_argument(
          "Invalid number of values for WallThermalEmissivity parameter. It "
          "must have 9.");
    }
    structure.setWallThermalEmissivity(vec);
  }

  /// Gets a Structure property.
  [[nodiscard]] double wallThermalEmissivityE() const noexcept {
    return structure.wallThermalEmissivity()[2];
  }

  /// Sets a Structure property.
  void setWallThermalEmissivityE(double val) { structure.setWallThermalEmissivity(2, val); }

  /// Gets a Structure property.
  [[nodiscard]] double wallThermalEmissivityN() const noexcept {
    return structure.wallThermalEmissivity()[4];
  }

  /// Sets a Structure property.
  void setWallThermalEmissivityN(double val) { structure.setWallThermalEmissivity(4, val); }

  /// Gets a Structure property.
  [[nodiscard]] double wallThermalEmissivityNE() const noexcept {
    return structure.wallThermalEmissivity()[3];
  }

  /// Sets a Structure property.
  void setWallThermalEmissivityNE(double val) { structure.setWallThermalEmissivity(3, val); }

  /// Gets a Structure property.
  [[nodiscard]] double wallThermalEmissivityNW() const noexcept {
    return structure.wallThermalEmissivity()[5];
  }

  /// Sets a Structure property.
  void setWallThermalEmissivityNW(double val) { structure.setWallThermalEmissivity(5, val); }

  /// Gets a Structure property.
  [[nodiscard]] double wallThermalEmissivityS() const noexcept {
    return structure.wallThermalEmissivity()[0];
  }

  /// Sets a Structure property.
  void setWallThermalEmissivityS(double val) { structure.setWallThermalEmissivity(0, val); }

  /// Gets a Structure property.
  [[nodiscard]] double wallThermalEmissivitySE() const noexcept {
    return structure.wallThermalEmissivity()[1];
  }

  /// Sets a Structure property.
  void setWallThermalEmissivitySE(double val) { structure.setWallThermalEmissivity(1, val); }

  /// Gets a Structure property.
  [[nodiscard]] double wallThermalEmissivitySW() const noexcept {
    return structure.wallThermalEmissivity()[7];
  }

  /// Sets a Structure property.
  void setWallThermalEmissivitySW(double val) { structure.setWallThermalEmissivity(7, val); }

  /// Gets a Structure property.
  [[nodiscard]] double wallThermalEmissivityW() const noexcept {
    return structure.wallThermalEmissivity()[6];
  }

  /// Sets a Structure property.
  void setWallThermalEmissivityW(double val) { structure.setWallThermalEmissivity(6, val); }

  /// Sets a Structure property.
  void setWallU(const Vector &vec) {
    if (vec.size() != 9) {
      throw std::invalid_argument("Invalid number of values for WallU parameter. It must have 9.");
    }
    structure.setWallUniform(vec);
  }

  /// Gets a Structure property.
  [[nodiscard]] double wallUvalueE() const noexcept { return structure.wallUniform()[2]; }

  /// Sets a Structure property.
  void setWallUvalueE(double val) { structure.setWallUniform(2, val); }

  /// Gets a Structure property.
  [[nodiscard]] double wallUvalueN() const noexcept { return structure.wallUniform()[4]; }

  /// Sets a Structure property.
  void setWallUvalueN(double val) { structure.setWallUniform(4, val); }

  /// Gets a Structure property.
  [[nodiscard]] double wallUvalueNE() const noexcept { return structure.wallUniform()[3]; }

  /// Sets a Structure property.
  void setWallUvalueNE(double val) { structure.setWallUniform(3, val); }

  /// Gets a Structure property.
  [[nodiscard]] double wallUvalueNW() const noexcept { return structure.wallUniform()[5]; }

  /// Sets a Structure property.
  void setWallUvalueNW(double val) { structure.setWallUniform(5, val); }

  /// Gets a Structure property.
  [[nodiscard]] double wallUvalueS() const noexcept { return structure.wallUniform()[0]; }

  /// Sets a Structure property.
  void setWallUvalueS(double val) { structure.setWallUniform(0, val); }

  /// Gets a Structure property.
  [[nodiscard]] double wallUvalueSE() const noexcept { return structure.wallUniform()[1]; }

  /// Sets a Structure property.
  void setWallUvalueSE(double val) { structure.setWallUniform(1, val); }

  /// Gets a Structure property.
  [[nodiscard]] double wallUvalueSW() const noexcept { return structure.wallUniform()[7]; }

  /// Sets a Structure property.
  void setWallUvalueSW(double val) { structure.setWallUniform(7, val); }

  /// Gets a Structure property.
  [[nodiscard]] double wallUvalueW() const noexcept { return structure.wallUniform()[6]; }

  /// Sets a Structure property.
  void setWallUvalueW(double val) { structure.setWallUniform(6, val); }

  /// Gets a Structure property.
  [[nodiscard]] double win_F_W() const noexcept { return structure.win_F_W(); }

  /// Sets a Structure property.
  void setWin_F_W(double win_F_W) { structure.setWin_F_W(win_F_W); }

  /// Gets a Structure property.
  [[nodiscard]] double win_ff() const noexcept { return structure.win_ff(); }

  /// Sets a Structure property.
  void setWin_ff(double win_ff) { structure.setWin_ff(win_ff); }

  /// Sets a Structure property.
  void setWindowArea(const Vector &vec) {
    if (vec.size() != 9) {
      throw std::invalid_argument(
          "Invalid number of values for WindowArea parameter. It must have 9.");
    }
    structure.setWindowArea(vec);
  }

  /// Gets a Structure property.
  double windowAreaE() { return structure.windowArea()[2]; }

  /// Sets a Structure property.
  void setWindowAreaE(double val) { structure.setWindowArea(2, val); }

  /// Gets a Structure property.
  double windowAreaN() { return structure.windowArea()[4]; }

  /// Sets a Structure property.
  void setWindowAreaN(double val) { structure.setWindowArea(4, val); }

  /// Gets a Structure property.
  double windowAreaNE() { return structure.windowArea()[3]; }

  /// Sets a Structure property.
  void setWindowAreaNE(double val) { structure.setWindowArea(3, val); }

  /// Gets a Structure property.
  double windowAreaNW() { return structure.windowArea()[5]; }

  /// Sets a Structure property.
  void setWindowAreaNW(double val) { structure.setWindowArea(5, val); }

  /// Gets a Structure property.
  double windowAreaS() { return structure.windowArea()[0]; }

  /// Sets a Structure property.
  void setWindowAreaS(double val) { structure.setWindowArea(0, val); }

  /// Gets a Structure property.
  double windowAreaSE() { return structure.windowArea()[1]; }

  /// Sets a Structure property.
  void setWindowAreaSE(double val) { structure.setWindowArea(1, val); }

  /// Gets a Structure property.
  double windowAreaSW() { return structure.windowArea()[7]; }

  /// Sets a Structure property.
  void setWindowAreaSW(double val) { structure.setWindowArea(7, val); }

  /// Gets a Structure property.
  double windowAreaW() { return structure.windowArea()[6]; }

  /// Sets a Structure property.
  void setWindowAreaW(double val) { structure.setWindowArea(6, val); }

  /// Sets a Structure property.
  void setWindowSCF(const Vector &vec) {
    if (vec.size() != 9) {
      throw std::invalid_argument(
          "Invalid number of values for WindowSCF parameter. It must have 9.");
    }
    structure.setWindowShadingCorrectionFactor(vec);
  }

  /// Gets a Structure property.
  [[nodiscard]] double windowSCFE() const noexcept {
    return structure.windowShadingCorrectionFactor()[2];
  }

  /// Sets a Structure property.
  void setWindowSCFE(double val) { structure.setWindowShadingCorrectionFactor(2, val); }

  /// Gets a Structure property.
  [[nodiscard]] double windowSCFN() const noexcept {
    return structure.windowShadingCorrectionFactor()[4];
  }

  /// Sets a Structure property.
  void setWindowSCFN(double val) { structure.setWindowShadingCorrectionFactor(4, val); }

  /// Gets a Structure property.
  [[nodiscard]] double windowSCFNE() const noexcept {
    return structure.windowShadingCorrectionFactor()[3];
  }

  /// Sets a Structure property.
  void setWindowSCFNE(double val) { structure.setWindowShadingCorrectionFactor(3, val); }

  /// Gets a Structure property.
  [[nodiscard]] double windowSCFNW() const noexcept {
    return structure.windowShadingCorrectionFactor()[5];
  }

  /// Sets a Structure property.
  void setWindowSCFNW(double val) { structure.setWindowShadingCorrectionFactor(5, val); }

  /// Gets a Structure property.
  [[nodiscard]] double windowSCFS() const noexcept {
    return structure.windowShadingCorrectionFactor()[0];
  }

  /// Sets a Structure property.
  void setWindowSCFS(double val) { structure.setWindowShadingCorrectionFactor(0, val); }

  /// Gets a Structure property.
  [[nodiscard]] double windowSCFSE() const noexcept {
    return structure.windowShadingCorrectionFactor()[1];
  }

  /// Sets a Structure property.
  void setWindowSCFSE(double val) { structure.setWindowShadingCorrectionFactor(1, val); }

  /// Gets a Structure property.
  [[nodiscard]] double windowSCFSW() const noexcept {
    return structure.windowShadingCorrectionFactor()[7];
  }

  /// Sets a Structure property.
  void setWindowSCFSW(double val) { structure.setWindowShadingCorrectionFactor(7, val); }

  /// Gets a Structure property.
  [[nodiscard]] double windowSCFW() const noexcept {
    return structure.windowShadingCorrectionFactor()[6];
  }

  /// Sets a Structure property.
  void setWindowSCFW(double val) { structure.setWindowShadingCorrectionFactor(6, val); }

  /// Sets a Structure property.
  void setWindowSDF(const Vector &vec) {
    if (vec.size() != 9) {
      throw std::invalid_argument(
          "Invalid number of values for WindowSDF parameter. It must have 9.");
    }
    structure.setWindowShadingDevice(vec);
  }

  /// Gets a Structure property.
  [[nodiscard]] double windowSDFE() const noexcept { return structure.windowShadingDevice()[2]; }

  /// Sets a Structure property.
  void setWindowSDFE(double val) { structure.setWindowShadingDevice(2, val); }

  /// Gets a Structure property.
  [[nodiscard]] double windowSDFN() const noexcept { return structure.windowShadingDevice()[4]; }

  /// Sets a Structure property.
  void setWindowSDFN(double val) { structure.setWindowShadingDevice(4, val); }

  /// Gets a Structure property.
  [[nodiscard]] double windowSDFNE() const noexcept { return structure.windowShadingDevice()[3]; }

  /// Sets a Structure property.
  void setWindowSDFNE(double val) { structure.setWindowShadingDevice(3, val); }

  /// Gets a Structure property.
  [[nodiscard]] double windowSDFNW() const noexcept { return structure.windowShadingDevice()[5]; }

  /// Sets a Structure property.
  void setWindowSDFNW(double val) { structure.setWindowShadingDevice(5, val); }

  /// Gets a Structure property.
  [[nodiscard]] double windowSDFS() const noexcept { return structure.windowShadingDevice()[0]; }

  /// Sets a Structure property.
  void setWindowSDFS(double val) { structure.setWindowShadingDevice(0, val); }

  /// Gets a Structure property.
  [[nodiscard]] double windowSDFSE() const noexcept { return structure.windowShadingDevice()[1]; }

  /// Sets a Structure property.
  void setWindowSDFSE(double val) { structure.setWindowShadingDevice(1, val); }

  /// Gets a Structure property.
  [[nodiscard]] double windowSDFSW() const noexcept { return structure.windowShadingDevice()[7]; }

  /// Sets a Structure property.
  void setWindowSDFSW(double val) { structure.setWindowShadingDevice(7, val); }

  /// Gets a Structure property.
  [[nodiscard]] double windowSDFW() const noexcept { return structure.windowShadingDevice()[6]; }

  /// Sets a Structure property.
  void setWindowSDFW(double val) { structure.setWindowShadingDevice(6, val); }

  /// Sets a Structure property.
  void setWindowSHGC(const Vector &vec) {
    if (vec.size() != 9) {
      throw std::invalid_argument(
          "Invalid number of values for WindowSHGC parameter. It must have 9.");
    }
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(vec);
  }

  /// Gets a Structure property.
  [[nodiscard]] double windowSHGCE() const noexcept {
    return structure.windowNormalIncidenceSolarEnergyTransmittance()[2];
  }

  /// Sets a Structure property.
  void setWindowSHGCE(double val) {
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(2, val);
  }

  /// Gets a Structure property.
  [[nodiscard]] double windowSHGCN() const noexcept {
    return structure.windowNormalIncidenceSolarEnergyTransmittance()[4];
  }

  /// Sets a Structure property.
  void setWindowSHGCN(double val) {
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(4, val);
  }

  /// Gets a Structure property.
  [[nodiscard]] double windowSHGCNE() const noexcept {
    return structure.windowNormalIncidenceSolarEnergyTransmittance()[3];
  }

  /// Sets a Structure property.
  void setWindowSHGCNE(double val) {
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(3, val);
  }

  /// Gets a Structure property.
  [[nodiscard]] double windowSHGCNW() const noexcept {
    return structure.windowNormalIncidenceSolarEnergyTransmittance()[5];
  }

  /// Sets a Structure property.
  void setWindowSHGCNW(double val) {
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(5, val);
  }

  /// Gets a Structure property.
  [[nodiscard]] double windowSHGCS() const noexcept {
    return structure.windowNormalIncidenceSolarEnergyTransmittance()[0];
  }

  /// Sets a Structure property.
  void setWindowSHGCS(double val) {
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(0, val);
  }

  /// Gets a Structure property.
  [[nodiscard]] double windowSHGCSE() const noexcept {
    return structure.windowNormalIncidenceSolarEnergyTransmittance()[1];
  }

  /// Sets a Structure property.
  void setWindowSHGCSE(double val) {
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(1, val);
  }

  /// Gets a Structure property.
  [[nodiscard]] double windowSHGCSW() const noexcept {
    return structure.windowNormalIncidenceSolarEnergyTransmittance()[7];
  }

  /// Sets a Structure property.
  void setWindowSHGCSW(double val) {
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(7, val);
  }

  /// Gets a Structure property.
  [[nodiscard]] double windowSHGCW() const noexcept {
    return structure.windowNormalIncidenceSolarEnergyTransmittance()[6];
  }

  /// Sets a Structure property.
  void setWindowSHGCW(double val) {
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(6, val);
  }

  /// Sets a Structure property.
  void setWindowU(const Vector &vec) {
    if (vec.size() != 9) {
      throw std::invalid_argument(
          "Invalid number of values for WindowU parameter. It must have 9.");
    }
    structure.setWindowUniform(vec);
  }

  /// Gets a Structure property.
  [[nodiscard]] double windowUvalueE() const noexcept { return structure.windowUniform()[2]; }

  /// Sets a Structure property.
  void setWindowUvalueE(double val) { structure.setWindowUniform(2, val); }

  /// Gets a Structure property.
  [[nodiscard]] double windowUvalueN() const noexcept { return structure.windowUniform()[4]; }

  /// Sets a Structure property.
  void setWindowUvalueN(double val) { structure.setWindowUniform(4, val); }

  /// Gets a Structure property.
  [[nodiscard]] double windowUvalueNE() const noexcept { return structure.windowUniform()[3]; }

  /// Sets a Structure property.
  void setWindowUvalueNE(double val) { structure.setWindowUniform(3, val); }

  /// Gets a Structure property.
  [[nodiscard]] double windowUvalueNW() const noexcept { return structure.windowUniform()[5]; }

  /// Sets a Structure property.
  void setWindowUvalueNW(double val) { structure.setWindowUniform(5, val); }

  /// Gets a Structure property.
  [[nodiscard]] double windowUvalueS() const noexcept { return structure.windowUniform()[0]; }

  /// Sets a Structure property.
  void setWindowUvalueS(double val) { structure.setWindowUniform(0, val); }

  /// Gets a Structure property.
  [[nodiscard]] double windowUvalueSE() const noexcept { return structure.windowUniform()[1]; }

  /// Sets a Structure property.
  void setWindowUvalueSE(double val) { structure.setWindowUniform(1, val); }

  /// Gets a Structure property.
  [[nodiscard]] double windowUvalueSW() const noexcept { return structure.windowUniform()[7]; }

  /// Sets a Structure property.
  void setWindowUvalueSW(double val) { structure.setWindowUniform(7, val); }

  /// Gets a Structure property.
  [[nodiscard]] double windowUvalueW() const noexcept { return structure.windowUniform()[6]; }

  /// Sets a Structure property.
  void setWindowUvalueW(double val) { structure.setWindowUniform(6, val); }

  /// Gets a Ventilation property.
  [[nodiscard]] double dCp() const noexcept { return ventilation.dCp(); }

  /// Sets a Ventilation property.
  void setDCp(double dCp) { ventilation.setDCp(dCp); }

  /// Gets a Ventilation property.
  [[nodiscard]] double exhaustAirRecirclation() const noexcept {
    return ventilation.exhaustAirRecirculated();
  }

  /// Sets a Ventilation property.
  void setExhaustAirRecirclation(double val) { ventilation.setExhaustAirRecirculated(val); }

  /// Gets a Ventilation property.
  [[nodiscard]] double fanFlowControlFactor() const noexcept {
    return ventilation.fanControlFactor();
  }

  /// Sets a Ventilation property.
  void setFanFlowControlFactor(double val) { ventilation.setFanControlFactor(val); }

  /// Gets a Ventilation property.
  [[nodiscard]] double freshAirFlowRate() const noexcept { return ventilation.supplyRate(); }

  /// Sets a Ventilation property.
  void setFreshAirFlowRate(double val) { ventilation.setSupplyRate(val); }

  /// Gets a Ventilation property.
  [[nodiscard]] double H_ve() const noexcept { return ventilation.H_ve(); }

  /// Sets a Ventilation property.
  void setH_ve(double H_ve) { ventilation.setH_ve(H_ve); }

  /// Gets a Ventilation property.
  [[nodiscard]] double heatRecovery() const noexcept {
    return ventilation.heatRecoveryEfficiency();
  }

  /// Sets a Ventilation property.
  void setHeatRecovery(double val) { ventilation.setHeatRecoveryEfficiency(val); }

  /// Gets a Ventilation property.
  [[nodiscard]] double hzone() const noexcept { return ventilation.hzone(); }

  /// Sets a Ventilation property.
  void setHzone(double hzone) { ventilation.setHzone(hzone); }

  /// Gets a Ventilation property.
  [[nodiscard]] double infiltrationRateUnoccupied() const noexcept {
    return ventilation.infiltrationRateUnoccupied();
  }

  /// Sets a Ventilation property.
  void setInfiltrationRateUnoccupied(double infiltrationRateUnoccupied) {
    ventilation.setInfiltrationRateUnoccupied(infiltrationRateUnoccupied);
  }

  /// Gets a Ventilation property.
  [[nodiscard]] double n50() const noexcept { return ventilation.n50(); }

  /// Sets a Ventilation property.
  void setN50(double n50) { ventilation.setN50(n50); }

  /// Gets a Ventilation property.
  [[nodiscard]] double p_exp() const noexcept { return ventilation.p_exp(); }

  /// Sets a Ventilation property.
  void setP_exp(double p_exp) { ventilation.setP_exp(p_exp); }

  /// Gets a Ventilation property.
  [[nodiscard]] double specificFanPower() const noexcept { return ventilation.fanPower(); }

  /// Sets a Ventilation property.
  void setSpecificFanPower(double val) { ventilation.setFanPower(val); }

  /// Gets a Ventilation property.
  [[nodiscard]] double stack_coeff() const noexcept { return ventilation.stack_coeff(); }

  /// Sets a Ventilation property.
  void setStack_coeff(double stack_coeff) { ventilation.setStack_coeff(stack_coeff); }

  /// Gets a Ventilation property.
  [[nodiscard]] double stack_exp() const noexcept { return ventilation.stack_exp(); }

  /// Sets a Ventilation property.
  void setStack_exp(double stack_exp) { ventilation.setStack_exp(stack_exp); }

  /// Gets a Ventilation property.
  [[nodiscard]] double supplyExhaustRate() const noexcept { return ventilation.supplyDifference(); }

  /// Sets a Ventilation property.
  void setSupplyExhaustRate(double val) { ventilation.setSupplyDifference(val); }

  /// Gets a Ventilation property.
  [[nodiscard]] double vent_rate_flag() const noexcept { return ventilation.vent_rate_flag(); }

  /// Sets a Ventilation property.
  void setVent_rate_flag(int vent_rate_flag) { ventilation.setVent_rate_flag(vent_rate_flag); }

  /// Gets a Ventilation property.
  [[nodiscard]] double ventilationExhaustRateUnoccupied() const noexcept {
    return ventilation.ventilationExhaustRateUnoccupied();
  }

  /// Sets a Ventilation property.
  void setVentilationExhaustRateUnoccupied(double ventilationExhaustRateUnoccupied) {
    ventilation.setVentilationExhaustRateUnoccupied(ventilationExhaustRateUnoccupied);
  }

  /// Gets a Ventilation property.
  [[nodiscard]] double ventilationIntakeRateUnoccupied() const noexcept {
    return ventilation.ventilationIntakeRateUnoccupied();
  }

  /// Sets a Ventilation property.
  void setVentilationIntakeRateUnoccupied(double ventilationIntakeRateUnoccupied) {
    ventilation.setVentilationIntakeRateUnoccupied(ventilationIntakeRateUnoccupied);
  }

  /// Gets a Ventilation property.
  [[nodiscard]] double ventilationType() const noexcept {
    return static_cast<double>(ventilation.ventType());
  }

  /// Sets a Ventilation property.
  void setVentilationType(std::string type) {
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);
    if (auto it = STRING_TO_VENTILATION_TYPE.find(type); it != STRING_TO_VENTILATION_TYPE.end()) {
      ventilation.setVentType(it->second);
    } else {
      throw std::invalid_argument("ventilationType parameter must be one of "
                                  "'mechanical', 'natural', or 'combined'");
    }
  }

  /// Gets a Ventilation property.
  [[nodiscard]] double ventPreheatDegC() const noexcept { return ventilation.ventPreheatDegC(); }

  /// Sets a Ventilation property.
  void setVentPreheatDegC(double ventPreheatDegC) {
    ventilation.setVentPreheatDegC(ventPreheatDegC);
  }

  /// Gets a Ventilation property.
  [[nodiscard]] double wind_coeff() const noexcept { return ventilation.wind_coeff(); }

  /// Sets a Ventilation property.
  void setWind_coeff(double wind_coeff) { ventilation.setWind_coeff(wind_coeff); }

  /// Gets a Ventilation property.
  [[nodiscard]] double wind_exp() const noexcept { return ventilation.wind_exp(); }

  /// Sets a Ventilation property.
  void setWind_exp(double wind_exp) { ventilation.setWind_exp(wind_exp); }

  /// Gets a Ventilation property.
  [[nodiscard]] double zone_frac() const noexcept { return ventilation.zone_frac(); }

  /// Sets a Ventilation property.
  void setZone_frac(double zone_frac) { ventilation.setZone_frac(zone_frac); }

private:
  void setCoreSimulationProperties(Simulation &sim) const;

  std::string resolveFilename(std::string_view baseFile, std::string_view relativeFile);
  void initializeStructure(const YAML::Node &params);

  std::map<LatLon, std::shared_ptr<WeatherData>> m_weatherCache;

  // In-class initialization
  std::shared_ptr<WeatherData> m_weather = std::make_shared<WeatherData>();
  std::shared_ptr<EpwData> m_edata = std::make_shared<EpwData>();

  Population pop;
  Location location;
  Lighting lights;
  Building building;
  Structure structure;
  Heating heating;
  Cooling cooling;
  Ventilation ventilation;
  // PhysicalQuantities phys;
  SimulationSettings simSettings;

  bool _valid = false;

  std::string m_weatherFilePath, m_scheduleFilePath, m_hourlySchedulePath;
  std::string dataFile;

  // Private setter for use by initializeParameters
  void setHourlySchedulePath(std::string hourlySchedulePath) {
    m_hourlySchedulePath = std::move(hourlySchedulePath);
  }

  void initializeParameters(const YAML::Node &params);

  void initializeParameter(void (UserModel::*setProp)(double), const YAML::Node &buildingParams,
                           std::string_view propertyName, bool required);
  void initializeParameter(void (UserModel::*setProp)(int), const YAML::Node &buildingParams,
                           std::string_view propertyName, bool required);
  void initializeParameter(void (UserModel::*setProp)(bool), const YAML::Node &buildingParams,
                           std::string_view propertyName, bool required);
  void initializeParameter(void (UserModel::*setProp)(const Vector &),
                           const YAML::Node &buildingParams, std::string_view propertyName,
                           bool required);
  void initializeParameter(void (UserModel::*setProp)(std::string),
                           const YAML::Node &buildingParams, std::string_view propertyName,
                           bool required);

  void northToSouth(Vector &vec);

  void loadBuilding(const std::string &buildingFile);
  void loadBuilding(const std::string &buildingFile, const std::string &defaultsFile);
  int weatherState(std::string_view header);
  void initializeSolar();
};

} // namespace openstudio::isomodel
