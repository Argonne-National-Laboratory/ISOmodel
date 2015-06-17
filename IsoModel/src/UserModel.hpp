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
  double _terrainClass;
  double _floorArea;
  double _buildingHeight;
  double _buildingOccupancyFrom;
  double _buildingOccupancyTo;
  double _equivFullLoadOccupancyFrom;
  double _equivFullLoadOccupancyTo;
  double _peopleDensityOccupied;
  double _peopleDensityUnoccupied;
  double _heatingOccupiedSetpoint;
  double _heatingUnoccupiedSetpoint;
  double _coolingOccupiedSetpoint;
  double _coolingUnoccupiedSetpoint;
  double _elecPowerAppliancesOccupied;
  double _elecPowerAppliancesUnoccupied;
  double _gasPowerAppliancesOccupied;
  double _gasPowerAppliancesUnoccupied;
  double _lightingPowerIntensityOccupied;
  double _lightingPowerIntensityUnoccupied;
  double _exteriorLightingPower;
  double _daylightSensorSystem;
  double _lightingOccupancySensorSystem;
  double _constantIlluminationControl;
  double _coolingSystemCOP;
  double _coolingSystemIPLVToCOPRatio;
  double _heatingEnergyCarrier;
  double _heatingSystemEfficiency;
  double _ventilationType;
  double _freshAirFlowRate;
  double _supplyExhaustRate;
  double _heatRecovery;
  double _exhaustAirRecirclation;
  double _buildingAirLeakage;
  double _dhwDemand;
  double _dhwEfficiency;
  double _dhwDistributionSystem;
  double _dhwEnergyCarrier;
  double _bemType;
  double _interiorHeatCapacity;
  double _specificFanPower;
  double _fanFlowControlFactor;
  // double _roofSHGC; // Unused. _skylightSHGC is the same.

  /* Area */
  double _wallAreaS;
  double _wallAreaSE;
  double _wallAreaE;
  double _wallAreaNE;
  double _wallAreaN;
  double _wallAreaNW;
  double _wallAreaW;
  double _wallAreaSW;
  double _roofArea;

  /* UValue */
  double _wallUvalueS;
  double _wallUvalueSE;
  double _wallUvalueE;
  double _wallUvalueNE;
  double _wallUvalueN;
  double _wallUvalueNW;
  double _wallUvalueW;
  double _wallUvalueSW;
  double _roofUValue;

  /* SolarAbsorption */
  double _wallSolarAbsorptionS;
  double _wallSolarAbsorptionSE;
  double _wallSolarAbsorptionE;
  double _wallSolarAbsorptionNE;
  double _wallSolarAbsorptionN;
  double _wallSolarAbsorptionNW;
  double _wallSolarAbsorptionW;
  double _wallSolarAbsorptionSW;
  double _roofSolarAbsorption;

  /* ThermalEmissivity */
  double _wallThermalEmissivityS;
  double _wallThermalEmissivitySE;
  double _wallThermalEmissivityE;
  double _wallThermalEmissivityNE;
  double _wallThermalEmissivityN;
  double _wallThermalEmissivityNW;
  double _wallThermalEmissivityW;
  double _wallThermalEmissivitySW;
  double _roofThermalEmissivity;

  double _windowAreaS;
  double _windowAreaSE;
  double _windowAreaE;
  double _windowAreaNE;
  double _windowAreaN;
  double _windowAreaNW;
  double _windowAreaW;
  double _windowAreaSW;
  double _skylightArea;

  double _windowUvalueS;
  double _windowUvalueSE;
  double _windowUvalueE;
  double _windowUvalueNE;
  double _windowUvalueN;
  double _windowUvalueNW;
  double _windowUvalueW;
  double _windowUvalueSW;
  double _skylightUvalue;

  double _windowSHGCS;
  double _windowSHGCSE;
  double _windowSHGCE;
  double _windowSHGCNE;
  double _windowSHGCN;
  double _windowSHGCNW;
  double _windowSHGCW;
  double _windowSHGCSW;
  double _skylightSHGC;

  double _windowSCFS;
  double _windowSCFSE;
  double _windowSCFE;
  double _windowSCFNE;
  double _windowSCFN;
  double _windowSCFNW;
  double _windowSCFW;
  double _windowSCFSW;
  double _skylightSCF;

  double _windowSDFS;
  double _windowSDFSE;
  double _windowSDFE;
  double _windowSDFNE;
  double _windowSDFN;
  double _windowSDFNW;
  double _windowSDFW;
  double _windowSDFSW;
  double _skylightSDF;

  double _exteriorHeatCapacity;
  double _infiltration;
  double _hvacWasteFactor;
  double _hvacHeatingLossFactor;
  double _hvacCoolingLossFactor;
  double _dhwDistributionEfficiency;
  double _heatingPumpControl;
  double _coolingPumpControl;
  double _heatGainPerPerson;

  double _ventilationIntakeRateUnoccupied;
  double _ventilationExhaustRateUnoccupied;
  double _infiltrationRateUnoccupied;
  double _lightingPowerFixedOccupied;
  double _lightingPowerFixedUnoccupied;
  double _electricAppliancePowerFixedOccupied;
  double _electricAppliancePowerFixedUnoccupied;
  double _gasAppliancePowerFixedOccupied;
  double _gasAppliancePowerFixedUnoccupied;

  std::string _weatherFilePath, _scheduleFilePath;
  std::string dataFile;

  void initializeParameters(const Properties& props);
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
  double dhwDistributionSystem() const
  {
    return _dhwDistributionSystem;
  }

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

  // XXX: Not used. Seems redundant with buildingAirLeakage, set from "infiltrationrateoccupied"
  double infiltration()
  {
    return _infiltration;
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

  // XXX: appears to be unused.
  void setInfiltration(double val)
  {
    _infiltration = val;
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


  // TODO: Add these new ism properties to the appropriate classes.
  // Schedule files.
  std::string scheduleFilePath() const {
    return _scheduleFilePath;
  }

  void setScheduleFilePath(const std::string& path) {
    _scheduleFilePath = path;
  }

  // Building.
  double electricAppliancePowerFixedOccupied() const  {
    return _electricAppliancePowerFixedOccupied;
  }

  void setElectricAppliancePowerFixedOccupied(double electricAppliancePowerFixedOccupied)
  {
    _electricAppliancePowerFixedOccupied = electricAppliancePowerFixedOccupied;
  }

  double electricAppliancePowerFixedUnoccupied() const  {
    return _electricAppliancePowerFixedUnoccupied;
  }

  void setElectricAppliancePowerFixedUnoccupied(double electricAppliancePowerFixedUnoccupied)
  {
    _electricAppliancePowerFixedUnoccupied = electricAppliancePowerFixedUnoccupied;
  }

  double gasAppliancePowerFixedOccupied() const  {
    return _gasAppliancePowerFixedOccupied;
  }

  void setGasAppliancePowerFixedOccupied(double gasAppliancePowerFixedOccupied)
  {
    _gasAppliancePowerFixedOccupied = gasAppliancePowerFixedOccupied;
  }

  double gasAppliancePowerFixedUnoccupied() const  {
    return _gasAppliancePowerFixedUnoccupied;
  }

  void setGasAppliancePowerFixedUnoccupied(double gasAppliancePowerFixedUnoccupied)
  {
    _gasAppliancePowerFixedUnoccupied = gasAppliancePowerFixedUnoccupied;
  }

  double infiltrationRateUnoccupied() const  {
    return _infiltrationRateUnoccupied;
  }

  void setInfiltrationRateUnoccupied(double infiltrationRateUnoccupied)
  {
    _infiltrationRateUnoccupied = infiltrationRateUnoccupied;
  }

  double lightingPowerFixedOccupied() const  {
    return _lightingPowerFixedOccupied;
  }

  void setLightingPowerFixedOccupied(double lightingPowerFixedOccupied)
  {
    _lightingPowerFixedOccupied = lightingPowerFixedOccupied;
  }

  double lightingPowerFixedUnoccupied() const  {
    return _lightingPowerFixedUnoccupied;
  }

  void setLightingPowerFixedUnoccupied(double lightingPowerFixedUnoccupied)
  {
    _lightingPowerFixedUnoccupied = lightingPowerFixedUnoccupied;
  }

  double ventilationExhaustRateUnoccupied() const  {
    return _ventilationExhaustRateUnoccupied;
  }

  void setVentilationExhaustRateUnoccupied(double ventilationExhaustRateUnoccupied)
  {
    _ventilationExhaustRateUnoccupied = ventilationExhaustRateUnoccupied;
  }

  double ventilationIntakeRateUnoccupied() const  {
    return _ventilationIntakeRateUnoccupied;
  }

  void setVentilationIntakeRateUnoccupied(double ventilationIntakeRateUnoccupied)
  {
    _ventilationIntakeRateUnoccupied = ventilationIntakeRateUnoccupied;
  }
};

} // isomodel
} // openstudio

#endif // ISOMODEL_USERMODEL_HPP
