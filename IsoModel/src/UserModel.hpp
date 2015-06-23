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
#include "SimModel.hpp"
#include "ISOHourly.hpp"
#include "Properties.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>

namespace openstudio {

namespace isomodel {

class SimModel;
class WeatherData;


struct LatLon {

  double lat, lon;
  bool operator<(const LatLon& rhs) const;

};

class ISOMODEL_API UserModel
{
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
  
  void initializeParameter(void(UserModel::*setProp)(double), const Properties& props, std::string propertyName, bool required = true);
  void initializeParameter(void(UserModel::*setProp)(int), const Properties& props, std::string propertyName, bool required = true);
  void initializeParameter(void(UserModel::*setProp)(bool), const Properties& props, std::string propertyName, bool required = true);

  void loadBuilding(std::string buildingFile);
  void loadBuilding(std::string buildingFile, std::string defaultsFile);
  int weatherState(std::string header);
  void initializeSolar();

public:
  /**
   * Loads the specified weather data from disk.
   * Exposed to allow for separate loading from Ruby Scripts
   * Call setWeatherFilePath(path) then loadWeather() to update
   * the UserModel with a new set of weather data
   */
  void loadWeather();

  /**
   * Loads the weather from the specified array of doubles.
   *
   */
  void loadWeather(int block_size, double* weather_data);

  void loadAndSetWeather();

  /**
   * Loads an ISO model from the specified .ISO file
   */
  void load(std::string buildingFile);

  /**
  * Loads an ISO model file from the specified .ism file and defaults properties from the specified .ism.
  */
  void load(std::string buildingFile, std::string defaultsFile);

  UserModel();
  virtual ~UserModel();

  /**
   * Generates a SimModel from the specified parameters of the 
   * UserModel
   */
  SimModel toSimModel() const;
  ISOHourly toHourlyModel() const;

  const std::shared_ptr<WeatherData> weatherData()
  {
    return _weather;
  }

  const std::shared_ptr<EpwData> epwData()
  {
    return _edata;
  }

  /**
   * Indicates whether or not the user model loaded in correctly
   * If either the ISO file or the Weather File cannot be found
   * valid will be false
   * userModel.load(<filename>)
   * if(userModel.valid()){
   *     userModel.toSimModel().simulate();
   * }
   */
  bool valid() const  {
    return _valid;
  }

  std::string weatherFilePath() const
  {
    return _weatherFilePath;
  }

  // Location
  double terrainClass() const
  {
    return location.terrain();
  }

  // Structure
  double floorArea() const
  {
    return structure.floorArea();
  }
  double buildingHeight() const
  {
    return structure.buildingHeight();
  }

  // Population
  double buildingOccupancyFrom() const
  {
    return pop.daysStart();
  }
  double buildingOccupancyTo() const
  {
    return pop.daysEnd();
  }
  double equivFullLoadOccupancyFrom() const
  {
    return pop.hoursStart();
  }
  double equivFullLoadOccupancyTo() const
  {
    return pop.hoursEnd();
  }
  double peopleDensityOccupied() const
  {
    return pop.densityOccupied();
  }
  double peopleDensityUnoccupied() const
  {
    return pop.densityUnoccupied();
  }

  // Heating
  double heatingOccupiedSetpoint() const
  {
    return heating.temperatureSetPointOccupied();
  }
  double heatingUnoccupiedSetpoint() const
  {
    return heating.temperatureSetPointUnoccupied();
  }

  // Cooling
  double coolingOccupiedSetpoint() const
  {
    return cooling.temperatureSetPointOccupied();
  }
  double coolingUnoccupiedSetpoint() const
  {
    return cooling.temperatureSetPointUnoccupied();
  }

  // Building
  double elecPowerAppliancesOccupied() const
  {
    return building.electricApplianceHeatGainOccupied();
  }
  double elecPowerAppliancesUnoccupied() const
  {
    return building.electricApplianceHeatGainUnoccupied();
  }
  double gasPowerAppliancesOccupied() const
  {
    return building.gasApplianceHeatGainOccupied();
  }
  double gasPowerAppliancesUnoccupied() const
  {
    return building.gasApplianceHeatGainUnoccupied();
  }

  // Lighting
  double lightingPowerIntensityOccupied() const
  {
    return lights.powerDensityOccupied();
  }
  double lightingPowerIntensityUnoccupied() const
  {
    return lights.powerDensityUnoccupied();
  }
  double exteriorLightingPower() const
  {
    return lights.exteriorEnergy();
  }
  double daylightSensorSystem() const
  {
    return lights.dimmingFraction();
  }

  // Building
  double lightingOccupancySensorSystem() const
  {
    return building.lightingOccupancySensor();
  }
  double constantIlluminationControl() const
  {
    return building.constantIllumination();
  }

  // Cooling
  double coolingSystemCOP() const
  {
    return cooling.cop();
  }
  double coolingSystemIPLVToCOPRatio() const
  {
    return cooling.partialLoadValue();
  }

  // Heating
  double heatingEnergyCarrier() const
  {
    return heating.energyType();
  }
  double heatingSystemEfficiency() const
  {
    return heating.efficiency();
  }

  // Ventilation
  double ventilationType() const
  {
    return ventilation.ventType();
  }
  double freshAirFlowRate() const
  {
    return ventilation.supplyRate();
  }
  double supplyExhaustRate() const
  {
    return ventilation.supplyDifference();
  }
  double heatRecovery() const
  {
    return ventilation.heatRecoveryEfficiency();
  }
  double exhaustAirRecirclation() const
  {
    return ventilation.exhaustAirRecirculated();
  }

  // Structure.
  double buildingAirLeakage() const
  {
    return structure.infiltrationRate();
  }

  // Heating
  double dhwDemand() const
  {
    return heating.hotWaterDemand();
  }
  double dhwEfficiency() const
  {
    return heating.hotWaterSystemEfficiency();
  }

  // XXX: Appears to be unused. BAA@2015-06-16
  // double dhwDistributionSystem() const
  // {
  //   return _dhwDistributionSystem;
  // }

  double dhwEnergyCarrier() const
  {
    return heating.hotWaterEnergyType();
  }

  // Building
  double bemType() const
  {
    return building.buildingEnergyManagement();
  }

  // Structure
  double interiorHeatCapacity() const
  {
    return structure.interiorHeatCapacity();
  }

  // Ventilation
  double specificFanPower() const
  {
    return ventilation.fanPower();
  }
  double fanFlowControlFactor() const
  {
    return ventilation.fanControlFactor();
  }

  // Structure
  double wallUvalueS() const
  {
    return structure.wallUniform()[0];
  }
  double wallUvalueSE() const
  {
    return structure.wallUniform()[1];
  }
  double wallUvalueE() const
  {
    return structure.wallUniform()[2];
  }
  double wallUvalueNE() const
  {
    return structure.wallUniform()[3];
  }
  double wallUvalueN() const
  {
    return structure.wallUniform()[4];
  }
  double wallUvalueNW() const
  {
    return structure.wallUniform()[5];
  }
  double wallUvalueW() const
  {
    return structure.wallUniform()[6];
  }
  double wallUvalueSW() const
  {
    return structure.wallUniform()[7];
  }
  double roofUValue() const
  {
    return structure.wallUniform()[8];
  }

  double wallSolarAbsorptionS() const
  {
    return structure.wallSolarAbsorbtion()[0];
  }
  double wallSolarAbsorptionSE() const
  {
    return structure.wallSolarAbsorbtion()[1];
  }
  double wallSolarAbsorptionE() const
  {
    return structure.wallSolarAbsorbtion()[2];
  }
  double wallSolarAbsorptionNE() const
  {
    return structure.wallSolarAbsorbtion()[3];
  }
  double wallSolarAbsorptionN() const
  {
    return structure.wallSolarAbsorbtion()[4];
  }
  double wallSolarAbsorptionNW() const
  {
    return structure.wallSolarAbsorbtion()[5];
  }
  double wallSolarAbsorptionW() const
  {
    return structure.wallSolarAbsorbtion()[6];
  }
  double wallSolarAbsorptionSW() const
  {
    return structure.wallSolarAbsorbtion()[7];
  }
  double roofSolarAbsorption() const
  {
    return structure.wallSolarAbsorbtion()[8];
  }

  double wallThermalEmissivityS() const
  {
    return structure.wallThermalEmissivity()[0];
  }
  double wallThermalEmissivitySE() const
  {
    return structure.wallThermalEmissivity()[1];
  }
  double wallThermalEmissivityE() const
  {
    return structure.wallThermalEmissivity()[2];
  }
  double wallThermalEmissivityNE() const
  {
    return structure.wallThermalEmissivity()[3];
  }
  double wallThermalEmissivityN() const
  {
    return structure.wallThermalEmissivity()[4];
  }
  double wallThermalEmissivityNW() const
  {
    return structure.wallThermalEmissivity()[5];
  }
  double wallThermalEmissivityW() const
  {
    return structure.wallThermalEmissivity()[6];
  }
  double wallThermalEmissivitySW() const
  {
    return structure.wallThermalEmissivity()[7];
  }
  double roofThermalEmissivity() const
  {
    return structure.wallThermalEmissivity()[8];
  }
  
  double windowUvalueS() const
  {
    return structure.windowUniform()[0];
  }
  double windowUvalueSE() const
  {
    return structure.windowUniform()[1];
  }
  double windowUvalueE() const
  {
    return structure.windowUniform()[2];
  }
  double windowUvalueNE() const
  {
    return structure.windowUniform()[3];
  }
  double windowUvalueN() const
  {
    return structure.windowUniform()[4];
  }
  double windowUvalueNW() const
  {
    return structure.windowUniform()[5];
  }
  double windowUvalueW() const
  {
    return structure.windowUniform()[6];
  }
  double windowUvalueSW() const
  {
    return structure.windowUniform()[7];
  }
  double skylightUvalue()
  {
    return structure.windowUniform()[8];
  }

  double windowSHGCS() const
  {
    return structure.windowNormalIncidenceSolarEnergyTransmittance()[0];
  }
  double windowSHGCSE() const
  {
    return structure.windowNormalIncidenceSolarEnergyTransmittance()[1];
  }
  double windowSHGCE() const
  {
    return structure.windowNormalIncidenceSolarEnergyTransmittance()[2];
  }
  double windowSHGCNE() const
  {
    return structure.windowNormalIncidenceSolarEnergyTransmittance()[3];
  }
  double windowSHGCN() const
  {
    return structure.windowNormalIncidenceSolarEnergyTransmittance()[4];
  }
  double windowSHGCNW() const
  {
    return structure.windowNormalIncidenceSolarEnergyTransmittance()[5];
  }
  double windowSHGCW() const
  {
    return structure.windowNormalIncidenceSolarEnergyTransmittance()[6];
  }
  double windowSHGCSW() const
  {
    return structure.windowNormalIncidenceSolarEnergyTransmittance()[7];
  }
  double skylightSHGC()
  {
    return structure.windowNormalIncidenceSolarEnergyTransmittance()[8];
  }

//  XXX: Unused.
//  double roofSHGC() const
//  {
//    return structure.windowNormalIncidenceSolarEnergyTransmittance()[8];
//  }

  double windowSCFS() const
  {
    return structure.windowShadingCorrectionFactor()[0];
  }
  double windowSCFSE() const
  {
    return structure.windowShadingCorrectionFactor()[1];
  }
  double windowSCFE() const
  {
    return structure.windowShadingCorrectionFactor()[2];
  }
  double windowSCFNE() const
  {
    return structure.windowShadingCorrectionFactor()[3];
  }
  double windowSCFN() const
  {
    return structure.windowShadingCorrectionFactor()[4];
  }
  double windowSCFNW() const
  {
    return structure.windowShadingCorrectionFactor()[5];
  }
  double windowSCFW() const
  {
    return structure.windowShadingCorrectionFactor()[6];
  }
  double windowSCFSW() const
  {
    return structure.windowShadingCorrectionFactor()[7];
  }
  double skylightSCF()
  {
    return structure.windowShadingCorrectionFactor()[8];
  }

  double windowSDFS() const
  {
    return structure.windowShadingDevice()[0];
  }
  double windowSDFSE() const
  {
    return structure.windowShadingDevice()[1];
  }
  double windowSDFE() const
  {
    return structure.windowShadingDevice()[2];
  }
  double windowSDFNE() const
  {
    return structure.windowShadingDevice()[3];
  }
  double windowSDFN() const
  {
    return structure.windowShadingDevice()[4];
  }
  double windowSDFNW() const
  {
    return structure.windowShadingDevice()[5];
  }
  double windowSDFW() const
  {
    return structure.windowShadingDevice()[6];
  }
  double windowSDFSW() const
  {
    return structure.windowShadingDevice()[7];
  }
  double skylightSDF() const
  {
    return structure.windowShadingDevice()[8];
  }

  // Validation
  void setValid(bool val)
  {
    _valid = val;
  }

  // Structure
  void setWallUvalueS(double val)
  {
    structure.setWallUniform(0, val);
  }
  void setWallUvalueSE(double val)
  {
    structure.setWallUniform(1, val);
  }
  void setWallUvalueE(double val)
  {
    structure.setWallUniform(2, val);
  }
  void setWallUvalueNE(double val)
  {
    structure.setWallUniform(3, val);
  }
  void setWallUvalueN(double val)
  {
    structure.setWallUniform(4, val);
  }
  void setWallUvalueNW(double val)
  {
    structure.setWallUniform(5, val);
  }
  void setWallUvalueW(double val)
  {
    structure.setWallUniform(6, val);
  }
  void setWallUvalueSW(double val)
  {
    structure.setWallUniform(7, val);
  }
  void setRoofUValue(double val)
  {
    structure.setWallUniform(8, val);
  }

  void setWallSolarAbsorptionS(double val)
  {
    structure.setWallSolarAbsorbtion(0, val);
  }
  void setWallSolarAbsorptionSE(double val)
  {
    structure.setWallSolarAbsorbtion(1, val);
  }
  void setWallSolarAbsorptionE(double val)
  {
    structure.setWallSolarAbsorbtion(2, val);
  }
  void setWallSolarAbsorptionNE(double val)
  {
    structure.setWallSolarAbsorbtion(3, val);
  }
  void setWallSolarAbsorptionN(double val)
  {
    structure.setWallSolarAbsorbtion(4, val);
  }
  void setWallSolarAbsorptionNW(double val)
  {
    structure.setWallSolarAbsorbtion(5, val);
  }
  void setWallSolarAbsorptionW(double val)
  {
    structure.setWallSolarAbsorbtion(6, val);
  }
  void setWallSolarAbsorptionSW(double val)
  {
    structure.setWallSolarAbsorbtion(7, val);
  }
  void setRoofSolarAbsorption(double val)
  {
    structure.setWallSolarAbsorbtion(8, val);
  }

  void setWallThermalEmissivityS(double val)
  {
    structure.setWallThermalEmissivity(0, val);
  }
  void setWallThermalEmissivitySE(double val)
  {
    structure.setWallThermalEmissivity(1, val);
  }
  void setWallThermalEmissivityE(double val)
  {
    structure.setWallThermalEmissivity(2, val);
  }
  void setWallThermalEmissivityNE(double val)
  {
    structure.setWallThermalEmissivity(3, val);
  }
  void setWallThermalEmissivityN(double val)
  {
    structure.setWallThermalEmissivity(4, val);
  }
  void setWallThermalEmissivityNW(double val)
  {
    structure.setWallThermalEmissivity(5, val);
  }
  void setWallThermalEmissivityW(double val)
  {
    structure.setWallThermalEmissivity(6, val);
  }
  void setWallThermalEmissivitySW(double val)
  {
    structure.setWallThermalEmissivity(7, val);
  }
  void setRoofThermalEmissivity(double val)
  {
    structure.setWallThermalEmissivity(8, val);
  }

  void setWindowSCFS(double val)
  {
    structure.setWindowShadingCorrectionFactor(0, val);
  }
  void setWindowSCFSE(double val)
  {
    structure.setWindowShadingCorrectionFactor(1, val);
  }
  void setWindowSCFE(double val)
  {
    structure.setWindowShadingCorrectionFactor(2, val);
  }
  void setWindowSCFNE(double val)
  {
    structure.setWindowShadingCorrectionFactor(3, val);
  }
  void setWindowSCFN(double val)
  {
    structure.setWindowShadingCorrectionFactor(4, val);
  }
  void setWindowSCFNW(double val)
  {
    structure.setWindowShadingCorrectionFactor(5, val);
  }
  void setWindowSCFW(double val)
  {
    structure.setWindowShadingCorrectionFactor(6, val);
  }
  void setWindowSCFSW(double val)
  {
    structure.setWindowShadingCorrectionFactor(7, val);
  }
  void setSkylightSCF(double val)
  {
    structure.setWindowShadingCorrectionFactor(8, val);
  }


  void setWindowSDFS(double val)
  {
    structure.setWindowShadingDevice(0, val);
  }
  void setWindowSDFSE(double val)
  {
    structure.setWindowShadingDevice(1, val);
  }
  void setWindowSDFE(double val)
  {
    structure.setWindowShadingDevice(2, val);
  }
  void setWindowSDFNE(double val)
  {
    structure.setWindowShadingDevice(3, val);
  }
  void setWindowSDFN(double val)
  {
    structure.setWindowShadingDevice(4, val);
  }
  void setWindowSDFNW(double val)
  {
    structure.setWindowShadingDevice(5, val);
  }
  void setWindowSDFW(double val)
  {
    structure.setWindowShadingDevice(6, val);
  }
  void setWindowSDFSW(double val)
  {
    structure.setWindowShadingDevice(7, val);
  }
  void setSkylightSDF(double val)
  {
    structure.setWindowShadingDevice(8, val);
  }

  void setWindowSHGCS(double val)
  {
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(0, val);
  }
  void setWindowSHGCSE(double val)
  {
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(1, val);
  }
  void setWindowSHGCE(double val)
  {
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(2, val);
  }
  void setWindowSHGCNE(double val)
  {
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(3, val);
  }
  void setWindowSHGCN(double val)
  {
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(4, val);
  }
  void setWindowSHGCNW(double val)
  {
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(5, val);
  }
  void setWindowSHGCW(double val)
  {
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(6, val);
  }
  void setWindowSHGCSW(double val)
  {
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(7, val);
  }
  void setSkylightSHGC(double val)
  {
    structure.setWindowNormalIncidenceSolarEnergyTransmittance(8, val);
  }

//  XXX: Unused.
//  void setRoofSHGC(double val)
//  {
//    structure.setWindowNormalIncidenceSolarEnergyTransmittance(8, val);
//  }

  void setWindowUvalueS(double val)
  {
    structure.setWindowUniform(0, val);
  }
  void setWindowUvalueSE(double val)
  {
    structure.setWindowUniform(1, val);
  }
  void setWindowUvalueE(double val)
  {
    structure.setWindowUniform(2, val);
  }
  void setWindowUvalueNE(double val)
  {
    structure.setWindowUniform(3, val);
  }
  void setWindowUvalueN(double val)
  {
    structure.setWindowUniform(4, val);
  }
  void setWindowUvalueNW(double val)
  {
    structure.setWindowUniform(5, val);
  }
  void setWindowUvalueW(double val)
  {
    structure.setWindowUniform(6, val);
  }
  void setWindowUvalueSW(double val)
  {
    structure.setWindowUniform(7, val);
  }
  void setSkylightUvalue(double val)
  {
    structure.setWindowUniform(8, val);
  }

  double wallAreaS()
  {
    return structure.wallArea()[0];
  }
  double wallAreaSE()
  {
    return structure.wallArea()[1];
  }
  double wallAreaE()
  {
    return structure.wallArea()[2];
  }
  double wallAreaNE()
  {
    return structure.wallArea()[3];
  }
  double wallAreaN()
  {
    return structure.wallArea()[4];
  }
  double wallAreaNW()
  {
    return structure.wallArea()[5];
  }
  double wallAreaW()
  {
    return structure.wallArea()[6];
  }
  double wallAreaSW()
  {
    return structure.wallArea()[7];
  }
  double roofArea()
  {
    return structure.wallArea()[8];
  }

  double windowAreaS()
  {
    return structure.windowArea()[0];
  }
  double windowAreaSE()
  {
    return structure.windowArea()[1];
  }
  double windowAreaE()
  {
    return structure.windowArea()[2];
  }
  double windowAreaNE()
  {
    return structure.windowArea()[3];
  }
  double windowAreaN()
  {
    return structure.windowArea()[4];
  }
  double windowAreaNW()
  {
    return structure.windowArea()[5];
  }
  double windowAreaW()
  {
    return structure.windowArea()[6];
  }
  double windowAreaSW()
  {
    return structure.windowArea()[7];
  }
  double skylightArea()
  {
    return structure.windowArea()[8];
  }


  double exteriorHeatCapacity()
  {
    return structure.wallHeatCapacity();
  }

  // Heating
  double hvacWasteFactor()
  {
    return heating.hotcoldWasteFactor();
  }
  double hvacHeatingLossFactor()
  {
    return heating.hvacLossFactor();
  }
  // Cooling
  double hvacCoolingLossFactor()
  {
    return cooling.hvacLossFactor();
  }
  // Heating
  double dhwDistributionEfficiency()
  {
    return heating.hotWaterDistributionEfficiency();
  }
  double heatingPumpControl()
  {
    return heating.pumpControlReduction();
  }
  // Cooling
  double coolingPumpControl()
  {
    return cooling.pumpControlReduction();
  }

  // Population
  double heatGainPerPerson()
  {
    return pop.heatGainPerPerson();
  }

  // Weather
  void setWeatherFilePath(std::string val)
  {
    _weatherFilePath = val;
  }
  void setTerrainClass(double val)
  {
    location.setTerrain(val);
  }

  // Structure.
  void setFloorArea(double val)
  {
    structure.setFloorArea(val);
  }
  void setBuildingHeight(double val)
  {
    structure.setBuildingHeight(val);
  }

  // Population
  void setBuildingOccupancyFrom(double val)
  {
    pop.setDaysStart(val);
  }
  void setBuildingOccupancyTo(double val)
  {
    pop.setDaysEnd(val);
  }
  void setEquivFullLoadOccupancyFrom(double val)
  {
    pop.setHoursStart(val);
  }
  void setEquivFullLoadOccupancyTo(double val)
  {
    pop.setHoursEnd(val);
  }
  void setPeopleDensityOccupied(double val)
  {
    pop.setDensityOccupied(val);
  }
  void setPeopleDensityUnoccupied(double val)
  {
    pop.setDensityUnoccupied(val);
  }

  // Heating
  void setHeatingOccupiedSetpoint(double val)
  {
    heating.setTemperatureSetPointOccupied(val);
  }
  void setHeatingUnoccupiedSetpoint(double val)
  {
    heating.setTemperatureSetPointUnoccupied(val);
  }
  // Cooling
  void setCoolingOccupiedSetpoint(double val)
  {
    cooling.setTemperatureSetPointOccupied(val);
  }
  void setCoolingUnoccupiedSetpoint(double val)
  {
    cooling.setTemperatureSetPointUnoccupied(val);
  }

  // Building
  void setElecPowerAppliancesOccupied(double val)
  {
    building.setElectricApplianceHeatGainOccupied(val);
  }
  void setElecPowerAppliancesUnoccupied(double val)
  {
    building.setElectricApplianceHeatGainUnoccupied(val);
  }
  void setGasPowerAppliancesOccupied(double val)
  {
    building.setGasApplianceHeatGainOccupied(val);
  }
  void setGasPowerAppliancesUnoccupied(double val)
  {
    building.setGasApplianceHeatGainUnoccupied(val);
  }

  // Lighting
  void setLightingPowerIntensityOccupied(double val)
  {
    lights.setPowerDensityOccupied(val);
  }
  void setLightingPowerIntensityUnoccupied(double val)
  {
    lights.setPowerDensityUnoccupied(val);
  }
  void setExteriorLightingPower(double val)
  {
    lights.setExteriorEnergy(val);
  }
  void setDaylightSensorSystem(double val)
  {
    lights.setDimmingFraction(val);
  }
  // Building
  void setLightingOccupancySensorSystem(double val)
  {
    building.setLightingOccupancySensor(val);
  }
  void setConstantIlluminationControl(double val)
  {
    building.setConstantIllumination(val);
  }

  // Cooling
  void setCoolingSystemCOP(double val)
  {
    cooling.setCop(val);
  }
  void setCoolingSystemIPLVToCOPRatio(double val)
  {
    cooling.setPartialLoadValue(val);
  }
  // Heating
  void setHeatingEnergyCarrier(double val)
  {
    heating.setEnergyType(val);
  }
  void setHeatingSystemEfficiency(double val)
  {
    heating.setEfficiency(val);
  }

  // Ventilation
  void setVentilationType(double val)
  {
    ventilation.setVentType(val);
  }
  void setFreshAirFlowRate(double val)
  {
    ventilation.setSupplyRate(val);
  }
  void setSupplyExhaustRate(double val)
  {
    ventilation.setSupplyDifference(val);
  }
  void setHeatRecovery(double val)
  {
    ventilation.setHeatRecoveryEfficiency(val);
  }
  void setExhaustAirRecirclation(double val)
  {
    ventilation.setExhaustAirRecirculated(val);
  }
  // Structure.
  void setBuildingAirLeakage(double val)
  {
    structure.setInfiltrationRate(val);
  }

  // Heating.
  void setDhwDemand(double val)
  {
    heating.setHotWaterDemand(val);
  }
  void setDhwEfficiency(double val)
  {
    heating.setHotWaterSystemEfficiency(val);
  }
  void setDhwDistributionSystem(double val)
  {
    heating.setHotWaterDistributionEfficiency(val);
  }
  void setDhwEnergyCarrier(double val)
  {
    heating.setHotWaterEnergyType(val);
  }

  // Building.
  void setBemType(double val)
  {
    building.setBuildingEnergyManagement(val);
  }

  // Structure.
  void setInteriorHeatCapacity(double val)
  {
    structure.setInteriorHeatCapacity(val);
  }

  // Ventilation.
  void setSpecificFanPower(double val)
  {
    ventilation.setFanPower(val);
  }
  void setFanFlowControlFactor(double val)
  {
    ventilation.setFanControlFactor(val);
  }

  // Structure.
  void setWallAreaS(double val)
  {
    structure.setWallArea(0, val);
  }
  void setWallAreaSE(double val)
  {
    structure.setWallArea(1, val);
  }
  void setWallAreaE(double val)
  {
    structure.setWallArea(2, val);
  }
  void setWallAreaNE(double val)
  {
    structure.setWallArea(3, val);
  }
  void setWallAreaN(double val)
  {
    structure.setWallArea(4, val);
  }
  void setWallAreaNW(double val)
  {
    structure.setWallArea(5, val);
  }
  void setWallAreaW(double val)
  {
    structure.setWallArea(6, val);
  }
  void setWallAreaSW(double val)
  {
    structure.setWallArea(7, val);
  }
  void setRoofArea(double val)
  {
    structure.setWallArea(8, val);
  }

  void setWindowAreaS(double val)
  {
    structure.setWindowArea(0, val);
  }
  void setWindowAreaSE(double val)
  {
    structure.setWindowArea(1, val);
  }
  void setWindowAreaE(double val)
  {
    structure.setWindowArea(2, val);
  }
  void setWindowAreaNE(double val)
  {
    structure.setWindowArea(3, val);
  }
  void setWindowAreaN(double val)
  {
    structure.setWindowArea(4, val);
  }
  void setWindowAreaNW(double val)
  {
    structure.setWindowArea(5, val);
  }
  void setWindowAreaW(double val)
  {
    structure.setWindowArea(6, val);
  }
  void setWindowAreaSW(double val)
  {
    structure.setWindowArea(7, val);
  }
  void setSkylightArea(double val)
  {
    structure.setWindowArea(8, val);
  }

  void setExteriorHeatCapacity(double val)
  {
    structure.setWallHeatCapacity(val);
  }

  // Heating
  void setHvacWasteFactor(double val)
  {
    heating.setHotcoldWasteFactor(val);
  }
  void setHvacHeatingLossFactor(double val)
  {
    heating.setHvacLossFactor(val);
  }
  // Cooling
  void setHvacCoolingLossFactor(double val)
  {
    cooling.setHvacLossFactor(val);
  }
  // Heating
  void setDhwDistributionEfficiency(double val)
  {
    heating.setHotWaterDistributionEfficiency(val);
  }
  void setHeatingPumpControl(double val)
  {
    heating.setPumpControlReduction(val);
  }
  // Cooling
  void setCoolingPumpControl(double val)
  {
    cooling.setPumpControlReduction(val);
  }

  // Population
  void setHeatGainPerPerson(double val)
  {
    pop.setHeatGainPerPerson(val);
  }


  // TODO: These properties aren't used by the simulations yet -BAA@2015-06-18
  // Population
  /**
  *
  */
  std::string scheduleFilePath() const {
    return pop.scheduleFilePath();
  }

  void setScheduleFilePath(std::string scheduleFilePath) {
    pop.setScheduleFilePath(scheduleFilePath);
  }

  // Building
  /**
  *
  */
  double electricAppliancePowerFixedOccupied() const {
    return building.electricAppliancePowerFixedOccupied();
  }

  void setElectricAppliancePowerFixedOccupied(double electricAppliancePowerFixedOccupied) {
    building.setElectricAppliancePowerFixedOccupied(electricAppliancePowerFixedOccupied);
  }

  /**
  *
  */
  double electricAppliancePowerFixedUnoccupied() const {
    return building.electricAppliancePowerFixedUnoccupied();
  }

  void setElectricAppliancePowerFixedUnoccupied(double electricAppliancePowerFixedUnoccupied) {
    building.setElectricAppliancePowerFixedUnoccupied(electricAppliancePowerFixedUnoccupied);
  }

  /**
  *
  */
  double gasAppliancePowerFixedOccupied() const {
    return building.gasAppliancePowerFixedOccupied();
  }

  void setGasAppliancePowerFixedOccupied(double gasAppliancePowerFixedOccupied) {
    building.setGasAppliancePowerFixedOccupied(gasAppliancePowerFixedOccupied);
  }

  /**
  *
  */
  double gasAppliancePowerFixedUnoccupied() const {
    return building.gasAppliancePowerFixedUnoccupied();
  }

  void setGasAppliancePowerFixedUnoccupied(double gasAppliancePowerFixedUnoccupied) {
    building.setGasAppliancePowerFixedUnoccupied(gasAppliancePowerFixedUnoccupied);
  }

  // Lighting
  /**
  *
  */
  double lightingPowerFixedOccupied() const {
    return lights.lightingPowerFixedOccupied();
  }

  void setLightingPowerFixedOccupied(double lightingPowerFixedOccupied) {
    lights.setLightingPowerFixedOccupied(lightingPowerFixedOccupied);
  }

  /**
  *
  */
  double lightingPowerFixedUnoccupied() const {
    return lights.lightingPowerFixedUnoccupied();
  }

  void setLightingPowerFixedUnoccupied(double lightingPowerFixedUnoccupied) {
    lights.setLightingPowerFixedUnoccupied(lightingPowerFixedUnoccupied);
  }

  // Ventilation
  /**
  *
  */
  double infiltrationRateUnoccupied() const {
    return ventilation.infiltrationRateUnoccupied();
  }

  void setInfiltrationRateUnoccupied(double infiltrationRateUnoccupied) {
    ventilation.setInfiltrationRateUnoccupied(infiltrationRateUnoccupied);
  }

  /**
  *
  */
  double ventilationExhaustRateUnoccupied() const {
    return ventilation.ventilationExhaustRateUnoccupied();
  }

  void setVentilationExhaustRateUnoccupied(double ventilationExhaustRateUnoccupied) {
    ventilation.setVentilationExhaustRateUnoccupied(ventilationExhaustRateUnoccupied);
  }

  /**
  *
  */
  double ventilationIntakeRateUnoccupied() const {
    return ventilation.ventilationIntakeRateUnoccupied();
  }

  void setVentilationIntakeRateUnoccupied(double ventilationIntakeRateUnoccupied) {
    ventilation.setVentilationIntakeRateUnoccupied(ventilationIntakeRateUnoccupied);
  }


  // Getters and setters for default properties.

  // Building
  /**
  *
  */
  double externalEquipment() const {
    return building.externalEquipment();
  }

  void setExternalEquipment(double externalEquipment) {
    building.setExternalEquipment(externalEquipment);
  }

  // Cooling
  /**
  *
  */
  bool forcedAirCooling() const {
    return cooling.forcedAirCooling();
  }

  void setForcedAirCooling(bool forcedAirCooling) {
    cooling.setForcedAirCooling(forcedAirCooling);
  }

  /**
  *
  */
  double T_cl_ctrl_flag() const {
    return cooling.T_cl_ctrl_flag();
  }

  void setT_cl_ctrl_flag(double T_cl_ctrl_flag) {
    cooling.setT_cl_ctrl_flag(T_cl_ctrl_flag);
  }

  /**
  *
  */
  double dT_supp_cl() const {
    return cooling.dT_supp_cl();
  }

  void setDT_supp_cl(double dT_supp_cl) {
    cooling.setDT_supp_cl(dT_supp_cl);
  }

  /**
  *
  */
  double DC_YesNo() const {
    return cooling.DC_YesNo();
  }

  void setDC_YesNo(double DC_YesNo) {
    cooling.setDC_YesNo(DC_YesNo);
  }

  /**
  *
  */
  double eta_DC_network() const {
    return cooling.eta_DC_network();
  }

  void setEta_DC_network(double eta_DC_network) {
    cooling.setEta_DC_network(eta_DC_network);
  }

  /**
  *
  */
  double eta_DC_COP() const {
    return cooling.eta_DC_COP();
  }

  void setEta_DC_COP(double eta_DC_COP) {
    cooling.setEta_DC_COP(eta_DC_COP);
  }

  /**
  *
  */
  double eta_DC_frac_abs() const {
    return cooling.eta_DC_frac_abs();
  }

  void setEta_DC_frac_abs(double eta_DC_frac_abs) {
    cooling.setEta_DC_frac_abs(eta_DC_frac_abs);
  }

  /**
  *
  */
  double eta_DC_COP_abs() const {
    return cooling.eta_DC_COP_abs();
  }

  void setEta_DC_COP_abs(double eta_DC_COP_abs) {
    cooling.setEta_DC_COP_abs(eta_DC_COP_abs);
  }

  /**
  *
  */
  double frac_DC_free() const {
    return cooling.frac_DC_free();
  }

  void setFrac_DC_free(double frac_DC_free) {
    cooling.setFrac_DC_free(frac_DC_free);
  }

  /**
  *
  */
  double E_pumps_cl() const {
    return cooling.E_pumps();
  }

  void setE_pumps_cl(double E_pumps) {
    cooling.setE_pumps(E_pumps);
  }

  // Heating.
  /**
  *
  */
  bool forcedAirHeating() const {
    return heating.forcedAirHeating();
  }

  void setForcedAirHeating(bool forcedAirHeating) {
    heating.setForcedAirHeating(forcedAirHeating);
  }

  /**
  *
  */
  double dT_supp_ht() const {
    return heating.dT_supp_ht();
  }

  void setDT_supp_ht(double dT_supp_ht) {
    heating.setDT_supp_ht(dT_supp_ht);
  }

  /**
  *
  */
  double E_pumps_ht() const {
    return heating.E_pumps();
  }

  void setE_pumps_ht(double E_pumps) {
    heating.setE_pumps(E_pumps);
  }

  /**
  *
  */
  double T_ht_ctrl_flag() const {
    return heating.T_ht_ctrl_flag();
  }

  void setT_ht_ctrl_flag(double T_ht_ctrl_flag) {
    heating.setT_ht_ctrl_flag(T_ht_ctrl_flag);
  }

  /**
  *
  */
  double a_H0() const {
    return heating.a_H0();
  }

  void setA_H0(double a_H0) {
    heating.setA_H0(a_H0);
  }

  /**
  *
  */
  double tau_H0() const {
    return heating.tau_H0();
  }

  void setTau_H0(double tau_H0) {
    heating.setTau_H0(tau_H0);
  }

  /**
  *
  */
  double DH_YesNo() const {
    return heating.DH_YesNo();
  }

  void setDH_YesNo(double DH_YesNo) {
    heating.setDH_YesNo(DH_YesNo);
  }

  /**
  *
  */
  double eta_DH_network() const {
    return heating.eta_DH_network();
  }

  void setEta_DH_network(double eta_DH_network) {
    heating.setEta_DH_network(eta_DH_network);
  }

  /**
  *
  */
  double eta_DH_sys() const {
    return heating.eta_DH_sys();
  }

  void setEta_DH_sys(double eta_DH_sys) {
    heating.setEta_DH_sys(eta_DH_sys);
  }

  /**
  *
  */
  double frac_DH_free() const {
    return heating.frac_DH_free();
  }

  void setFrac_DH_free(double frac_DH_free) {
    heating.setFrac_DH_free(frac_DH_free);
  }

  /**
  *
  */
  double dhw_tset() const {
    return heating.dhw_tset();
  }

  void setDhw_tset(double dhw_tset) {
    heating.setDhw_tset(dhw_tset);
  }

  /**
  *
  */
  double dhw_tsupply() const {
    return heating.dhw_tsupply();
  }

  void setDhw_tsupply(double dhw_tsupply) {
    heating.setDhw_tsupply(dhw_tsupply);
  }

  // Lighting
  /**
  *
  */
  double n_day_start() const {
    return lights.n_day_start();
  }

  void setN_day_start(double n_day_start) {
    lights.setN_day_start(n_day_start);
  }

  /**
  *
  */
  double n_day_end() const {
    return lights.n_day_end();
  }

  void setN_day_end(double n_day_end) {
    lights.setN_day_end(n_day_end);
  }

  /**
  *
  */
  double n_weeks() const {
    return lights.n_weeks();
  }

  void setN_weeks(double n_weeks) {
    lights.setN_weeks(n_weeks);
  }

  /**
  *
  */
  double elecInternalGains() const {
    return lights.elecInternalGains();
  }

  void setElecInternalGains(double elecInternalGains) {
    lights.setElecInternalGains(elecInternalGains);
  }

  /**
  *
  */
  double permLightPowerDensity() const {
    return lights.permLightPowerDensity();
  }

  void setPermLightPowerDensity(double permLightPowerDensity) {
    lights.setPermLightPowerDensity(permLightPowerDensity);
  }

  /**
  *
  */
  double presenceSensorAd() const {
    return lights.presenceSensorAd();
  }

  void setPresenceSensorAd(double presenceSensorAd) {
    lights.setPresenceSensorAd(presenceSensorAd);
  }

  /**
  *
  */
  double automaticAd() const {
    return lights.automaticAd();
  }

  void setAutomaticAd(double automaticAd) {
    lights.setAutomaticAd(automaticAd);
  }

  /**
  *
  */
  double presenceAutoAd() const {
    return lights.presenceAutoAd();
  }

  void setPresenceAutoAd(double presenceAutoAd) {
    lights.setPresenceAutoAd(presenceAutoAd);
  }

  /**
  *
  */
  double manualSwitchAd() const {
    return lights.manualSwitchAd();
  }

  void setManualSwitchAd(double manualSwitchAd) {
    lights.setManualSwitchAd(manualSwitchAd);
  }

  /**
  *
  */
  double presenceSensorLux() const {
    return lights.presenceSensorLux();
  }

  void setPresenceSensorLux(double presenceSensorLux) {
    lights.setPresenceSensorLux(presenceSensorLux);
  }

  /**
  *
  */
  double automaticLux() const {
    return lights.automaticLux();
  }

  void setAutomaticLux(double automaticLux) {
    lights.setAutomaticLux(automaticLux);
  }

  /**
  *
  */
  double presenceAutoLux() const {
    return lights.presenceAutoLux();
  }

  void setPresenceAutoLux(double presenceAutoLux) {
    lights.setPresenceAutoLux(presenceAutoLux);
  }

  /**
  *
  */
  double manualSwitchLux() const {
    return lights.manualSwitchLux();
  }

  void setManualSwitchLux(double manualSwitchLux) {
    lights.setManualSwitchLux(manualSwitchLux);
  }

  /**
  *
  */
  double naturallyLightedArea() const {
    return lights.naturallyLightedArea();
  }

  void setNaturallyLightedArea(double naturallyLightedArea) {
    lights.setNaturallyLightedArea(naturallyLightedArea);
  }

  // Physical Quantities
  /**
  *
  */
  double rhoCpAir() const {
    return phys.rhoCpAir();
  }

  void setRhoCpAir(double rhoCpAir) {
    phys.setRhoCpAir(rhoCpAir);
  }

  /**
  *
  */
  double rhoCpWater() const {
    return phys.rhoCpWater();
  }

  void setRhoCpWater(double rhoCpWater) {
    phys.setRhoCpWater(rhoCpWater);
  }

  // Simulation Settings.
  /**
  *
  */
  double phiIntFractionToAirNode() const {
    return simSettings.phiIntFractionToAirNode();
  }
  
  void setPhiIntFractionToAirNode(double phiIntFractionToAirNode) {
    simSettings.setPhiIntFractionToAirNode(phiIntFractionToAirNode);
  }
  
  /**
  *
  */
  double phiSolFractionToAirNode() const {
    return simSettings.phiSolFractionToAirNode();
  }
  
  void setPhiSolFractionToAirNode(double phiSolFractionToAirNode) {
    simSettings.setPhiSolFractionToAirNode(phiSolFractionToAirNode);
  }
  
  /**
  *
  */
  double hci() const {
    return simSettings.hci();
  }
  
  void setHci(double hci) {
    simSettings.setHci(hci);
  }
  
  /**
  *
  */
  double hri() const {
    return simSettings.hri();
  }
  
  void setHri(double hri) {
    simSettings.setHri(hri);
  }

  // Structure.
  /**
  *
  */
  double R_se() const {
    return structure.R_se();
  }

  void setR_se(double R_se) {
    structure.setR_se(R_se);
  }

  /**
  *
  */
  double irradianceForMaxShadingUse() const {
    return structure.irradianceForMaxShadingUse();
  }

  void setIrradianceForMaxShadingUse(double irradianceForMaxShadingUse) {
    structure.setIrradianceForMaxShadingUse(irradianceForMaxShadingUse);
  }

  /**
  *
  */
  double shadingFactorAtMaxUse() const {
    return structure.shadingFactorAtMaxUse();
  }

  void setShadingFactorAtMaxUse(double shadingFactorAtMaxUse) {
    structure.setShadingFactorAtMaxUse(shadingFactorAtMaxUse);
  }

  /**
  *
  */
  double totalAreaPerFloorArea() const {
    return structure.totalAreaPerFloorArea();
  }

  void setTotalAreaPerFloorArea(double totalAreaPerFloorArea) {
    structure.setTotalAreaPerFloorArea(totalAreaPerFloorArea);
  }

  /**
  *
  */
  double win_ff() const {
    return structure.win_ff();
  }

  void setWin_ff(double win_ff) {
    structure.setWin_ff(win_ff);
  }

  /**
  *
  */
  double win_F_W() const {
    return structure.win_F_W();
  }

  void setWin_F_W(double win_F_W) {
    structure.setWin_F_W(win_F_W);
  }

  /**
  *
  */
  double R_sc_ext() const {
    return structure.R_sc_ext();
  }

  void setR_sc_ext(double R_sc_ext) {
    structure.setR_sc_ext(R_sc_ext);
  }

  // Ventilation
  /**
  *
  */
  double ventPreheatDegC() const {
    return ventilation.ventPreheatDegC();
  }

  void setVentPreheatDegC(double ventPreheatDegC) {
    ventilation.setVentPreheatDegC(ventPreheatDegC);
  }

  /**
  *
  */
  double n50() const {
    return ventilation.n50();
  }

  void setN50(double n50) {
    ventilation.setN50(n50);
  }

  /**
  *
  */
  double hzone() const {
    return ventilation.hzone();
  }

  void setHzone(double hzone) {
    ventilation.setHzone(hzone);
  }

  /**
  *
  */
  double p_exp() const {
    return ventilation.p_exp();
  }

  void setP_exp(double p_exp) {
    ventilation.setP_exp(p_exp);
  }

  /**
  *
  */
  double zone_frac() const {
    return ventilation.zone_frac();
  }

  void setZone_frac(double zone_frac) {
    ventilation.setZone_frac(zone_frac);
  }

  /**
  *
  */
  double stack_exp() const {
    return ventilation.stack_exp();
  }

  void setStack_exp(double stack_exp) {
    ventilation.setStack_exp(stack_exp);
  }

  /**
  *
  */
  double stack_coeff() const {
    return ventilation.stack_coeff();
  }

  void setStack_coeff(double stack_coeff) {
    ventilation.setStack_coeff(stack_coeff);
  }

  /**
  *
  */
  double wind_exp() const {
    return ventilation.wind_exp();
  }

  void setWind_exp(double wind_exp) {
    ventilation.setWind_exp(wind_exp);
  }

  /**
  *
  */
  double wind_coeff() const {
    return ventilation.wind_coeff();
  }

  void setWind_coeff(double wind_coeff) {
    ventilation.setWind_coeff(wind_coeff);
  }

  /**
  *
  */
  double dCp() const {
    return ventilation.dCp();
  }

  void setDCp(double dCp) {
    ventilation.setDCp(dCp);
  }

  /**
  *
  */
  double vent_rate_flag() const {
    return ventilation.vent_rate_flag();
  }

  void setVent_rate_flag(double vent_rate_flag) {
    ventilation.setVent_rate_flag(vent_rate_flag);
  }

  /**
  *
  */
  double H_ve() const {
    return ventilation.H_ve();
  }

  void setH_ve(double H_ve) {
    ventilation.setH_ve(H_ve);
  }

};

} // isomodel
} // openstudio

#endif // ISOMODEL_USERMODEL_HPP
