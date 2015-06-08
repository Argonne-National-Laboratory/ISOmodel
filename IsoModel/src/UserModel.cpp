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
  std::shared_ptr<Population> pop(new Population);
  pop->setDaysStart(_buildingOccupancyFrom);
  pop->setDaysEnd(_buildingOccupancyTo);
  pop->setHoursEnd(_equivFullLoadOccupancyTo);
  pop->setHoursStart(_equivFullLoadOccupancyFrom);
  pop->setDensityOccupied(_peopleDensityOccupied);
  pop->setDensityUnoccupied(_peopleDensityUnoccupied);
  pop->setHeatGainPerPerson(_heatGainPerPerson);
  sim.setPop(pop);

  std::shared_ptr<Building> building(new Building);
  building->setBuildingEnergyManagement(_bemType);
  building->setConstantIllumination(_constantIlluminationControl);
  building->setElectricApplianceHeatGainOccupied(_elecPowerAppliancesOccupied);
  building->setElectricApplianceHeatGainUnoccupied(_elecPowerAppliancesUnoccupied);
  building->setGasApplianceHeatGainOccupied(_gasPowerAppliancesOccupied);
  building->setGasApplianceHeatGainUnoccupied(_gasPowerAppliancesUnoccupied);
  building->setLightingOccupancySensor(_lightingOccupancySensorSystem);
  sim.setBuilding(building);

  std::shared_ptr<Cooling> cooling(new Cooling);
  cooling->setCop(_coolingSystemCOP);
  cooling->setHvacLossFactor(_hvacCoolingLossFactor);
  cooling->setPartialLoadValue(_coolingSystemIPLVToCOPRatio);
  cooling->setPumpControlReduction(_coolingPumpControl);
  cooling->setTemperatureSetPointOccupied(_coolingOccupiedSetpoint);
  cooling->setTemperatureSetPointUnoccupied(_coolingUnoccupiedSetpoint);
  sim.setCooling(cooling);

  std::shared_ptr<Heating> heating(new Heating);
  heating->setEfficiency(_heatingSystemEfficiency);
  heating->setEnergyType(_heatingEnergyCarrier);
  heating->setHotcoldWasteFactor(_hvacWasteFactor); // Used in hvac distribution efficiency.
  heating->setHotWaterDemand(_dhwDemand);
  heating->setHotWaterDistributionEfficiency(_dhwDistributionEfficiency);
  heating->setHotWaterEnergyType(_dhwEnergyCarrier);
  heating->setHotWaterSystemEfficiency(_dhwEfficiency);
  heating->setHvacLossFactor(_hvacHeatingLossFactor);
  heating->setTemperatureSetPointOccupied(_heatingOccupiedSetpoint);
  heating->setTemperatureSetPointUnoccupied(_heatingUnoccupiedSetpoint);
  heating->setPumpControlReduction(_heatingPumpControl);
  sim.setHeating(heating);

  std::shared_ptr<Lighting> lighting(new Lighting);
  lighting->setDimmingFraction(_daylightSensorSystem);
  lighting->setExteriorEnergy(_exteriorLightingPower);
  lighting->setPowerDensityOccupied(_lightingPowerIntensityOccupied);
  lighting->setPowerDensityUnoccupied(_lightingPowerIntensityUnoccupied);
  sim.setLights(lighting);

  std::shared_ptr<Structure> structure(new Structure);
  structure->setFloorArea(_floorArea);
  structure->setBuildingHeight(_buildingHeight);
  structure->setInfiltrationRate(_buildingAirLeakage);
  structure->setInteriorHeatCapacity(_interiorHeatCapacity);
  //directions in the order [S, SE, E, NE, N, NW, W, SW, roof/skylight]
  Vector wallArea(9);
  wallArea[0] = _wallAreaS;
  wallArea[1] = _wallAreaSE;
  wallArea[2] = _wallAreaE;
  wallArea[3] = _wallAreaNE;
  wallArea[4] = _wallAreaN;
  wallArea[5] = _wallAreaNW;
  wallArea[6] = _wallAreaW;
  wallArea[7] = _wallAreaSW;
  wallArea[8] = _roofArea;
  structure->setWallArea(wallArea); //vector
  structure->setWallHeatCapacity(_exteriorHeatCapacity); //??

  Vector wallSolar(9);
  wallSolar[0] = _wallSolarAbsorptionS;
  wallSolar[1] = _wallSolarAbsorptionSE;
  wallSolar[2] = _wallSolarAbsorptionE;
  wallSolar[3] = _wallSolarAbsorptionNE;
  wallSolar[4] = _wallSolarAbsorptionN;
  wallSolar[5] = _wallSolarAbsorptionNW;
  wallSolar[6] = _wallSolarAbsorptionW;
  wallSolar[7] = _wallSolarAbsorptionSW;
  wallSolar[8] = _roofSolarAbsorption;
  structure->setWallSolarAbsorbtion(wallSolar); //vector

  Vector wallTherm(9);
  wallTherm[0] = _wallThermalEmissivityS;
  wallTherm[1] = _wallThermalEmissivitySE;
  wallTherm[2] = _wallThermalEmissivityE;
  wallTherm[3] = _wallThermalEmissivityNE;
  wallTherm[4] = _wallThermalEmissivityN;
  wallTherm[5] = _wallThermalEmissivityNW;
  wallTherm[6] = _wallThermalEmissivityW;
  wallTherm[7] = _wallThermalEmissivitySW;
  wallTherm[8] = _roofThermalEmissivity;
  structure->setWallThermalEmissivity(wallTherm); //vector

  Vector wallU(9);
  wallU[0] = _wallUvalueS;
  wallU[1] = _wallUvalueSE;
  wallU[2] = _wallUvalueE;
  wallU[3] = _wallUvalueNE;
  wallU[4] = _wallUvalueN;
  wallU[5] = _wallUvalueNW;
  wallU[6] = _wallUvalueW;
  wallU[7] = _wallUvalueSW;
  wallU[8] = _roofUValue;
  structure->setWallUniform(wallU); //vector

  Vector windowArea(9);
  windowArea[0] = _windowAreaS;
  windowArea[1] = _windowAreaSE;
  windowArea[2] = _windowAreaE;
  windowArea[3] = _windowAreaNE;
  windowArea[4] = _windowAreaN;
  windowArea[5] = _windowAreaNW;
  windowArea[6] = _windowAreaW;
  windowArea[7] = _windowAreaSW;
  windowArea[8] = _skylightArea;
  structure->setWindowArea(windowArea); //vector

  Vector winSHGC(9);
  winSHGC[0] = _windowSHGCS;
  winSHGC[1] = _windowSHGCSE;
  winSHGC[2] = _windowSHGCE;
  winSHGC[3] = _windowSHGCNE;
  winSHGC[4] = _windowSHGCN;
  winSHGC[5] = _windowSHGCNW;
  winSHGC[6] = _windowSHGCW;
  winSHGC[7] = _windowSHGCSW;
  winSHGC[8] = _skylightSHGC;
  structure->setWindowNormalIncidenceSolarEnergyTransmittance(winSHGC); //vector

  Vector winSCF(9);
  winSCF[0] = _windowSCFS;
  winSCF[1] = _windowSCFSE;
  winSCF[2] = _windowSCFE;
  winSCF[3] = _windowSCFNE;
  winSCF[4] = _windowSCFN;
  winSCF[5] = _windowSCFNW;
  winSCF[6] = _windowSCFW;
  winSCF[7] = _windowSCFSW;
  winSCF[8] = _windowSCFN;
  structure->setWindowShadingCorrectionFactor(winSCF); //vector
  
  Vector winSDF(9);
  winSDF[0] = _windowSDFS;
  winSDF[1] = _windowSDFSE;
  winSDF[2] = _windowSDFE;
  winSDF[3] = _windowSDFNE;
  winSDF[4] = _windowSDFN;
  winSDF[5] = _windowSDFNW;
  winSDF[6] = _windowSDFW;
  winSDF[7] = _windowSDFSW;
  winSDF[8] = _windowSDFN;
  structure->setWindowShadingDevice(winSDF);

  Vector winU(9);
  winU[0] = _windowUvalueS;
  winU[1] = _windowUvalueSE;
  winU[2] = _windowUvalueE;
  winU[3] = _windowUvalueNE;
  winU[4] = _windowUvalueN;
  winU[5] = _windowUvalueNW;
  winU[6] = _windowUvalueW;
  winU[7] = _windowUvalueSW;
  winU[8] = _skylightUvalue;
  structure->setWindowUniform(winU); //vector
  sim.setStructure(structure);

  std::shared_ptr<Ventilation> ventilation(new Ventilation);
  ventilation->setExhaustAirRecirculated(_exhaustAirRecirclation);
  ventilation->setFanControlFactor(_fanFlowControlFactor);
  ventilation->setFanPower(_specificFanPower);
  ventilation->setHeatRecoveryEfficiency(_heatRecovery);
  ventilation->setSupplyDifference(_supplyExhaustRate);
  ventilation->setSupplyRate(_freshAirFlowRate);
  ventilation->setType(_ventilationType);
  sim.setVentilation(ventilation);
}

ISOHourly UserModel::toHourlyModel() const
{
  ISOHourly sim = ISOHourly();
  if (!_valid) {
    return *((ISOHourly*) NULL);
  }
  
  setCoreSimulationProperties(sim);

  sim.setEpwData(_edata);

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

  std::shared_ptr<Location> loc(new Location);
  loc->setTerrain(_terrainClass);
  loc->setWeatherData(_weather);
  sim.setLocation(loc);

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

  std::vector<double> values;
  buildingParams.getPropertyAsDoubleVector("wallArea", values);
  if (values.size() != 9)
    throw invalid_argument("Invalid number of values for wallArea parameter. It must have 9.");
  _wallAreaN = values[0];
  _wallAreaNE = values[1];
  _wallAreaE = values[2];
  _wallAreaSE = values[3];
  _wallAreaS = values[4];
  _wallAreaSW = values[5];
  _wallAreaW = values[6];
  _wallAreaNW = values[7];
  _roofArea = values[8];

  buildingParams.getPropertyAsDoubleVector("wallU", values);
  if (values.size() != 9)
    throw invalid_argument("Invalid number of values for wallU parameter. It must have 9.");
  _wallUvalueN = values[0];
  _wallUvalueNE = values[1];
  _wallUvalueE = values[2];
  _wallUvalueSE = values[3];
  _wallUvalueS = values[4];
  _wallUvalueSW = values[5];
  _wallUvalueW = values[6];
  _wallUvalueNW = values[7];
  _roofUValue = values[8];

  buildingParams.getPropertyAsDoubleVector("wallEmissivity", values);
  if (values.size() != 9)
    throw invalid_argument("Invalid number of values for wallEmissivity parameter. It must have 9.");
  _wallThermalEmissivityN = values[0];
  _wallThermalEmissivityNE = values[1];
  _wallThermalEmissivityE = values[2];
  _wallThermalEmissivitySE = values[3];
  _wallThermalEmissivityS = values[4];
  _wallThermalEmissivitySW = values[5];
  _wallThermalEmissivityW = values[6];
  _wallThermalEmissivityNW = values[7];
  _roofThermalEmissivity = values[8];

  buildingParams.getPropertyAsDoubleVector("wallAbsorption", values);
  if (values.size() != 9)
    throw invalid_argument("Invalid number of values for wallAbsorption parameter. It must have 9.");
  _wallSolarAbsorptionN = values[0];
  _wallSolarAbsorptionNE = values[1];
  _wallSolarAbsorptionE = values[2];
  _wallSolarAbsorptionSE = values[3];
  _wallSolarAbsorptionS = values[4];
  _wallSolarAbsorptionSW = values[5];
  _wallSolarAbsorptionW = values[6];
  _wallSolarAbsorptionNW = values[7];
  _roofSolarAbsorption = values[8];

  buildingParams.getPropertyAsDoubleVector("windowArea", values);
  if (values.size() != 9)
    throw invalid_argument("Invalid number of values for windowArea parameter. It must have 9.");
  _windowAreaN = values[0];
  _windowAreaNE = values[1];
  _windowAreaE = values[2];
  _windowAreaSE = values[3];
  _windowAreaS = values[4];
  _windowAreaSW = values[5];
  _windowAreaW = values[6];
  _windowAreaNW = values[7];
  _skylightArea = values[8];

  buildingParams.getPropertyAsDoubleVector("windowU", values);
  if (values.size() != 9)
    throw invalid_argument("Invalid number of values for windowU parameter. It must have 9.");
  _windowUvalueN = values[0];
  _windowUvalueNE = values[1];
  _windowUvalueE = values[2];
  _windowUvalueSE = values[3];
  _windowUvalueS = values[4];
  _windowUvalueSW = values[5];
  _windowUvalueW = values[6];
  _windowUvalueNW = values[7];
  _skylightUvalue = values[8];

  buildingParams.getPropertyAsDoubleVector("windowSHGC", values);
  if (values.size() != 9)
    throw invalid_argument("Invalid number of values for windowSHGC parameter. It must have 9.");
  _windowSHGCN = values[0];
  _windowSHGCNE = values[1];
  _windowSHGCE = values[2];
  _windowSHGCSE = values[3];
  _windowSHGCS = values[4];
  _windowSHGCSW = values[5];
  _windowSHGCW = values[6];
  _windowSHGCNW = values[7];
  _skylightSHGC = values[8];

  buildingParams.getPropertyAsDoubleVector("windowSCF", values);
  if (values.size() != 9)
    throw invalid_argument("Invalid number of values for windowSCF parameter. It must have 9.");
  _windowSCFN = values[0];
  _windowSCFNE = values[1];
  _windowSCFE = values[2];
  _windowSCFSE = values[3];
  _windowSCFS = values[4];
  _windowSCFSW = values[5];
  _windowSCFW = values[6];
  _windowSCFNW = values[7];
  _skylightSCF = values[8];

  buildingParams.getPropertyAsDoubleVector("windowSDF", values);
  if (values.size() != 9)
    throw invalid_argument("Invalid number of values for windowSDF parameter. It must have 9.");
  _windowSDFN = values[0];
  _windowSDFNE = values[1];
  _windowSDFE = values[2];
  _windowSDFSE = values[3];
  _windowSDFS = values[4];
  _windowSDFSW = values[5];
  _windowSDFW = values[6];
  _windowSDFNW = values[7];
  _skylightSDF = values[8];
}

void UserModel::initializeParameters(const Properties& buildingParams)
{
  double attributeValue = buildingParams.getPropertyAsDouble("terrainclass");
  setTerrainClass(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("buildingheight");
  setBuildingHeight(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("floorarea");
  setFloorArea(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("occupancydayfirst");
  setBuildingOccupancyFrom(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("occupancydaylast");
  setBuildingOccupancyTo(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("occupancyhourfirst");
  setEquivFullLoadOccupancyFrom(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("occupancyhourlast");
  setEquivFullLoadOccupancyTo(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("peopledensityoccupied");
  setPeopleDensityOccupied(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("peopledensityunoccupied");
  setPeopleDensityUnoccupied(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("lightingpowerdensityoccupied");
  setLightingPowerIntensityOccupied(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("lightingpowerdensityunoccupied");
  setLightingPowerIntensityUnoccupied(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("electricappliancepowerdensityoccupied");
  setElecPowerAppliancesOccupied(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("electricappliancepowerdensityunoccupied");
  setElecPowerAppliancesUnoccupied(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("gasappliancepowerdensityoccupied");
  setGasPowerAppliancesOccupied(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("gasappliancepowerdensityunoccupied");
  setGasPowerAppliancesUnoccupied(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("exteriorlightingpower");
  setExteriorLightingPower(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("hvacwastefactor");
  setHvacWasteFactor(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("hvacheatinglossfactor");
  setHvacHeatingLossFactor(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("hvaccoolinglossfactor");
  setHvacCoolingLossFactor(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("daylightsensordimmingfraction");
  setDaylightSensorSystem(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("lightingoccupancysensordimmingfraction");
  setLightingOccupancySensorSystem(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("constantilluminationcontrolmultiplier");	//constantilluminaitoncontrol
  setConstantIlluminationControl(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("coolingsystemcop");
  setCoolingSystemCOP(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("coolingsystemiplvtocopratio");
  setCoolingSystemIPLVToCOPRatio(attributeValue);

  std::string type = buildingParams.getProperty("heatingfueltype");
  std::transform(type.begin(), type.end(), type.begin(), ::tolower);
  if (type == ELECTRIC)
    attributeValue = 1.0;
  else if (type == GAS)
    attributeValue = 2.0;
  else
    throw invalid_argument("heatingFuelType parameter must be one of 'gas' or 'electric'");
  setHeatingEnergyCarrier(attributeValue);

  attributeValue = buildingParams.getPropertyAsDouble("heatingsystemefficiency");
  setHeatingSystemEfficiency(attributeValue);

  type = buildingParams.getProperty("ventilationtype");
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

  attributeValue = buildingParams.getPropertyAsDouble("ventilationintakerateoccupied");
  setFreshAirFlowRate(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("ventilationExhaustRateOccupied");
  setSupplyExhaustRate(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("heatrecovery");
  setHeatRecovery(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("exhaustairrecirculation");
  setExhaustAirRecirclation(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("infiltrationrateoccupied");
  // set both of these to infiltration. Prior version set only the
  // buildingAirLeakage from the infiltration and the _infiltration var
  // wasn't used. Its still not used, but does have a getter and setter
  // so we set it
  setBuildingAirLeakage(attributeValue);
  setInfiltration(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("dhwdemand");
  setDhwDemand(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("dhwsystemefficiency");
  setDhwEfficiency(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("dhwdistributionefficiency");
  setDhwDistributionEfficiency(attributeValue);

  type = buildingParams.getProperty("dhwfueltype");
  std::transform(type.begin(), type.end(), type.begin(), ::tolower);
  if (type == ELECTRIC)
    attributeValue = 1.0;
  else if (type == GAS)
    attributeValue = 2.0;
  else
    throw invalid_argument("dhwFuelType parameter must be one of 'gas' or 'electric'");
  setDhwEnergyCarrier(attributeValue);

  type = buildingParams.getProperty("bemtype");
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

  attributeValue = buildingParams.getPropertyAsDouble("interiorheatcapacity");
  setInteriorHeatCapacity(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("exteriorheatcapacity");
  setExteriorHeatCapacity(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("heatingpumpcontrol");
  setHeatingPumpControl(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("coolingpumpcontrol");
  setCoolingPumpControl(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("heatgainperperson");
  setHeatGainPerPerson(attributeValue);
  //specificFanPower
  attributeValue = buildingParams.getPropertyAsDouble("specificfanpower");
  setSpecificFanPower(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("fanflowcontrolfactor");
  setFanFlowControlFactor(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("coolingsetpointoccupied");
  setCoolingOccupiedSetpoint(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("coolingsetpointunoccupied");
  setCoolingUnoccupiedSetpoint(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("heatingsetpointoccupied");
  setHeatingOccupiedSetpoint(attributeValue);
  attributeValue = buildingParams.getPropertyAsDouble("heatingsetpointunoccupied");
  setHeatingUnoccupiedSetpoint(attributeValue);

#if (USE_NEW_BUILDING_PARAMS)
  attributeValue = buildingParams.getPropertyAsDouble("ventilationIntakeRateUnoccupied");
  setVentilationIntakeRateUnoccupied(attributeValue);

  attributeValue = buildingParams.getPropertyAsDouble("ventilationExhaustRateUnoccupied");
  setVentilationExhaustRateUnoccupied(attributeValue);

  attributeValue = buildingParams.getPropertyAsDouble("infiltrationRateUnoccupied");
  setInfiltrationRateUnoccupied(attributeValue);

  attributeValue = buildingParams.getPropertyAsDouble("lightingPowerFixedOccupied");
  setLightingPowerFixedOccupied(attributeValue);

  attributeValue = buildingParams.getPropertyAsDouble("lightingPowerFixedUnoccupied");
  setLightingPowerFixedUnoccupied(attributeValue);

  attributeValue = buildingParams.getPropertyAsDouble("electricAppliancePowerFixedOccupied");
  setElectricAppliancePowerFixedOccupied(attributeValue);

  attributeValue = buildingParams.getPropertyAsDouble("electricAppliancePowerFixedUnoccupied");
  setElectricAppliancePowerFixedUnoccupied(attributeValue);

  attributeValue = buildingParams.getPropertyAsDouble("gasAppliancePowerFixedOccupied");
  setGasAppliancePowerFixedOccupied(attributeValue);

  attributeValue = buildingParams.getPropertyAsDouble("gasAppliancePowerFixedUnoccupied");
  setGasAppliancePowerFixedUnoccupied(attributeValue);

  std::string scheduleFilePath = buildingParams.getProperty("schedulefilepath");
  if (scheduleFilePath == "")
    throw invalid_argument("scheduleFilePath building parameter is missing");
  setScheduleFilePath(scheduleFilePath);
#endif

  std::string weatherFilePath = buildingParams.getProperty("weatherfilepath");
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

