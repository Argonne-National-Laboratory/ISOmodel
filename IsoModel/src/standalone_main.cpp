/*
 * standalone_main.cpp
 *
 *  Created on: Dec 5, 2014
 *      Author: nick
 */

#include "UserModel.hpp"
#include "SimModel.hpp"

using namespace openstudio::isomodel;
using namespace openstudio;

int main(int argc, char* argv[])
{
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <filename.ISO>" << std::endl;
    return -1;
  }
  if (DEBUG_ISO_MODEL_SIMULATION)
    std::cout << "Loading User Model..." << std::endl;
  openstudio::isomodel::UserModel umodel;
  umodel.load(argv[1]);
  if (DEBUG_ISO_MODEL_SIMULATION)
    std::cout << "User Model Loaded" << std::endl;

  if (DEBUG_ISO_MODEL_SIMULATION) {
    umodel.loadWeather();
    WeatherData wd = *umodel.weatherData();
    Matrix msolar = wd.msolar();
    Matrix mhdbt = wd.mhdbt();
    Matrix mhEgh = wd.mhEgh();
    Vector mEgh = wd.mEgh();
    Vector mdbt = wd.mdbt();
    Vector mwind = wd.mwind();
    printMatrix("msolar", msolar);
    printMatrix("mhdbt", mhdbt);
    printMatrix("mhEgh", mhEgh);
    printVector("mEgh", mEgh);
    printVector("mdbt", mdbt);
    printVector("mwind", mwind);
  }

  openstudio::isomodel::ISOHourly hourly = umodel.toHourlyModel();
  ISOResults hourlyResults = hourly.calculateHourly();
  std::cout << "Hourly simulation complete" << std::endl;

  openstudio::isomodel::SimModel simModel = umodel.toSimModel();
  ISOResults results = simModel.simulate();
  std::cout << "Monthly simulation complete" << std::endl;

  if (DEBUG_ISO_MODEL_SIMULATION)
    std::cout << std::endl;

  std::cout << "Monthly Results:" << std::endl;
  std::cout
      << "Month,ElecHeat,ElecCool,ElecIntLights,ElecExtLights,ElecFans,ElecPump,ElecEquipInt,ElecEquipExt,ElectDHW,GasHeat,GasCool,GasEquip,GasDHW"
      << std::endl;
  for (int i = 0; i < 12; i++) {
    std::cout << i + 1;
#ifdef _OPENSTUDIOS
    std::cout << ", " << results.monthlyResults[i].getEndUse(EndUseFuelType::Electricity, EndUseCategoryType::Heating);
    std::cout << ", " << results.monthlyResults[i].getEndUse(EndUseFuelType::Electricity, EndUseCategoryType::Cooling);
    std::cout << ", " << results.monthlyResults[i].getEndUse(EndUseFuelType::Electricity, EndUseCategoryType::InteriorLights);
    std::cout << ", " << results.monthlyResults[i].getEndUse(EndUseFuelType::Electricity, EndUseCategoryType::ExteriorLights);
    std::cout << ", " << results.monthlyResults[i].getEndUse(EndUseFuelType::Electricity, EndUseCategoryType::Fans);
    std::cout << ", " << results.monthlyResults[i].getEndUse(EndUseFuelType::Electricity, EndUseCategoryType::Pumps);
    std::cout << ", " << results.monthlyResults[i].getEndUse(EndUseFuelType::Electricity, EndUseCategoryType::InteriorEquipment);
    std::cout << ", " << results.monthlyResults[i].getEndUse(EndUseFuelType::Electricity, EndUseCategoryType::ExteriorEquipment);
    std::cout << ", " << results.monthlyResults[i].getEndUse(EndUseFuelType::Electricity, EndUseCategoryType::WaterSystems);

    std::cout << ", " << results.monthlyResults[i].getEndUse(EndUseFuelType::Gas, EndUseCategoryType::Heating);
    std::cout << ", " << results.monthlyResults[i].getEndUse(EndUseFuelType::Gas, EndUseCategoryType::Cooling);
    std::cout << ", " << results.monthlyResults[i].getEndUse(EndUseFuelType::Gas, EndUseCategoryType::InteriorEquipment);
    std::cout << ", " << results.monthlyResults[i].getEndUse(EndUseFuelType::Gas, EndUseCategoryType::WaterSystems);
#else
    for (int j = 0; j < 13; j++) {
      std::cout << ", " << results.monthlyResults[i].getEndUse(j);
    }
#endif
    std::cout << std::endl;
  }

  std::cout << "Hourly results by month:" << std::endl;
  std::cout
      << "Month,ElecHeat,ElecCool,ElecIntLights,ElecExtLights,ElecFans,ElecPump,ElecEquipInt,ElecEquipExt,ElectDHW,GasHeat,GasCool,GasEquip,GasDHW"
      << std::endl;
  for (int i = 0; i < 12; i++) {
    std::cout << i + 1;
#ifdef _OPENSTUDIOS
    std::cout << ", " << hourlyResults.hourlyResultsByMonth[i].getEndUse(EndUseFuelType::Electricity, EndUseCategoryType::Heating);
    std::cout << ", " << hourlyResults.hourlyResultsByMonth[i].getEndUse(EndUseFuelType::Electricity, EndUseCategoryType::Cooling);
    std::cout << ", " << hourlyResults.hourlyResultsByMonth[i].getEndUse(EndUseFuelType::Electricity, EndUseCategoryType::InteriorLights);
    std::cout << ", " << hourlyResults.hourlyResultsByMonth[i].getEndUse(EndUseFuelType::Electricity, EndUseCategoryType::ExteriorLights);
    std::cout << ", " << hourlyResults.hourlyResultsByMonth[i].getEndUse(EndUseFuelType::Electricity, EndUseCategoryType::Fans);
    std::cout << ", " << hourlyResults.hourlyResultsByMonth[i].getEndUse(EndUseFuelType::Electricity, EndUseCategoryType::Pumps);
    std::cout << ", " << hourlyResults.hourlyResultsByMonth[i].getEndUse(EndUseFuelType::Electricity, EndUseCategoryType::InteriorEquipment);
    std::cout << ", " << hourlyResults.hourlyResultsByMonth[i].getEndUse(EndUseFuelType::Electricity, EndUseCategoryType::ExteriorEquipment);
    std::cout << ", " << hourlyResults.hourlyResultsByMonth[i].getEndUse(EndUseFuelType::Electricity, EndUseCategoryType::WaterSystems);

    std::cout << ", " << hourlyResults.hourlyResultsByMonth[i].getEndUse(EndUseFuelType::Gas, EndUseCategoryType::Heating);
    std::cout << ", " << hourlyResults.hourlyResultsByMonth[i].getEndUse(EndUseFuelType::Gas, EndUseCategoryType::Cooling);
    std::cout << ", " << hourlyResults.hourlyResultsByMonth[i].getEndUse(EndUseFuelType::Gas, EndUseCategoryType::InteriorEquipment);
    std::cout << ", " << hourlyResults.hourlyResultsByMonth[i].getEndUse(EndUseFuelType::Gas, EndUseCategoryType::WaterSystems);
#else
    for (int j = 0; j < 13; j++) {
      std::cout << ", " << hourlyResults.hourlyResultsByMonth[i].getEndUse(j);
    }
#endif
    std::cout << std::endl;
  }
  //if(DEBUG_ISO_MODEL_SIMULATION)
  //  std::cin.get();
}


