/**********************************************************************
 * Copyright (c) 2008-2013, Alliance for Sustainable Energy.
 * All rights reserved.
 **********************************************************************/

#include "UserModel.hpp"
#include <algorithm> // Required for std::transform in loadBuilding
#include <filesystem>
#include <iostream>
#include <optional>
#include <string_view>

namespace {
std::string resolveFilenameImpl(std::string_view baseFile,
                                std::string_view relativeFile) {
  unsigned int lastSeparator = 0;
  unsigned int i = 0;
  const char separatorChar = '/';
  const char winSeparatorChar = '\\';
  std::string result;

  for (; i < baseFile.size(); i++) {
    result += (baseFile[i] == winSeparatorChar) ? separatorChar : baseFile[i];
    if (result[i] == separatorChar) {
      lastSeparator = i;
    }
  }
  result = result.substr(0, lastSeparator + 1);

  unsigned int j = 0;
  if (!relativeFile.empty()) {
    if (relativeFile[0] == separatorChar ||
        relativeFile[0] == winSeparatorChar) {
      j++;
    }
  }
  for (; j < relativeFile.size(); j++, i++) {
    result +=
        (relativeFile[j] == winSeparatorChar) ? separatorChar : relativeFile[j];
  }
  return result;
}

int weatherStateImpl(std::string_view header) {
  // Preserve exact existing behavior (case-sensitive comparisons)
  if (header == "solar")
    return 1;
  if (header == "hdbt")
    return 2;
  if (header == "hEgh")
    return 3;
  if (header == "mEgh")
    return 4;
  if (header == "mdbt")
    return 5;
  if (header == "mwind")
    return 6;
  return -1;
}

template <typename T>
std::optional<T> getParameter(const YAML::Node &params,
                              std::string_view paramName) {
  // Direct access to map avoids string allocation and transformation
  // Note: yaml-cpp 0.8.0 supports string_view keys, or implicit conversion
  if (params[std::string(paramName)]) {
    try {
      return params[std::string(paramName)].as<T>();
    } catch (const YAML::TypedBadConversion<T> &) {
      return std::nullopt;
    }
  }
  return std::nullopt;
}

bool getParameterAsVector(const YAML::Node &params, std::string_view paramName,
                          openstudio::Vector &vec) {

  // Direct access to map avoids string allocation and transformation
  if (params[std::string(paramName)]) {
    vec.clear();
    auto param = params[std::string(paramName)];
    size_t n = std::distance(param.begin(), param.end());
    if (vec.size() != n) {
      vec.resize(n);
    }
    try {
      size_t index = 0;
      for (const auto &v : param) {
        vec[index] = v.as<double>();
        ++index;
      }
      return true;
    } catch (const YAML::TypedBadConversion<double> &) {
      return false;
    }
  }
  return false;
}

// // Internal helper overloads: paramName is already lowercased
// template <typename T>
YAML::Node loadLowercasedYamlMapFromFile(const std::string &filename) {
  YAML::Node src = YAML::LoadFile(filename);
  YAML::Node dst = YAML::Load("{}");

  for (const auto &kv : src) {
    std::string key = kv.first.as<std::string>();
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    dst[key] = kv.second;
  }
  return dst;
}

void mergeYamlMapInto(YAML::Node &base, const YAML::Node &overlay) {
  for (const auto &kv : overlay) {
    const std::string key = kv.first.as<std::string>();
    base[key] = kv.second;
  }
}

void throwIfEmptyYamlMap(const YAML::Node &node, const std::string &filename) {
  const auto n = static_cast<size_t>(std::distance(node.begin(), node.end()));
  if (n == 0) {
    throw std::invalid_argument("No parameters found in building file " +
                                filename + ". Is this a YAML format file?");
  }
}

bool fileExists(const std::string &path) {
  return std::filesystem::exists(path);
}

void failAndInvalidate(openstudio::isomodel::UserModel &self, const char *label,
                       const std::string &path) {
  std::cout << label << ": " << path << std::endl;
  self.setValid(false);
}

} // namespace

namespace openstudio::isomodel {

void UserModel::setCoreSimulationProperties(Simulation &sim) const {
  sim.setPop(pop);
  sim.setBuilding(building);
  sim.setCooling(cooling);
  sim.setHeating(heating);
  sim.setLights(lights);
  sim.setStructure(structure);
  sim.setVentilation(ventilation);
  sim.setLocation(location);
  sim.setEpwData(_edata);
  sim.setSimulationSettings(simSettings);
  // Removed setPhysicalQuantities, as models now use constants
}

HourlyModel UserModel::toHourlyModel() const {
  HourlyModel sim = HourlyModel();
  if (!_valid) {
    // return *((HourlyModel*)NULL);
    return HourlyModel();
  }

  setCoreSimulationProperties(sim);
  sim.setHourlySchedulePath(_hourlySchedulePath);
  return sim;
}

MonthlyModel UserModel::toMonthlyModel() const {
  MonthlyModel sim;

  if (!valid()) {
    std::cout << "Invalid" << std::endl;
    // return *((MonthlyModel*)NULL);
    return MonthlyModel();
  }

  setCoreSimulationProperties(sim);

  return sim;
}

void UserModel::initializeStructure(const YAML::Node &buildingParams) {
  initializeParameter(&UserModel::setWallArea, buildingParams, "wallarea",
                      true);
  initializeParameter(&UserModel::setWallU, buildingParams, "wallu", true);
  initializeParameter(&UserModel::setWallThermalEmissivity, buildingParams,
                      "wallemissivity", true);
  initializeParameter(&UserModel::setWallSolarAbsorption, buildingParams,
                      "wallabsorption", true);
  initializeParameter(&UserModel::setWindowArea, buildingParams, "windowarea",
                      true);
  initializeParameter(&UserModel::setWindowU, buildingParams, "windowu", true);
  initializeParameter(&UserModel::setWindowSHGC, buildingParams, "windowshgc",
                      true);
  initializeParameter(&UserModel::setWindowSCF, buildingParams, "windowscf",
                      true);
  initializeParameter(&UserModel::setWindowSDF, buildingParams, "windowsdf",
                      true);
}

void UserModel::initializeParameters(const YAML::Node &buildingParams) {
  initializeParameter(&UserModel::setTerrainClass, buildingParams,
                      "terrainclass", true);
  initializeParameter(&UserModel::setBuildingHeight, buildingParams,
                      "buildingheight", true);
  initializeParameter(&UserModel::setFloorArea, buildingParams, "floorarea",
                      true);
  initializeParameter(&UserModel::setBuildingOccupancyFrom, buildingParams,
                      "occupancydayfirst", true);
  initializeParameter(&UserModel::setBuildingOccupancyTo, buildingParams,
                      "occupancydaylast", true);
  initializeParameter(&UserModel::setEquivFullLoadOccupancyFrom, buildingParams,
                      "occupancyhourfirst", true);
  initializeParameter(&UserModel::setEquivFullLoadOccupancyTo, buildingParams,
                      "occupancyhourlast", true);
  initializeParameter(&UserModel::setPeopleDensityOccupied, buildingParams,
                      "peopledensityoccupied", true);
  initializeParameter(&UserModel::setPeopleDensityUnoccupied, buildingParams,
                      "peopledensityunoccupied", true);
  initializeParameter(&UserModel::setLightingPowerIntensityOccupied,
                      buildingParams, "lightingpowerdensityoccupied", true);
  initializeParameter(&UserModel::setLightingPowerIntensityUnoccupied,
                      buildingParams, "lightingpowerdensityunoccupied", true);
  initializeParameter(&UserModel::setElecPowerAppliancesOccupied,
                      buildingParams, "electricappliancepowerdensityoccupied",
                      true);
  initializeParameter(&UserModel::setElecPowerAppliancesUnoccupied,
                      buildingParams, "electricappliancepowerdensityunoccupied",
                      true);
  initializeParameter(&UserModel::setGasPowerAppliancesOccupied, buildingParams,
                      "gasappliancepowerdensityoccupied", true);
  initializeParameter(&UserModel::setGasPowerAppliancesUnoccupied,
                      buildingParams, "gasappliancepowerdensityunoccupied",
                      true);
  initializeParameter(&UserModel::setExteriorLightingPower, buildingParams,
                      "exteriorlightingpower", true);
  initializeParameter(&UserModel::setHvacWasteFactor, buildingParams,
                      "hvacwastefactor", true);
  initializeParameter(&UserModel::setHvacHeatingLossFactor, buildingParams,
                      "hvacheatinglossfactor", true);
  initializeParameter(&UserModel::setHvacCoolingLossFactor, buildingParams,
                      "hvaccoolinglossfactor", true);
  initializeParameter(&UserModel::setDaylightSensorSystem, buildingParams,
                      "daylightsensordimmingfraction", true);
  initializeParameter(&UserModel::setLightingOccupancySensorSystem,
                      buildingParams, "lightingoccupancysensordimmingfraction",
                      true);
  initializeParameter(&UserModel::setConstantIlluminationControl,
                      buildingParams, "constantilluminationcontrolmultiplier",
                      true);
  initializeParameter(&UserModel::setCoolingSystemCOP, buildingParams,
                      "coolingsystemcop", true);
  initializeParameter(&UserModel::setCoolingSystemIPLVToCOPRatio,
                      buildingParams, "coolingsystemiplvtocopratio", true);

  initializeParameter(&UserModel::setHeatingSystemEfficiency, buildingParams,
                      "heatingsystemefficiency", true);

  void (UserModel::*setHeatingEnergyCarrierWithString)(std::string) =
      &UserModel::setHeatingEnergyCarrier;
  initializeParameter(setHeatingEnergyCarrierWithString, buildingParams,
                      "heatingfueltype", true);

  void (UserModel::*setVentilationTypeWithString)(std::string) =
      &UserModel::setVentilationType;
  initializeParameter(setVentilationTypeWithString, buildingParams,
                      "ventilationtype", true);

  void (UserModel::*setDhwEnergyCarrierWithString)(std::string) =
      &UserModel::setDhwEnergyCarrier;
  initializeParameter(setDhwEnergyCarrierWithString, buildingParams,
                      "dhwfueltype", true);

  void (UserModel::*setBemTypeWithString)(std::string) = &UserModel::setBemType;
  initializeParameter(setBemTypeWithString, buildingParams, "bemtype", true);

  initializeParameter(&UserModel::setFreshAirFlowRate, buildingParams,
                      "ventilationintakerateoccupied", true);
  initializeParameter(&UserModel::setSupplyExhaustRate, buildingParams,
                      "ventilationexhaustrateoccupied", true); // was camelCase
  initializeParameter(&UserModel::setHeatRecovery, buildingParams,
                      "heatrecovery", true);
  initializeParameter(&UserModel::setExhaustAirRecirclation, buildingParams,
                      "exhaustairrecirculation", true);
  initializeParameter(&UserModel::setBuildingAirLeakage, buildingParams,
                      "infiltrationrateoccupied", true);
  initializeParameter(&UserModel::setDhwDemand, buildingParams, "dhwdemand",
                      true);
  initializeParameter(&UserModel::setDhwEfficiency, buildingParams,
                      "dhwsystemefficiency", true);
  initializeParameter(&UserModel::setDhwDistributionEfficiency, buildingParams,
                      "dhwdistributionefficiency", true);

  initializeParameter(&UserModel::setInteriorHeatCapacity, buildingParams,
                      "interiorheatcapacity", true);
  initializeParameter(&UserModel::setExteriorHeatCapacity, buildingParams,
                      "exteriorheatcapacity", true);
  initializeParameter(&UserModel::setHeatingPumpControl, buildingParams,
                      "heatingpumpcontrol", true);
  initializeParameter(&UserModel::setCoolingPumpControl, buildingParams,
                      "coolingpumpcontrol", true);
  initializeParameter(&UserModel::setHeatGainPerPerson, buildingParams,
                      "heatgainperperson", true);
  initializeParameter(&UserModel::setSpecificFanPower, buildingParams,
                      "specificfanpower", true);
  initializeParameter(&UserModel::setFanFlowControlFactor, buildingParams,
                      "fanflowcontrolfactor", true);
  initializeParameter(&UserModel::setCoolingOccupiedSetpoint, buildingParams,
                      "coolingsetpointoccupied", true);
  initializeParameter(&UserModel::setCoolingUnoccupiedSetpoint, buildingParams,
                      "coolingsetpointunoccupied", true);
  initializeParameter(&UserModel::setHeatingOccupiedSetpoint, buildingParams,
                      "heatingsetpointoccupied", true);
  initializeParameter(&UserModel::setHeatingUnoccupiedSetpoint, buildingParams,
                      "heatingsetpointunoccupied", true);

#if (USE_NEW_BUILDING_PARAMS)
  initializeParameter(&UserModel::setVentilationIntakeRateUnoccupied,
                      buildingParams, "ventilationintakerateunoccupied", true);
  initializeParameter(&UserModel::setVentilationExhaustRateUnoccupied,
                      buildingParams, "ventilationexhaustrateunoccupied", true);
  initializeParameter(&UserModel::setInfiltrationRateUnoccupied, buildingParams,
                      "infiltrationrateunoccupied", true);
  initializeParameter(&UserModel::setLightingPowerFixedOccupied, buildingParams,
                      "lightingpowerfixedoccupied", true);
  initializeParameter(&UserModel::setLightingPowerFixedUnoccupied,
                      buildingParams, "lightingpowerfixedunoccupied", true);
  initializeParameter(&UserModel::setElectricAppliancePowerFixedOccupied,
                      buildingParams, "electricappliancepowerfixedoccupied",
                      true);
  initializeParameter(&UserModel::setElectricAppliancePowerFixedUnoccupied,
                      buildingParams, "electricappliancepowerfixedunoccupied",
                      true);
  initializeParameter(&UserModel::setGasAppliancePowerFixedOccupied,
                      buildingParams, "gasappliancepowerfixedoccupied", true);
  initializeParameter(&UserModel::setGasAppliancePowerFixedUnoccupied,
                      buildingParams, "gasappliancepowerfixedunoccupied", true);

  initializeParameter(&UserModel::setScheduleFilePath, buildingParams,
                      "schedulefilepath", true);
#endif

  // Updated to match the YAML key "hourlyScheduleFilePath" (which becomes
  // lowercase)
  initializeParameter(&UserModel::setHourlySchedulePath, buildingParams,
                      "hourlyschedulefilepath", false);

  initializeParameter(&UserModel::setWeatherFilePath, buildingParams,
                      "weatherfilepath", true);

  initializeParameter(&UserModel::setExternalEquipment, buildingParams,
                      "externalequipment", false);
  initializeParameter(&UserModel::setForcedAirCooling, buildingParams,
                      "forcedaircooling", false);
  initializeParameter(&UserModel::setT_cl_ctrl_flag, buildingParams,
                      "t_cl_ctrl_flag", false);
  initializeParameter(&UserModel::setDT_supp_cl, buildingParams, "dt_supp_cl",
                      false);
  initializeParameter(&UserModel::setDC_YesNo, buildingParams, "dc_yesno",
                      false);
  initializeParameter(&UserModel::setEta_DC_network, buildingParams,
                      "eta_dc_network", false);
  initializeParameter(&UserModel::setEta_DC_COP, buildingParams, "eta_dc_cop",
                      false);
  initializeParameter(&UserModel::setEta_DC_frac_abs, buildingParams,
                      "eta_dc_frac_abs", false);
  initializeParameter(&UserModel::setEta_DC_COP_abs, buildingParams,
                      "eta_dc_cop_abs", false);
  initializeParameter(&UserModel::setFrac_DC_free, buildingParams,
                      "frac_dc_free", false);
  initializeParameter(&UserModel::setE_pumps_cl, buildingParams, "e_pumps_cl",
                      false);
  initializeParameter(&UserModel::setForcedAirHeating, buildingParams,
                      "forcedairheating", false);
  initializeParameter(&UserModel::setDT_supp_ht, buildingParams, "dt_supp_ht",
                      false);
  initializeParameter(&UserModel::setE_pumps_ht, buildingParams, "e_pumps_ht",
                      false);
  initializeParameter(&UserModel::setT_ht_ctrl_flag, buildingParams,
                      "t_ht_ctrl_flag", false);
  initializeParameter(&UserModel::setA_H0, buildingParams, "a_h0", false);
  initializeParameter(&UserModel::setTau_H0, buildingParams, "tau_h0", false);
  initializeParameter(&UserModel::setDH_YesNo, buildingParams, "dh_yesno",
                      false);
  initializeParameter(&UserModel::setEta_DH_network, buildingParams,
                      "eta_dh_network", false);
  initializeParameter(&UserModel::setEta_DH_sys, buildingParams, "eta_dh_sys",
                      false);
  initializeParameter(&UserModel::setFrac_DH_free, buildingParams,
                      "frac_dh_free", false);
  initializeParameter(&UserModel::setDhw_tset, buildingParams, "dhw_tset",
                      false);
  initializeParameter(&UserModel::setDhw_tsupply, buildingParams, "dhw_tsupply",
                      false);
  initializeParameter(&UserModel::setN_day_start, buildingParams, "n_day_start",
                      false);
  initializeParameter(&UserModel::setN_day_end, buildingParams, "n_day_end",
                      false);
  initializeParameter(&UserModel::setN_weeks, buildingParams, "n_weeks", false);
  initializeParameter(&UserModel::setElecInternalGains, buildingParams,
                      "elecinternalgains", false);
  initializeParameter(&UserModel::setPermLightPowerDensity, buildingParams,
                      "permlightpowerdensity", false);
  initializeParameter(&UserModel::setPresenceSensorAd, buildingParams,
                      "presencesensorad", false);
  initializeParameter(&UserModel::setAutomaticAd, buildingParams, "automaticad",
                      false);
  initializeParameter(&UserModel::setPresenceAutoAd, buildingParams,
                      "presenceautoad", false);
  initializeParameter(&UserModel::setManualSwitchAd, buildingParams,
                      "manualswitchad", false);
  initializeParameter(&UserModel::setPresenceSensorLux, buildingParams,
                      "presencesensorlux", false);
  initializeParameter(&UserModel::setAutomaticLux, buildingParams,
                      "automaticlux", false);
  initializeParameter(&UserModel::setPresenceAutoLux, buildingParams,
                      "presenceautolux", false);
  initializeParameter(&UserModel::setManualSwitchLux, buildingParams,
                      "manualswitchlux", false);
  initializeParameter(&UserModel::setNaturallyLightedArea, buildingParams,
                      "naturallylightedarea", false);

  initializeParameter(&UserModel::setPhiIntFractionToAirNode, buildingParams,
                      "phiintfractiontoairnode", false);
  initializeParameter(&UserModel::setPhiSolFractionToAirNode, buildingParams,
                      "phisolfractiontoairnode", false);
  initializeParameter(&UserModel::setHci, buildingParams, "hci", false);
  initializeParameter(&UserModel::setHri, buildingParams, "hri", false);
  initializeParameter(&UserModel::setR_se, buildingParams, "r_se", false);
  initializeParameter(&UserModel::setIrradianceForMaxShadingUse, buildingParams,
                      "irradianceformaxshadinguse", false);
  initializeParameter(&UserModel::setShadingFactorAtMaxUse, buildingParams,
                      "shadingfactoratmaxuse", false);
  initializeParameter(&UserModel::setTotalAreaPerFloorArea, buildingParams,
                      "totalareaperfloorarea", false);
  initializeParameter(&UserModel::setWin_ff, buildingParams, "win_ff", false);
  initializeParameter(&UserModel::setWin_F_W, buildingParams, "win_f_w", false);
  initializeParameter(&UserModel::setR_sc_ext, buildingParams, "r_sc_ext",
                      false);
  initializeParameter(&UserModel::setVentPreheatDegC, buildingParams,
                      "ventpreheatdegc", false);
  initializeParameter(&UserModel::setN50, buildingParams, "n50", false);
  initializeParameter(&UserModel::setHzone, buildingParams, "hzone", false);
  initializeParameter(&UserModel::setP_exp, buildingParams, "p_exp", false);
  initializeParameter(&UserModel::setZone_frac, buildingParams, "zone_frac",
                      false);
  initializeParameter(&UserModel::setStack_exp, buildingParams, "stack_exp",
                      false);
  initializeParameter(&UserModel::setStack_coeff, buildingParams, "stack_coeff",
                      false);
  initializeParameter(&UserModel::setWind_exp, buildingParams, "wind_exp",
                      false);
  initializeParameter(&UserModel::setWind_coeff, buildingParams, "wind_coeff",
                      false);
  initializeParameter(&UserModel::setDCp, buildingParams, "dcp", false);
  initializeParameter(&UserModel::setVent_rate_flag, buildingParams,
                      "vent_rate_flag", false);
  initializeParameter(&UserModel::setH_ve, buildingParams, "h_ve", false);
}

// NOTE: YAML keys are lowercased by loadLowercasedYamlMapFromFile.
// This avoids repeated string allocations and transformations in each
// getParameter call.
void UserModel::initializeParameter(void (UserModel::*setProp)(double),
                                    const YAML::Node &params,
                                    std::string_view paramName, bool required) {

  if (auto prop = getParameter<double>(params, paramName)) {
    (this->*setProp)(*prop);
  } else if (required) {
    throw std::invalid_argument("Required property " + std::string(paramName) +
                                " missing in .ism file.");
  }
}

void UserModel::initializeParameter(void (UserModel::*setProp)(int),
                                    const YAML::Node &params,
                                    std::string_view paramName, bool required) {

  if (auto prop = getParameter<int>(params, paramName)) {
    (this->*setProp)(*prop);
  } else if (required) {
    throw std::invalid_argument("Required property " + std::string(paramName) +
                                " missing in .ism file.");
  }
}

void UserModel::initializeParameter(void (UserModel::*setProp)(bool),
                                    const YAML::Node &params,
                                    std::string_view paramName, bool required) {

  if (auto prop = getParameter<bool>(params, paramName)) {
    (this->*setProp)(*prop);
  } else if (required) {
    throw std::invalid_argument("Required property " + std::string(paramName) +
                                " missing in .ism file.");
  }
}

void UserModel::initializeParameter(void (UserModel::*setProp)(const Vector &),
                                    const YAML::Node &params,
                                    std::string_view paramName, bool required) {

  Vector vec;
  if (getParameterAsVector(params, paramName, vec)) {
    northToSouth(vec);
    (this->*setProp)(vec);
  } else if (required) {
    throw std::invalid_argument("Required property " + std::string(paramName) +
                                " missing in .ism file.");
  }
}

void UserModel::initializeParameter(void (UserModel::*setProp)(std::string),
                                    const YAML::Node &params,
                                    std::string_view paramName, bool required) {
  if (auto prop = getParameter<std::string>(params, paramName)) {
    (this->*setProp)(*prop);
  } else if (required) {
    throw std::invalid_argument("Required property " + std::string(paramName) +
                                " missing in .ism file.");
  }
}

void UserModel::northToSouth(Vector &vec) {
  std::swap(vec[0], vec[4]);
  std::swap(vec[1], vec[3]);
  std::swap(vec[5], vec[7]);
}

void UserModel::loadBuilding(const std::string &buildingFile) {
  YAML::Node buildingParams = loadLowercasedYamlMapFromFile(buildingFile);
  throwIfEmptyYamlMap(buildingParams, buildingFile);

  initializeParameters(buildingParams);
  initializeStructure(buildingParams);
}

void UserModel::loadBuilding(const std::string &buildingFile,
                             const std::string &defaultsFile) {
  YAML::Node buildingParams = loadLowercasedYamlMapFromFile(defaultsFile);
  throwIfEmptyYamlMap(buildingParams, defaultsFile);

  YAML::Node overlay = loadLowercasedYamlMapFromFile(buildingFile);
  throwIfEmptyYamlMap(overlay, buildingFile);
  mergeYamlMapInto(buildingParams, overlay);

  initializeParameters(buildingParams);
  initializeStructure(buildingParams);
}

int UserModel::weatherState(std::string_view header) {
  return weatherStateImpl(header);
}

std::string UserModel::resolveFilename(std::string_view baseFile,
                                       std::string_view relativeFile) {
  return resolveFilenameImpl(baseFile, relativeFile);
}

void UserModel::loadWeather() {
  std::string weatherFilename;
  if (fileExists(_weatherFilePath)) {
    weatherFilename = _weatherFilePath;
  } else {
    weatherFilename = resolveFilename(dataFile, _weatherFilePath);
    if (!fileExists(weatherFilename)) {
      failAndInvalidate(*this, "Weather File Not Found", _weatherFilePath);
    }
  }

  _edata->loadData(weatherFilename);
  initializeSolar();
  location.setWeatherData(_weather);
}

void UserModel::loadAndSetWeather() {
  loadWeather();
  _valid = true;
}

bool LatLon::operator<(const LatLon &rhs) const {
  if (lat < rhs.lat)
    return true;
  if (lat > rhs.lat)
    return false;

  if (lon < rhs.lon)
    return true;
  return false;
}

void UserModel::loadWeather(int block_size, double *weather_data) {

  double lat = weather_data[0];
  double lon = weather_data[1];

  LatLon latlon = {lat, lon};
  auto iter = _weather_cache.find(latlon);
  if (iter == _weather_cache.end()) {
    _weather = std::make_shared<WeatherData>();
    _weather_cache.emplace(latlon, _weather);
    _edata->loadData(block_size, weather_data);
    initializeSolar();
  } else {
    _weather = iter->second;
  }

  location.setWeatherData(_weather);

  _valid = true;
}

// OPTIMIZATION ITEM 1: Direct transfer
void UserModel::initializeSolar() {
  // Optimization: Direct transfer replaces the CSV parsing logic
  if (_edata && _weather) {
    _edata->populateWeatherData(_weather);
  }
}

void UserModel::load(std::string buildingFile) {
  dataFile = buildingFile;
  _valid = true;

  if (!fileExists(buildingFile)) {
    failAndInvalidate(*this, "ISO Model File Not Found", buildingFile);
    return;
  }

  if (DEBUG_ISO_MODEL_SIMULATION)
    std::cout << "Loading Building File: " << buildingFile << std::endl;
  loadBuilding(buildingFile);
  if (DEBUG_ISO_MODEL_SIMULATION)
    std::cout << "Loading Weather File: " << weatherFilePath() << std::endl;
  loadWeather();
  if (DEBUG_ISO_MODEL_SIMULATION)
    std::cout << "Weather File Loaded" << std::endl;
}

void UserModel::load(std::string buildingFile, std::string defaultsFile) {
  dataFile = buildingFile;
  _valid = true;

  if (!fileExists(buildingFile)) {
    failAndInvalidate(*this, "ISO Model File Not Found", buildingFile);
    return;
  }

  if (!fileExists(defaultsFile)) {
    // std::cout << "ISO Model File Not Found: " << defaultsFile << std::endl;
    // _valid = false;
    failAndInvalidate(*this, "Defaults ISO Model File Not Found", defaultsFile);
    return;
  }

  if (DEBUG_ISO_MODEL_SIMULATION)
    std::cout << "Loading Building File: " << buildingFile << std::endl;

  loadBuilding(buildingFile, defaultsFile);

  if (DEBUG_ISO_MODEL_SIMULATION)
    std::cout << "Loading Weather File: " << weatherFilePath() << std::endl;

  loadWeather();

  if (DEBUG_ISO_MODEL_SIMULATION)
    std::cout << "Weather File Loaded" << std::endl;
}

} // namespace openstudio::isomodel