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

#include <isomodel/UserModel.hpp>

using namespace std;
namespace openstudio {
namespace isomodel {

  UserModel::UserModel()
  {
  }

  UserModel::~UserModel()
  {
  }
  
  ISOHourly UserModel::toHourlyModel() const
  {
    ISOHourly sim = ISOHourly();
    if(!_valid){
      return *((ISOHourly*)NULL);
    }
    boost::shared_ptr<Population> pop(new Population);
    pop->setDaysStart(_buildingOccupancyFrom);
    pop->setDaysEnd(_buildingOccupancyTo);
    pop->setHoursEnd(_equivFullLoadOccupancyTo);
    pop->setHoursStart(_equivFullLoadOccupancyFrom);
    pop->setDensityOccupied(_peopleDensityOccupied);
    pop->setDensityUnoccupied(_peopleDensityUnoccupied);
    pop->setHeatGainPerPerson(_heatGainPerPerson);
    sim.setPop(pop);

    boost::shared_ptr<Building> building(new Building);
    building->setElectricApplianceHeatGainOccupied(_elecPowerAppliancesOccupied);
    building->setElectricApplianceHeatGainUnoccupied(_elecPowerAppliancesUnoccupied);
    building->setLightingOccupancySensor(_lightingOccupancySensorSystem);
    sim.setBuilding(building);

    boost::shared_ptr<Cooling> cooling(new Cooling);
    cooling->setCOP(_coolingSystemCOP);
    cooling->setHvacLossFactor(_hvacCoolingLossFactor);
    cooling->setTemperatureSetPointOccupied(_coolingOccupiedSetpoint);
    cooling->setTemperatureSetPointUnoccupied(_coolingUnoccupiedSetpoint);
    sim.setCooling(cooling);

    boost::shared_ptr<Heating> heating(new Heating);
    heating->setEfficiency(_heatingSystemEfficiency);
    heating->setHvacLossFactor(_hvacHeatingLossFactor);
	heating->setHotcoldWasteFactor(_hvacWasteFactor);
    heating->setTemperatureSetPointOccupied(_heatingOccupiedSetpoint);
    heating->setTemperatureSetPointUnoccupied(_heatingUnoccupiedSetpoint);
    sim.setHeating(heating);

    boost::shared_ptr<Lighting> lighting(new Lighting);
    lighting->setExteriorEnergy(_exteriorLightingPower);
    lighting->setPowerDensityOccupied(_lightingPowerIntensityOccupied);
    lighting->setPowerDensityUnoccupied(_lightingPowerIntensityUnoccupied);
    sim.setLights(lighting);

    boost::shared_ptr<Structure> structure(new Structure);
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
    structure->setWallArea(wallArea);//vector
    structure->setWallHeatCapacity(_exteriorHeatCapacity);//??

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
    structure->setWallSolarAbsorbtion(wallSolar);//vector

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
    structure->setWallThermalEmissivity(wallTherm);//vector

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
    structure->setWallUniform(wallU);//vector


    Vector windowArea(9);
    windowArea[0] = _windowAreaS ;
    windowArea[1] = _windowAreaSE;
    windowArea[2] = _windowAreaE ;
    windowArea[3] = _windowAreaNE;
    windowArea[4] = _windowAreaN ;
    windowArea[5] = _windowAreaNW;
    windowArea[6] = _windowAreaW ;
    windowArea[7] = _windowAreaSW;
    windowArea[8] = _skylightArea ;
    structure->setWindowArea(windowArea);//vector

    Vector winSHGC(9);
    winSHGC[0] = _windowSHGCS ;
    winSHGC[1] = _windowSHGCSE;
    winSHGC[2] = _windowSHGCE ;
    winSHGC[3] = _windowSHGCNE;
    winSHGC[4] = _windowSHGCN ;
    winSHGC[5] = _windowSHGCNW;
    winSHGC[6] = _windowSHGCW ;
    winSHGC[7] = _windowSHGCSW;
    winSHGC[8] = _skylightSHGC;
    structure->setWindowNormalIncidenceSolarEnergyTransmittance(winSHGC);//vector
    
    Vector winSCF(9);
    winSCF[0] = _windowSCFS ;
    winSCF[1] = _windowSCFSE;
    winSCF[2] = _windowSCFE ;
    winSCF[3] = _windowSCFNE;
    winSCF[4] = _windowSCFN ;
    winSCF[5] = _windowSCFNW;
    winSCF[6] = _windowSCFW ;
    winSCF[7] = _windowSCFSW;
    winSCF[8] = _windowSCFN;
    structure->setWindowShadingCorrectionFactor(winSCF);//vector
    structure->setWindowShadingDevice(_windowSDFN);

    Vector winU(9);
    winU[0] = _windowUvalueS ;
    winU[1] = _windowUvalueSE;
    winU[2] = _windowUvalueE ;
    winU[3] = _windowUvalueNE;
    winU[4] = _windowUvalueN ;
    winU[5] = _windowUvalueNW;
    winU[6] = _windowUvalueW ;
    winU[7] = _windowUvalueSW;
    winU[8] = _skylightUvalue;
    structure->setWindowUniform(winU);//vector
    sim.setStructure(structure);

    boost::shared_ptr<Ventilation> ventilation(new Ventilation);
    ventilation->setExhaustAirRecirculated(_exhaustAirRecirclation);
    ventilation->setFanControlFactor(_fanFlowControlFactor);
    ventilation->setFanPower(_specificFanPower);
    ventilation->setHeatRecoveryEfficiency(_heatRecovery);
    ventilation->setSupplyDifference(_supplyExhaustRate);
    ventilation->setSupplyRate(_freshAirFlowRate);
    sim.setVentilation(ventilation);

    boost::shared_ptr<EpwData> wdata((EpwData*)&this->_edata);
    sim.setWeatherData(wdata);

    return sim;
  }
  SimModel UserModel::toSimModel() const
  {
    SimModel sim = SimModel();
    if(!_valid){
      return *((SimModel*)NULL);
    }

    boost::shared_ptr<Population> pop(new Population);
    pop->setDaysStart(_buildingOccupancyFrom);
    pop->setDaysEnd(_buildingOccupancyTo);
    pop->setHoursEnd(_equivFullLoadOccupancyTo);
    pop->setHoursStart(_equivFullLoadOccupancyFrom);
    pop->setDensityOccupied(_peopleDensityOccupied);
    pop->setDensityUnoccupied(_peopleDensityUnoccupied);
    pop->setHeatGainPerPerson(_heatGainPerPerson);
    sim.setPop(pop);

    boost::shared_ptr<Location> loc(new Location);
    loc->setTerrain(_terrainClass);
    loc->setWeatherData(_weather);
    sim.setLocation(loc);

    boost::shared_ptr<Building> building(new Building);
    building->setBuildingEnergyManagement(_bemType);
    building->setConstantIllumination(_constantIlluminationControl);
    building->setElectricApplianceHeatGainOccupied(_elecPowerAppliancesOccupied);
    building->setElectricApplianceHeatGainUnoccupied(_elecPowerAppliancesUnoccupied);
    building->setGasApplianceHeatGainOccupied(_gasPowerAppliancesOccupied);
    building->setGasApplianceHeatGainUnoccupied(_gasPowerAppliancesUnoccupied);
    building->setLightingOccupancySensor(_lightingOccupancySensorSystem);
    sim.setBuilding(building);

    boost::shared_ptr<Cooling> cooling(new Cooling);
    cooling->setCOP(_coolingSystemCOP);
    cooling->setHvacLossFactor(_hvacCoolingLossFactor);
    cooling->setPartialLoadValue(_coolingSystemIPLVToCOPRatio);
    cooling->setPumpControlReduction(_coolingPumpControl);
    cooling->setTemperatureSetPointOccupied(_coolingOccupiedSetpoint);
    cooling->setTemperatureSetPointUnoccupied(_coolingUnoccupiedSetpoint);
    sim.setCooling(cooling);
    
    boost::shared_ptr<Heating> heating(new Heating);
    heating->setEfficiency(_heatingSystemEfficiency);
    heating->setEnergyType(_heatingEnergyCarrier);
    heating->setHotcoldWasteFactor(_hvacWasteFactor); // Used in hvac distribution efficiency.
    heating->setHotWaterDemand(_dhwDemand);
    heating->setHotWaterDistributionEfficiency(_dhwDistributionEfficiency);
    heating->setHotWaterEnergyType(_dhwEnergyCarrier);
    heating->setHotWaterSystemEfficiency(_dhwEfficiency);
    heating->setHvacLossFactor(_hvacHeatingLossFactor);
    heating->setPumpControlReduction(_heatingPumpControl);
    heating->setTemperatureSetPointOccupied(_heatingOccupiedSetpoint);
    heating->setTemperatureSetPointUnoccupied(_heatingUnoccupiedSetpoint);
    sim.setHeating(heating);

    boost::shared_ptr<Lighting> lighting(new Lighting);
    lighting->setDimmingFraction(_daylightSensorSystem);
    lighting->setExteriorEnergy(_exteriorLightingPower);
    lighting->setPowerDensityOccupied(_lightingPowerIntensityOccupied);
    lighting->setPowerDensityUnoccupied(_lightingPowerIntensityUnoccupied);
    sim.setLights(lighting);

    boost::shared_ptr<Structure> structure(new Structure);
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
    structure->setWallArea(wallArea);//vector
    structure->setWallHeatCapacity(_exteriorHeatCapacity);//??

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
    structure->setWallSolarAbsorbtion(wallSolar);//vector

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
    structure->setWallThermalEmissivity(wallTherm);//vector

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
    structure->setWallUniform(wallU);//vector


    Vector windowArea(9);
    windowArea[0] = _windowAreaS ;
    windowArea[1] = _windowAreaSE;
    windowArea[2] = _windowAreaE ;
    windowArea[3] = _windowAreaNE;
    windowArea[4] = _windowAreaN ;
    windowArea[5] = _windowAreaNW;
    windowArea[6] = _windowAreaW ;
    windowArea[7] = _windowAreaSW;
    windowArea[8] = _skylightArea ;
    structure->setWindowArea(windowArea);//vector

    Vector winSHGC(9);
    winSHGC[0] = _windowSHGCS ;
    winSHGC[1] = _windowSHGCSE;
    winSHGC[2] = _windowSHGCE ;
    winSHGC[3] = _windowSHGCNE;
    winSHGC[4] = _windowSHGCN ;
    winSHGC[5] = _windowSHGCNW;
    winSHGC[6] = _windowSHGCW ;
    winSHGC[7] = _windowSHGCSW;
    winSHGC[8] = _skylightSHGC;
    structure->setWindowNormalIncidenceSolarEnergyTransmittance(winSHGC);//vector
    
    Vector winSCF(9);
    winSCF[0] = _windowSCFS ;
    winSCF[1] = _windowSCFSE;
    winSCF[2] = _windowSCFE ;
    winSCF[3] = _windowSCFNE;
    winSCF[4] = _windowSCFN ;
    winSCF[5] = _windowSCFNW;
    winSCF[6] = _windowSCFW ;
    winSCF[7] = _windowSCFSW;
    winSCF[8] = _windowSCFN;
    structure->setWindowShadingCorrectionFactor(winSCF);//vector
    structure->setWindowShadingDevice(_windowSDFN);

    Vector winU(9);
    winU[0] = _windowUvalueS ;
    winU[1] = _windowUvalueSE;
    winU[2] = _windowUvalueE ;
    winU[3] = _windowUvalueNE;
    winU[4] = _windowUvalueN ;
    winU[5] = _windowUvalueNW;
    winU[6] = _windowUvalueW ;
    winU[7] = _windowUvalueSW;
    winU[8] = _skylightUvalue;
    structure->setWindowUniform(winU);//vector
    sim.setStructure(structure);

    boost::shared_ptr<Ventilation> ventilation(new Ventilation);
    ventilation->setExhaustAirRecirculated(_exhaustAirRecirclation);
    ventilation->setFanControlFactor(_fanFlowControlFactor);
    ventilation->setFanPower(_specificFanPower);
    ventilation->setHeatRecoveryEfficiency(_heatRecovery);
    ventilation->setSupplyDifference(_supplyExhaustRate);
    ventilation->setSupplyRate(_freshAirFlowRate);
    ventilation->setType(_ventilationType);
    ventilation->setWasteFactor(_hvacWasteFactor);//??
    sim.setVentilation(ventilation);
    return sim;
  }
  //http://stackoverflow.com/questions/10051679/c-tokenize-string
  std::vector<std::string> inline stringSplit(const std::string &source, char delimiter = ' ', bool keepEmpty = false)
  {
    std::vector<std::string> results;

    size_t prev = 0;
    size_t next = 0;
    if(source.size()==0)
      return results;
    while ((next = source.find_first_of(delimiter, prev)) != std::string::npos)
    {
        if (keepEmpty || (next - prev != 0))
        {
            results.push_back(source.substr(prev, next - prev));
        }
        prev = next + 1;
    }

    if (prev < source.size())
    {
        results.push_back(source.substr(prev));
    }

    return results;
  }

  // trim from front and back ends
  static inline std::string trim(std::string &s) {
    const char* cstr = s.c_str();
    int start=0,end=s.size();
    while(*cstr == ' ')
    {
      cstr++;
      start++;
    }
    cstr += (end-start-1);
    while(*cstr == ' ')
    {
      cstr--;
      end--;
    }
    return s.substr(start,end-start);
  }
    // trim from front and back ends
  static inline std::string lcase(std::string &s) {
    for(unsigned int i = 0;i<s.size();i++){
      if(s.at(i) < 91){
        s.at(i) = s.at(i)+32;
      }
    }
    return s;
  }

  void UserModel::parseStructure(std::string attributeName, const char* attributeValue){
    //Window&Wall Values    
      if(!attributeName.compare("windowuvaluen")){
        _windowUvalueN = (atof(attributeValue));
      } else if(!attributeName.compare("windowshgcn")){
        _windowSHGCN = (atof(attributeValue));
      } else if(!attributeName.compare("windowscfn")){
        _windowSCFN = (atof(attributeValue));
      } else if(!attributeName.compare("windowsdfn")){
        _windowSDFN = (atof(attributeValue));
      } else if(!attributeName.compare("walluvaluen")){
        _wallUvalueN = (atof(attributeValue));
      } else if(!attributeName.compare("wallsolarabsorptionn")){
        _wallSolarAbsorptionN = (atof(attributeValue));
      } else if(!attributeName.compare("wallthermalemissivityn")){
        _wallThermalEmissivityN = (atof(attributeValue));
      }
    
      else if(!attributeName.compare("windowuvaluene")){
        _windowUvalueNE = (atof(attributeValue));
      } else if(!attributeName.compare("windowshgcne")){
        _windowSHGCNE = (atof(attributeValue));
      } else if(!attributeName.compare("windowscfne")){
        _windowSCFNE = (atof(attributeValue));
      } else if(!attributeName.compare("windowsdfne")){
        _windowSDFNE = (atof(attributeValue));
      } else if(!attributeName.compare("walluvaluene")){
        _wallUvalueNE = (atof(attributeValue));
      } else if(!attributeName.compare("wallsolarabsorptionne")){
        _wallSolarAbsorptionNE = (atof(attributeValue));
      } else if(!attributeName.compare("wallthermalemissivityne")){
        _wallThermalEmissivityNE = (atof(attributeValue));
      } 
    
      else if(!attributeName.compare("windowuvaluee")){
        _windowUvalueE = (atof(attributeValue));
      } else if(!attributeName.compare("windowshgce")){
        _windowSHGCE = (atof(attributeValue));
      } else if(!attributeName.compare("windowscfe")){
        _windowSCFE = (atof(attributeValue));
      } else if(!attributeName.compare("windowsdfe")){
        _windowSDFE = (atof(attributeValue));
      } else if(!attributeName.compare("walluvaluee")){
        _wallUvalueE = (atof(attributeValue));
      } else if(!attributeName.compare("wallsolarabsorptione")){
        _wallSolarAbsorptionE = (atof(attributeValue));
      } else if(!attributeName.compare("wallthermalemissivitye")){
        _wallThermalEmissivityE = (atof(attributeValue));
      } 
    
      else if(!attributeName.compare("windowuvaluese")){
        _windowUvalueSE =  (atof(attributeValue));
      } else if(!attributeName.compare("windowshgcse")){
        _windowSHGCSE =  (atof(attributeValue));
      } else if(!attributeName.compare("windowscfse")){
        _windowSCFSE =  (atof(attributeValue));
      } else if(!attributeName.compare("windowsdfse")){
        _windowSDFSE =  (atof(attributeValue));
      } else if(!attributeName.compare("walluvaluese")){
        _wallUvalueSE =  (atof(attributeValue));
      } else if(!attributeName.compare("wallsolarabsorptionse")){
        _wallSolarAbsorptionSE =  (atof(attributeValue));
      } else if(!attributeName.compare("wallthermalemissivityse")){
        _wallThermalEmissivitySE =  (atof(attributeValue));
      } 
    
      else if(!attributeName.compare("windowuvalues")){
        _windowUvalueS =  (atof(attributeValue));
      } else if(!attributeName.compare("windowshgcs")){
        _windowSHGCS =  (atof(attributeValue));
      } else if(!attributeName.compare("windowscfs")){
        _windowSCFS =  (atof(attributeValue));
      } else if(!attributeName.compare("windowsdfs")){
        _windowSDFS =  (atof(attributeValue));
      } else if(!attributeName.compare("walluvalues")){
        _wallUvalueS =  (atof(attributeValue));
      } else if(!attributeName.compare("wallsolarabsorptions")){
        _wallSolarAbsorptionS =  (atof(attributeValue));
      } else if(!attributeName.compare("wallthermalemissivitys")){
        _wallThermalEmissivityS =  (atof(attributeValue));
      } 

      else if(!attributeName.compare("windowuvaluesw")){
        _windowUvalueSW =  (atof(attributeValue));
      } else if(!attributeName.compare("windowshgcsw")){
        _windowSHGCSW =  (atof(attributeValue));
      } else if(!attributeName.compare("windowscfsw")){
        _windowSCFSW =  (atof(attributeValue));
      } else if(!attributeName.compare("windowsdfsw")){
        _windowSDFSW =  (atof(attributeValue));
      } else if(!attributeName.compare("walluvaluesw")){
        _wallUvalueSW =  (atof(attributeValue));
      } else if(!attributeName.compare("wallsolarabsorptionsw")){
        _wallSolarAbsorptionSW =  (atof(attributeValue));
      } else if(!attributeName.compare("wallthermalemissivitysw")){
        _wallThermalEmissivitySW =  (atof(attributeValue));
      } 
    
      else if(!attributeName.compare("windowuvaluew")){
        _windowUvalueW =  (atof(attributeValue));
      } else if(!attributeName.compare("windowshgcw")){
        _windowSHGCW =  (atof(attributeValue));
      } else if(!attributeName.compare("windowscfw")){
        _windowSCFW =  (atof(attributeValue));
      } else if(!attributeName.compare("windowsdfw")){
        _windowSDFW =  (atof(attributeValue));
      } else if(!attributeName.compare("walluvaluew")){
        _wallUvalueW =  (atof(attributeValue));
      } else if(!attributeName.compare("wallsolarabsorptionw")){
        _wallSolarAbsorptionW =  (atof(attributeValue));
      } else if(!attributeName.compare("wallthermalemissivityw")){
        _wallThermalEmissivityW =  (atof(attributeValue));
      } 

      else if(!attributeName.compare("windowuvaluenw")){
        _windowUvalueNW =  (atof(attributeValue));
      } else if(!attributeName.compare("windowshgcnw")){
        _windowSHGCNW =  (atof(attributeValue));
      } else if(!attributeName.compare("windowscfnw")){
        _windowSCFNW =  (atof(attributeValue));
      } else if(!attributeName.compare("windowsdfnw")){
        _windowSDFNW =  (atof(attributeValue));
      } else if(!attributeName.compare("walluvaluenw")){
        _wallUvalueNW =  (atof(attributeValue));
      } else if(!attributeName.compare("wallsolarabsorptionnw")){
        _wallSolarAbsorptionNW =  (atof(attributeValue));
      } else if(!attributeName.compare("wallthermalemissivitynw")){
        _wallThermalEmissivityNW =  (atof(attributeValue));
      } 
  }
  void UserModel::parseLine(string line){
    std::vector<std::string> linesplit = stringSplit(line, '=', true);
    if(linesplit.size()<2)
      return;
    for(unsigned int i = 0;i<linesplit.size();i++) {
      linesplit[i] = trim(linesplit[i]);
    }
    if(linesplit[0].at(0) == '#')
      return;
    string attributeName = lcase(linesplit[0]);
    
		// XXX BAA@20140730: atof() returns 0.0 when no valid conversion can be
		// performed. This seems like it makes it impossible to differentiate
		// between corrupted input data and intentional values of 0.0. Is this a
		// problem?
    const char* attributeValue = linesplit[1].c_str();
    if(!attributeName.compare("terrainclass")){
      setTerrainClass(atof(attributeValue));
    } else if(!attributeName.compare("buildingheight")){
      setBuildingHeight(atof(attributeValue));
    } else if(!attributeName.compare("floorarea")){
      setFloorArea(atof(attributeValue));
    } else if(!attributeName.compare("buildingoccupancyfrom")){
      setBuildingOccupancyFrom(atof(attributeValue));
    } else if(!attributeName.compare("buildingoccupancyto")){
      setBuildingOccupancyTo(atof(attributeValue));
    } else if(!attributeName.compare("equivfullloadoccupancyfrom")){
      setEquivFullLoadOccupancyFrom(atof(attributeValue));
    } else if(!attributeName.compare("equivfullloadoccupancyto")){
      setEquivFullLoadOccupancyTo(atof(attributeValue));
    } else if(!attributeName.compare("peopledensityoccupied")){
      setPeopleDensityOccupied(atof(attributeValue));
    } else if(!attributeName.compare("peopledensityunoccupied")){
      setPeopleDensityUnoccupied(atof(attributeValue));
    } else if(!attributeName.compare("lightingpowerintensityoccupied")){
      setLightingPowerIntensityOccupied(atof(attributeValue));
    } else if(!attributeName.compare("lightingpowerintensityunoccupied")){
      setLightingPowerIntensityUnoccupied(atof(attributeValue));
    } else if(!attributeName.compare("elecpowerappliancesoccupied")){
      setElecPowerAppliancesOccupied(atof(attributeValue));
    } else if(!attributeName.compare("elecpowerappliancesunoccupied")){
      setElecPowerAppliancesUnoccupied(atof(attributeValue));
    } else if(!attributeName.compare("gaspowerappliancesoccupied")){
      setGasPowerAppliancesOccupied(atof(attributeValue));
    } else if(!attributeName.compare("gaspowerappliancesunoccupied")){
      setGasPowerAppliancesUnoccupied(atof(attributeValue));
    } else if(!attributeName.compare("exteriorlightingpower")){
      setExteriorLightingPower(atof(attributeValue));
    } else if(!attributeName.compare("hvacwastefactor")){
      setHvacWasteFactor(atof(attributeValue));
    } else if(!attributeName.compare("hvacheatinglossfactor")){
      setHvacHeatingLossFactor(atof(attributeValue));
    } else if(!attributeName.compare("hvaccoolinglossfactor")){
      setHvacCoolingLossFactor(atof(attributeValue));
    } else if(!attributeName.compare("daylightsensorsystem")){
      setDaylightSensorSystem(atof(attributeValue));
    } else if(!attributeName.compare("lightingoccupancysensorsystem")){
      setLightingOccupancySensorSystem(atof(attributeValue));
    } else if(!attributeName.compare("constantilluminationcontrol")){//constantilluminaitoncontrol
      setConstantIlluminationControl(atof(attributeValue));
    } else if(!attributeName.compare("coolingsystemcop")){
      setCoolingSystemCOP(atof(attributeValue));
    } else if(!attributeName.compare("coolingsystemiplvtocopratio")){
      setCoolingSystemIPLVToCOPRatio(atof(attributeValue));
    } else if(!attributeName.compare("heatingenergycarrier")){
      setHeatingEnergyCarrier(atof(attributeValue));
    } else if(!attributeName.compare("heatingsystemefficiency")){
      setHeatingSystemEfficiency(atof(attributeValue));
    } else if(!attributeName.compare("ventilationtype")){
      setVentilationType(atof(attributeValue));
    } else if(!attributeName.compare("freshairflowrate")){
      setFreshAirFlowRate(atof(attributeValue));
    } else if(!attributeName.compare("supplyexhaustrate")){
      setSupplyExhaustRate(atof(attributeValue));
    } else if(!attributeName.compare("heatrecovery")){
      setHeatRecovery(atof(attributeValue));
    } else if(!attributeName.compare("exhaustairrecirculation")){
      setExhaustAirRecirclation(atof(attributeValue));
    } else if(!attributeName.compare("infiltration")){
      setBuildingAirLeakage(atof(attributeValue));
    } else if(!attributeName.compare("dhwdemand")){
      setDhwDemand(atof(attributeValue));
    } else if(!attributeName.compare("dhwsystemefficiency")){
      setDhwEfficiency(atof(attributeValue));
    } else if(!attributeName.compare("dhwdistributionefficiency")){
      setDhwDistributionEfficiency(atof(attributeValue));
    } else if(!attributeName.compare("dhwenergycarrier")){
      setDhwEnergyCarrier(atof(attributeValue));
    } else if(!attributeName.compare("bemtype")){
      setBemType(atof(attributeValue));
    } else if(!attributeName.compare("interiorheatcapacity")){
      setInteriorHeatCapacity(atof(attributeValue));
    } else if(!attributeName.compare("exteriorheatcapacity")){
      setExteriorHeatCapacity(atof(attributeValue));
    } else if(!attributeName.compare("heatingpumpcontrol")){
      setHeatingPumpControl(atof(attributeValue));
    } else if(!attributeName.compare("coolingpumpcontrol")){
      setCoolingPumpControl(atof(attributeValue));
    } else if(!attributeName.compare("heatgainperperson")){
      setHeatGainPerPerson(atof(attributeValue));
                                    //specificFanPower
    } else if(!attributeName.compare("specificfanpower")){
      setSpecificFanPower(atof(attributeValue));
    } else if(!attributeName.compare("fanflowcontrolfactor")){
      setFanFlowControlFactor(atof(attributeValue));
    } else if(!attributeName.compare("roofuvalue")){
      setRoofUValue(atof(attributeValue));
    } else if(!attributeName.compare("roofsolarabsorption")){
      setRoofSolarAbsorption(atof(attributeValue));
    } else if(!attributeName.compare("roofthermalemissivity")){
      setRoofThermalEmissivity(atof(attributeValue));
    } else if(!attributeName.compare("skylightuvalue")){
      setSkylightUvalue(atof(attributeValue));
    } else if(!attributeName.compare("skylightshgc")){
      setSkylightSHGC(atof(attributeValue));
    } else if(!attributeName.compare("wallareas")){
      setWallAreaS(atof(attributeValue));
    } else if(!attributeName.compare("wallarease")){
      setWallAreaSE(atof(attributeValue));
    } else if(!attributeName.compare("wallareae")){
      setWallAreaE(atof(attributeValue));
    } else if(!attributeName.compare("wallareane")){
      setWallAreaNE(atof(attributeValue));
    } else if(!attributeName.compare("wallarean")){
      setWallAreaN(atof(attributeValue));
    } else if(!attributeName.compare("wallareanw")){
      setWallAreaNW(atof(attributeValue));
    } else if(!attributeName.compare("wallareaw")){
      setWallAreaW(atof(attributeValue));
    } else if(!attributeName.compare("wallareasw")){
      setWallAreaSW(atof(attributeValue));
    } else if(!attributeName.compare("roofarea")){
      setRoofArea(atof(attributeValue));
    } else if(!attributeName.compare("windowareas")){
      setWindowAreaS(atof(attributeValue));
    } else if(!attributeName.compare("windowarease")){
      setWindowAreaSE(atof(attributeValue));
    } else if(!attributeName.compare("windowareae")){
      setWindowAreaE(atof(attributeValue));
    } else if(!attributeName.compare("windowareane")){
      setWindowAreaNE(atof(attributeValue));
    } else if(!attributeName.compare("windowarean")){
      setWindowAreaN(atof(attributeValue));
    } else if(!attributeName.compare("windowareanw")){
      setWindowAreaNW(atof(attributeValue));
    } else if(!attributeName.compare("windowareaw")){
      setWindowAreaW(atof(attributeValue));
    } else if(!attributeName.compare("windowareasw")){
      setWindowAreaSW(atof(attributeValue));
    } else if(!attributeName.compare("skylightarea")){
      setSkylightArea(atof(attributeValue));
    } else if(!attributeName.compare("coolingoccupiedsetpoint")){
      setCoolingOccupiedSetpoint(atof(attributeValue));
    } else if(!attributeName.compare("coolingunoccupiedsetpoint")){
      setCoolingUnoccupiedSetpoint(atof(attributeValue));
    } else if(!attributeName.compare("heatingoccupiedsetpoint")){
      setHeatingOccupiedSetpoint(atof(attributeValue));
    } else if(!attributeName.compare("heatingunoccupiedsetpoint")){//weatherFilePath
      setHeatingUnoccupiedSetpoint(atof(attributeValue));
    } else if(!attributeName.compare("weatherfilepath")){//weatherFilePath
      setWeatherFilePath(linesplit[1]);
    } else if(boost::starts_with(attributeName.c_str(),"window") || 
              boost::starts_with(attributeName.c_str(),"wall") ) {
      parseStructure(attributeName,attributeValue);//avoid max nested ifs.  Might be better to change to a map eventually
    }    
    else {
      cout << "Unknown Attribute: "<< attributeName << " = " << attributeValue <<endl;
    }    
  }
  void UserModel::loadBuilding(std::string buildingFile){
    string line;
    ifstream inputFile (buildingFile.c_str());
    if (inputFile.is_open()) {
      while (inputFile.good()) {
        getline (inputFile,line);
        if(line.size() > 0 && line[0] == '#')
          continue;
        parseLine(line);
      }
      inputFile.close();
    }
    else {
      cout << "Unable to open file"; 
    }
  }
    int UserModel::weatherState(std::string header){
      if(!header.compare("solar"))
        return 1;
      else if(!header.compare("hdbt"))
        return 2;
      else if(!header.compare("hEgh"))
        return 3;
      else if(!header.compare("mEgh"))
        return 4;
      else if(!header.compare("mdbt"))
        return 5;
      else if(!header.compare("mwind"))
        return 6;
      else 
        return -1;
    }
  std::string UserModel::resolveFilename(std::string baseFile, std::string relativeFile){
    unsigned int lastSeparator = 0;
    unsigned int i = 0;
    const char separatorChar = '/';
    const char winSeparatorChar = '\\';
    std::string result;
    for(;i<baseFile.length();i++)
    {
      result += (baseFile[i] == winSeparatorChar) ? separatorChar : baseFile[i];
      if(result[i] == separatorChar)
      {
        lastSeparator = i;
      }
    }
    result = result.substr(0, lastSeparator+1);
    unsigned int j=0;
    if(relativeFile.length() > 0){
      //if first char is a separator, skip it
      if(relativeFile[0] == separatorChar || relativeFile[0] == winSeparatorChar)
        j++;  
    }
    for(;j<relativeFile.length();j++,i++)
    {
      result += (relativeFile[j] == winSeparatorChar) ? separatorChar : relativeFile[j];
    }
    return result;
  }
  boost::shared_ptr<WeatherData> UserModel::loadWeather(){
    boost::shared_ptr<WeatherData> wdata(new WeatherData);
    std::vector<std::string> linesplit;
    std::string weatherFilename;
    //see if weather file path is absolute path
    //if so, use it, else assemble relative path
    if(boost::filesystem::exists( _weatherFilePath ))
    {
      weatherFilename = _weatherFilePath;      
    }
    else
    {
      weatherFilename = resolveFilename(this->dataFile,_weatherFilePath);
      if ( !boost::filesystem::exists( weatherFilename ) )
      {
        std::cout << "Weather File Not Found: " << _weatherFilePath << std::endl;
        _valid = false;
        return wdata;//using shared_ptr doesn't compile with any attempt at returning null
      }
    }
    string line;
    _edata.loadData(weatherFilename);

    int state = 0, row=0;
    Matrix _msolar(12,8);
    Matrix _mhdbt(12,24);
    Matrix _mhEgh(12,24);
    Vector _mEgh(12);
    Vector _mdbt(12);
    Vector _mwind(12);
    
    std::stringstream inputFile(_edata.toISOData());

    while (inputFile.good()) {
      getline (inputFile,line);
      if(line.size() > 0 && line[0] == '#')
        continue;
      linesplit = stringSplit(line, ',', true);
      if(linesplit.size() == 0) {
        continue;
      } else if(linesplit.size() == 1) {
        state = weatherState(linesplit[0]);
        row=0;
      } else if(row<12) {
        switch(state){
          case 1://solar = [12 x 8] mean monthly total solar radiation (W/m2) on a vertical surface for each of the 8 cardinal directions
            for(unsigned int c = 1;c<linesplit.size() && c<9;c++) {
              _msolar(row,c-1) = atof(linesplit[c].c_str());
            }
            break;
          case 2://hdbt = [12 x 24] mean monthly dry bulb temp for each of the 24 hours of the day (C)
            for(unsigned int c = 1;c<linesplit.size() && c<25;c++) {
              _mhdbt(row,c-1) = atof(linesplit[c].c_str());
            }
            break;
          case 3://hEgh =[12 x 24] mean monthly Global Horizontal Radiation for each of the 24 hours of the day (W/m2)
            for(unsigned int c = 1;c<linesplit.size() && c<25;c++) {
              _mhEgh(row,c-1) = atof(linesplit[c].c_str());
            }
            break;
          case 4://megh = [12 x 1] mean monthly Global Horizontal Radiation (W/m2)
            _mEgh[row] = atof(linesplit[1].c_str());
            break;
          case 5://mdbt = [12 x 1] mean monthly dry bulb temp (C)
            _mdbt[row] = atof(linesplit[1].c_str());
            break;
          case 6://mwind = [12 x 1] mean monthly wind speed; (m/s) 
            _mwind[row] = atof(linesplit[1].c_str());
            break;
          default:
            break;
        }
        row++;
      }
    }
    wdata->setMdbt(_mdbt);
    wdata->setMEgh(_mEgh);
    wdata->setMhdbt(_mhdbt);
    wdata->setMhEgh(_mhEgh);
    wdata->setMsolar(_msolar);
    wdata->setMwind(_mwind);
    
    return wdata;
  }
  void UserModel::load(std::string buildingFile){
    this->dataFile = buildingFile;
    _valid = true;
    if ( !boost::filesystem::exists( buildingFile ) )
    {
      std::cout << "ISO Model File Not Found: " << buildingFile << std::endl;
      _valid = false;
      return;
    }
    if(DEBUG_ISO_MODEL_SIMULATION)
      std::cout << "Loading Building File: " << buildingFile <<std::endl;
    loadBuilding(buildingFile);
    if(DEBUG_ISO_MODEL_SIMULATION)
      std::cout << "Loading Weather File: " << this->weatherFilePath() <<std::endl;
    _weather = loadWeather();
    if(DEBUG_ISO_MODEL_SIMULATION)
      std::cout << "Weather File Loaded" <<std::endl;
  }

} // isomodel
} // openstudio

