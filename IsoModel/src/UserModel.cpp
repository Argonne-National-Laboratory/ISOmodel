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

#include "UserModel.hpp"

using namespace std;
namespace openstudio {
namespace isomodel {

const std::string GAS = "gas";
const std::string ELECTRIC = "electric";

const std::string MECHANICAL = "mechanical";
const std::string NATURAL = "natural";
const std::string COMBINED = "combined";

const std::string NONE = "none";
const std::string SIMPLE = "simple";
const std::string ADVANCED = "advanced";

UserModel::UserModel() :
    _weather_cache(), _weather(new WeatherData()), _edata(new EpwData())
{
}

UserModel::~UserModel()
{
}

void UserModel::setCoreSimulationProperties(Simulation& sim) const {
  auto pPop = std::make_shared<Population>(pop);
  sim.setPop(pPop);
  // sim.setPop(pop);

  auto pBuilding = std::make_shared<Building>(building);
  sim.setBuilding(pBuilding);
  // sim.setBuilding(building);

  auto pCooling = std::make_shared<Cooling>(cooling);
  sim.setCooling(pCooling);
  // sim.setCooling(cooling);

  auto pHeating = std::make_shared<Heating>(heating);
  sim.setHeating(pHeating);
  // sim.setHeating(heating);

  auto pLights = std::make_shared<Lighting>(lights);
  sim.setLights(pLights);
  // sim.setLights(lights);

  auto pStructure = std::make_shared<Structure>(structure);
  sim.setStructure(pStructure);
  // sim.setStructure(structure);

  auto pVentilation = std::make_shared<Ventilation>(ventilation);
  sim.setVentilation(pVentilation);
  // sim.setVentilation(ventilation);

  std::shared_ptr<Location> loc(new Location);
  loc->setTerrain(terrainClass());
  loc->setWeatherData(_weather);
  sim.setLocation(loc);

  sim.setEpwData(_edata);
  
  auto pSimSettings = std::make_shared<SimulationSettings>(simSettings);
  sim.setSimulationSettings(pSimSettings);
  // sim.setSimulationSettings(simSettings);

  auto pPhys = std::make_shared<PhysicalQuantities>(phys);
  sim.setPhysicalQuantities(pPhys);
  // sim.setPhysicalQuantities(phys);
}

ISOHourly UserModel::toHourlyModel() const
{
  ISOHourly sim = ISOHourly();
  if (!_valid) {
    return *((ISOHourly*) NULL);
  }
  
  setCoreSimulationProperties(sim);
  return sim;
}

SimModel UserModel::toSimModel() const
{

  SimModel sim;

  if (!valid()) {
    std::cout << "Invalid" << std::endl;
    return *((SimModel*) NULL);
  }

  setCoreSimulationProperties(sim);

  return sim;
}
//http://stackoverflow.com/questions/10051679/c-tokenize-string
std::vector<std::string> inline stringSplit(const std::string &source, char delimiter = ' ', bool keepEmpty = false)
{
  std::vector<std::string> results;

  size_t prev = 0;
  size_t next = 0;
  if (source.size() == 0)
    return results;
  while ((next = source.find_first_of(delimiter, prev)) != std::string::npos) {
    if (keepEmpty || (next - prev != 0)) {
      results.push_back(source.substr(prev, next - prev));
    }
    prev = next + 1;
  }

  if (prev < source.size()) {
    results.push_back(source.substr(prev));
  }

  return results;
}

void UserModel::initializeStructure(const Properties& buildingParams)
{
  auto northToSouth = [](Vector& vec) {
    // .ism file is N, NE, E, SE, S, SW, W, NW, Roof
    // Structure is S, SE, E, NE, N, NW, W, SW, Roof
    double temp;

    // Swap 0 and 4 (N and S).
    temp = vec[0];
    vec[0] = vec[4];
    vec[4] = temp;

    // Swap 1 and 3 (NE and SE).
    temp = vec[1];
    vec[1] = vec[3];
    vec[3] = temp;
    
    // Swap 5 and 7 (SW and NW).
    temp = vec[5];
    vec[5] = vec[7];
    vec[7] = temp;
  };

  Vector values;
  buildingParams.getPropertyAsDoubleVector("wallArea", values);

  if (values.size() != 9)
    throw invalid_argument("Invalid number of values for wallArea parameter. It must have 9.");
  // Reorder the values from the .ism order to the order used in Structure.
  northToSouth(values);
  structure.setWallArea(values);

  buildingParams.getPropertyAsDoubleVector("wallU", values);
  if (values.size() != 9)
    throw invalid_argument("Invalid number of values for wallU parameter. It must have 9.");
  // Reorder the values from the .ism order to the order used in Structure.
  northToSouth(values);
  structure.setWallUniform(values);

  buildingParams.getPropertyAsDoubleVector("wallEmissivity", values);
  if (values.size() != 9)
    throw invalid_argument("Invalid number of values for wallEmissivity parameter. It must have 9.");
  // Reorder the values from the .ism order to the order used in Structure.
  northToSouth(values);
  structure.setWallThermalEmissivity(values);

  buildingParams.getPropertyAsDoubleVector("wallAbsorption", values);
  if (values.size() != 9)
    throw invalid_argument("Invalid number of values for wallAbsorption parameter. It must have 9.");
  // Reorder the values from the .ism order to the order used in Structure.
  northToSouth(values);
  structure.setWallSolarAbsorbtion(values);

  buildingParams.getPropertyAsDoubleVector("windowArea", values);
  if (values.size() != 9)
    throw invalid_argument("Invalid number of values for windowArea parameter. It must have 9.");
  // Reorder the values from the .ism order to the order used in Structure.
  northToSouth(values);
  structure.setWindowArea(values);

  buildingParams.getPropertyAsDoubleVector("windowU", values);
  if (values.size() != 9)
    throw invalid_argument("Invalid number of values for windowU parameter. It must have 9.");
  // Reorder the values from the .ism order to the order used in Structure.
  northToSouth(values);
  structure.setWindowUniform(values);

  buildingParams.getPropertyAsDoubleVector("windowSHGC", values);
  if (values.size() != 9)
    throw invalid_argument("Invalid number of values for windowSHGC parameter. It must have 9.");
  // Reorder the values from the .ism order to the order used in Structure.
  northToSouth(values);
  structure.setWindowNormalIncidenceSolarEnergyTransmittance(values);

  buildingParams.getPropertyAsDoubleVector("windowSCF", values);
  if (values.size() != 9)
    throw invalid_argument("Invalid number of values for windowSCF parameter. It must have 9.");
  // Reorder the values from the .ism order to the order used in Structure.
  northToSouth(values);
  structure.setWindowShadingCorrectionFactor(values);

  buildingParams.getPropertyAsDoubleVector("windowSDF", values);
  if (values.size() != 9)
    throw invalid_argument("Invalid number of values for windowSDF parameter. It must have 9.");
  // Reorder the values from the .ism order to the order used in Structure.
  northToSouth(values);
  structure.setWindowShadingDevice(values);
}

void UserModel::initializeParameters(const Properties& buildingParams)
{
  double attributeValue = *buildingParams.getPropertyAsDouble("terrainclass");
  setTerrainClass(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("buildingheight");
  setBuildingHeight(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("floorarea");
  setFloorArea(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("occupancydayfirst");
  setBuildingOccupancyFrom(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("occupancydaylast");
  setBuildingOccupancyTo(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("occupancyhourfirst");
  setEquivFullLoadOccupancyFrom(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("occupancyhourlast");
  setEquivFullLoadOccupancyTo(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("peopledensityoccupied");
  setPeopleDensityOccupied(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("peopledensityunoccupied");
  setPeopleDensityUnoccupied(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("lightingpowerdensityoccupied");
  setLightingPowerIntensityOccupied(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("lightingpowerdensityunoccupied");
  setLightingPowerIntensityUnoccupied(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("electricappliancepowerdensityoccupied");
  setElecPowerAppliancesOccupied(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("electricappliancepowerdensityunoccupied");
  setElecPowerAppliancesUnoccupied(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("gasappliancepowerdensityoccupied");
  setGasPowerAppliancesOccupied(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("gasappliancepowerdensityunoccupied");
  setGasPowerAppliancesUnoccupied(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("exteriorlightingpower");
  setExteriorLightingPower(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("hvacwastefactor");
  setHvacWasteFactor(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("hvacheatinglossfactor");
  setHvacHeatingLossFactor(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("hvaccoolinglossfactor");
  setHvacCoolingLossFactor(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("daylightsensordimmingfraction");
  setDaylightSensorSystem(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("lightingoccupancysensordimmingfraction");
  setLightingOccupancySensorSystem(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("constantilluminationcontrolmultiplier");	//constantilluminaitoncontrol
  setConstantIlluminationControl(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("coolingsystemcop");
  setCoolingSystemCOP(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("coolingsystemiplvtocopratio");
  setCoolingSystemIPLVToCOPRatio(attributeValue);

  std::string type = *buildingParams.getProperty("heatingfueltype");
  std::transform(type.begin(), type.end(), type.begin(), ::tolower);
  if (type == ELECTRIC)
    attributeValue = 1.0;
  else if (type == GAS)
    attributeValue = 2.0;
  else
    throw invalid_argument("heatingFuelType parameter must be one of 'gas' or 'electric'");
  setHeatingEnergyCarrier(attributeValue);

  attributeValue = *buildingParams.getPropertyAsDouble("heatingsystemefficiency");
  setHeatingSystemEfficiency(attributeValue);

  type = *buildingParams.getProperty("ventilationtype");
  std::transform(type.begin(), type.end(), type.begin(), ::tolower);
  if (type == MECHANICAL)
    attributeValue = 1.0;
  else if (type == COMBINED)
    attributeValue = 2.0;
  else if (type == NATURAL)
    attributeValue = 3.0;
  else
    throw invalid_argument("ventilationType parameter must be one of 'mechanical', 'natural', or 'combined'");
  setVentilationType(attributeValue);

  attributeValue = *buildingParams.getPropertyAsDouble("ventilationintakerateoccupied");
  setFreshAirFlowRate(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("ventilationExhaustRateOccupied");
  setSupplyExhaustRate(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("heatrecovery");
  setHeatRecovery(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("exhaustairrecirculation");
  setExhaustAirRecirclation(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("infiltrationrateoccupied");
  // set both of these to infiltration. Prior version set only the
  // buildingAirLeakage from the infiltration and the _infiltration var
  // wasn't used. Its still not used, but does have a getter and setter
  // so we set it
  setBuildingAirLeakage(attributeValue);
  setInfiltration(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("dhwdemand");
  setDhwDemand(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("dhwsystemefficiency");
  setDhwEfficiency(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("dhwdistributionefficiency");
  setDhwDistributionEfficiency(attributeValue);

  type = *buildingParams.getProperty("dhwfueltype");
  std::transform(type.begin(), type.end(), type.begin(), ::tolower);
  if (type == ELECTRIC)
    attributeValue = 1.0;
  else if (type == GAS)
    attributeValue = 2.0;
  else
    throw invalid_argument("dhwFuelType parameter must be one of 'gas' or 'electric'");
  setDhwEnergyCarrier(attributeValue);

  type = *buildingParams.getProperty("bemtype");
  std::transform(type.begin(), type.end(), type.begin(), ::tolower);
  if (type == NONE)
    attributeValue = 1.0;
  else if (type == SIMPLE)
    attributeValue = 2.0;
  else if (type == ADVANCED)
    attributeValue = 3.0;
  else
    throw invalid_argument("bemType parameter must be one of 'none', 'simple', or 'advanced'");
  setBemType(attributeValue);

  attributeValue = *buildingParams.getPropertyAsDouble("interiorheatcapacity");
  setInteriorHeatCapacity(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("exteriorheatcapacity");
  setExteriorHeatCapacity(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("heatingpumpcontrol");
  setHeatingPumpControl(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("coolingpumpcontrol");
  setCoolingPumpControl(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("heatgainperperson");
  setHeatGainPerPerson(attributeValue);
  //specificFanPower
  attributeValue = *buildingParams.getPropertyAsDouble("specificfanpower");
  setSpecificFanPower(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("fanflowcontrolfactor");
  setFanFlowControlFactor(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("coolingsetpointoccupied");
  setCoolingOccupiedSetpoint(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("coolingsetpointunoccupied");
  setCoolingUnoccupiedSetpoint(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("heatingsetpointoccupied");
  setHeatingOccupiedSetpoint(attributeValue);
  attributeValue = *buildingParams.getPropertyAsDouble("heatingsetpointunoccupied");
  setHeatingUnoccupiedSetpoint(attributeValue);

#if (USE_NEW_BUILDING_PARAMS)
  attributeValue = *buildingParams.getPropertyAsDouble("ventilationIntakeRateUnoccupied");
  setVentilationIntakeRateUnoccupied(attributeValue);

  attributeValue = *buildingParams.getPropertyAsDouble("ventilationExhaustRateUnoccupied");
  setVentilationExhaustRateUnoccupied(attributeValue);

  attributeValue = *buildingParams.getPropertyAsDouble("infiltrationRateUnoccupied");
  setInfiltrationRateUnoccupied(attributeValue);

  attributeValue = *buildingParams.getPropertyAsDouble("lightingPowerFixedOccupied");
  setLightingPowerFixedOccupied(attributeValue);

  attributeValue = *buildingParams.getPropertyAsDouble("lightingPowerFixedUnoccupied");
  setLightingPowerFixedUnoccupied(attributeValue);

  attributeValue = *buildingParams.getPropertyAsDouble("electricAppliancePowerFixedOccupied");
  setElectricAppliancePowerFixedOccupied(attributeValue);

  attributeValue = *buildingParams.getPropertyAsDouble("electricAppliancePowerFixedUnoccupied");
  setElectricAppliancePowerFixedUnoccupied(attributeValue);

  attributeValue = *buildingParams.getPropertyAsDouble("gasAppliancePowerFixedOccupied");
  setGasAppliancePowerFixedOccupied(attributeValue);

  attributeValue = *buildingParams.getPropertyAsDouble("gasAppliancePowerFixedUnoccupied");
  setGasAppliancePowerFixedUnoccupied(attributeValue);

  std::string scheduleFilePath = *buildingParams.getProperty("schedulefilepath");
  if (scheduleFilePath == "")
    throw invalid_argument("scheduleFilePath building parameter is missing");
  setScheduleFilePath(scheduleFilePath);
#endif

  std::string weatherFilePath = *buildingParams.getProperty("weatherfilepath");
  if (weatherFilePath == "")
    throw invalid_argument("weatherFilePath building parameter is missing");

  setWeatherFilePath(weatherFilePath);
}

void UserModel::loadBuilding(std::string buildingFile)
{
  Properties buildingParams(buildingFile);
  initializeParameters(buildingParams);
  initializeStructure(buildingParams);
}

void UserModel::loadBuilding(std::string buildingFile, std::string defaultsFile)
{
  Properties buildingParams(buildingFile, defaultsFile);
  initializeParameters(buildingParams);
  initializeStructure(buildingParams);
}

int UserModel::weatherState(std::string header)
{
  if (!header.compare("solar"))
    return 1;
  else if (!header.compare("hdbt"))
    return 2;
  else if (!header.compare("hEgh"))
    return 3;
  else if (!header.compare("mEgh"))
    return 4;
  else if (!header.compare("mdbt"))
    return 5;
  else if (!header.compare("mwind"))
    return 6;
  else
    return -1;
}
std::string UserModel::resolveFilename(std::string baseFile, std::string relativeFile)
{
  unsigned int lastSeparator = 0;
  unsigned int i = 0;
  const char separatorChar = '/';
  const char winSeparatorChar = '\\';
  std::string result;
  for (; i < baseFile.length(); i++) {
    result += (baseFile[i] == winSeparatorChar) ? separatorChar : baseFile[i];
    if (result[i] == separatorChar) {
      lastSeparator = i;
    }
  }
  result = result.substr(0, lastSeparator + 1);
  unsigned int j = 0;
  if (relativeFile.length() > 0) {
    //if first char is a separator, skip it
    if (relativeFile[0] == separatorChar || relativeFile[0] == winSeparatorChar)
      j++;
  }
  for (; j < relativeFile.length(); j++, i++) {
    result += (relativeFile[j] == winSeparatorChar) ? separatorChar : relativeFile[j];
  }
  return result;
}

void UserModel::loadWeather()
{
  std::string weatherFilename;
  //see if weather file path is absolute path
  //if so, use it, else assemble relative path
  if (boost::filesystem::exists(_weatherFilePath)) {
    weatherFilename = _weatherFilePath;
  } else {
    weatherFilename = resolveFilename(dataFile, _weatherFilePath);
    if (!boost::filesystem::exists(weatherFilename)) {
      std::cout << "Weather File Not Found: " << _weatherFilePath << std::endl;
      _valid = false;
    }
  }

  _edata->loadData(weatherFilename);
  initializeSolar();
}

void UserModel::loadAndSetWeather()
{
  loadWeather();
  _valid = true;
}

bool LatLon::operator <(const LatLon& rhs) const {
  if (lat < rhs.lat) return true;
  if (lat > rhs.lat) return false;

  if (lon < rhs.lon) return true;
  return false;
}

void UserModel::loadWeather(int block_size, double* weather_data)
{

  double lat = weather_data[0];
  double lon = weather_data[1];

  LatLon latlon = {lat, lon};
  auto iter = _weather_cache.find(latlon);
  if (iter == _weather_cache.end()) {
    //std::cout << "not in cache" << std::endl;
    _weather = make_shared<WeatherData>();
    _weather_cache.insert(make_pair(latlon, _weather));
    _edata->loadData(block_size, weather_data);
    initializeSolar();
  } else {
    //std::cout << "in cache" << std::endl;
    _weather = iter->second;
  }

  _valid = true;
}

void UserModel::initializeSolar()
{

  int state = 0, row = 0;
  Matrix _msolar(12, 8);
  Matrix _mhdbt(12, 24);
  Matrix _mhEgh(12, 24);
  Vector _mEgh(12);
  Vector _mdbt(12);
  Vector _mwind(12);

  string line;
  std::vector<std::string> linesplit;

  std::stringstream inputFile(_edata->toISOData());

  while (inputFile.good()) {
    getline(inputFile, line);
    if (line.size() > 0 && line[0] == '#')
      continue;
    linesplit = stringSplit(line, ',', true);
    if (linesplit.size() == 0) {
      continue;
    } else if (linesplit.size() == 1) {
      state = weatherState(linesplit[0]);
      row = 0;
    } else if (row < 12) {
      switch (state) {
      case 1: //solar = [12 x 8] mean monthly total solar radiation (W/m2) on a vertical surface for each of the 8 cardinal directions
        for (unsigned int c = 1; c < linesplit.size() && c < 9; c++) {
          _msolar(row, c - 1) = atof(linesplit[c].c_str());
        }
        break;
      case 2: //hdbt = [12 x 24] mean monthly dry bulb temp for each of the 24 hours of the day (C)
        for (unsigned int c = 1; c < linesplit.size() && c < 25; c++) {
          _mhdbt(row, c - 1) = atof(linesplit[c].c_str());
        }
        break;
      case 3: //hEgh =[12 x 24] mean monthly Global Horizontal Radiation for each of the 24 hours of the day (W/m2)
        for (unsigned int c = 1; c < linesplit.size() && c < 25; c++) {
          _mhEgh(row, c - 1) = atof(linesplit[c].c_str());
        }
        break;
      case 4:  //megh = [12 x 1] mean monthly Global Horizontal Radiation (W/m2)
        _mEgh[row] = atof(linesplit[1].c_str());
        break;
      case 5:    //mdbt = [12 x 1] mean monthly dry bulb temp (C)
        _mdbt[row] = atof(linesplit[1].c_str());
        break;
      case 6:    //mwind = [12 x 1] mean monthly wind speed; (m/s) 
        _mwind[row] = atof(linesplit[1].c_str());
        break;
      default:
        break;
      }
      row++;
    }
  }
  _weather->setMdbt(_mdbt);
  _weather->setMEgh(_mEgh);
  _weather->setMhdbt(_mhdbt);
  _weather->setMhEgh(_mhEgh);
  _weather->setMsolar(_msolar);
  _weather->setMwind(_mwind);
}

void UserModel::load(std::string buildingFile)
{
  dataFile = buildingFile;
  _valid = true;
  if (!boost::filesystem::exists(buildingFile)) {
    std::cout << "ISO Model File Not Found: " << buildingFile << std::endl;
    _valid = false;
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

void UserModel::load(std::string buildingFile, std::string defaultsFile)
{
  dataFile = buildingFile;
  _valid = true;

  // Check for the .ism file.
  if (!boost::filesystem::exists(buildingFile)) {
    std::cout << "ISO Model File Not Found: " << buildingFile << std::endl;
    _valid = false;
    return;
  }

  // Check for the defaults file.
  if (!boost::filesystem::exists(defaultsFile)) {
    std::cout << "ISO Model File Not Found: " << defaultsFile << std::endl;
    _valid = false;
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
} // isomodel
} // openstudio

