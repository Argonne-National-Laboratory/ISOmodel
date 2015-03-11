/*
 * solar_debug.cpp
 *
 *  Created on: 2015-03-11
 *      Author: Brendan Albano
 */

#include "../UserModel.hpp"
#include "../SimModel.hpp"
#include <iostream>

#include <boost/program_options.hpp>

using namespace openstudio::isomodel;
using namespace openstudio;

void printMonthlySolar(UserModel umodel) {
    umodel.loadWeather();
    WeatherData wd = *umodel.weatherData();
    Matrix msolar = wd.msolar();
    Matrix mhEgh = wd.mhEgh();
    Vector mEgh = wd.mEgh();
    printMatrix("msolar", msolar);
    printMatrix("mhEgh", mhEgh);
    printVector("mEgh", mEgh);
    std::cout << std::endl;
}

void printHourlySolar(UserModel umodel) {
  umodel.loadWeather(); // Added a function UserModel::epwData() to return the _edata attribute.

  // Solar radiation code pulled from IsoHourly::calculatHourly
  // This has to be copied here for testing because it's all rolled into the calculate hourly function.
  TimeFrame frame;
  SolarRadiation pos(&frame, umodel.epwData().get());
  pos.Calculate();
  std::vector<std::vector<double> > radiation = pos.eglobe(); // Radiation for 8 directions (N, NE, E, etc.).

  // Add the roof radiation (9th direction). EGH is global horizontal radiation.
  for (auto i = 0; i != radiation.size(); ++i) {
    radiation[i].push_back(umodel.epwData()->data()[EGH][i]);
  }
  std::cout << "Hour, N, NE, E, SE, S, SW, W, NW, Horizontal" << std::endl;

  // Print out the radiation.
  auto hour = 0;

  for (auto hourRadiation : radiation) {
    std::cout << hour << ", ";
    for (auto directionRadiation : hourRadiation) {
      std::cout << directionRadiation << ", ";
    }
    std::cout << std::endl;
    ++hour;
  }
}

int main(int argc, char* argv[])
{
  namespace po = boost::program_options; 
  po::options_description desc("Options"); 
  desc.add_options()
    ("ismfilepath,i", po::value<std::string>()->required(), "Path to .ism file (positional argument).")
    ("monthly,m", "Print the monthly solar radiation values.")
    ("hourly,h", "Print the hourly solar radiation values.");

  po::positional_options_description positionalOptions; 
  positionalOptions.add("ismfilepath", 1); 

  po::variables_map vm;

  try {
    po::store(po::command_line_parser(argc, argv).options(desc).positional(positionalOptions).run(), vm); // Throws on error. 
    po::notify(vm); // Throws if required options are mising.
  }
  catch(boost::program_options::required_option& e) 
  { 
    std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
    std::cerr << desc << std::endl;
    return 1; 
  } 
  catch(boost::program_options::error& e) 
  { 
    std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
    std::cerr << desc << std::endl;
    return 1; 
  } 

  if (DEBUG_ISO_MODEL_SIMULATION) {
    std::cout << "Loading User Model..." << std::endl;
  }

  // Load the .ism file.
  openstudio::isomodel::UserModel umodel;
  umodel.load(vm["ismfilepath"].as<std::string>());

  bool simulationRan = false;

  if (vm.count("monthly")) {
    // Print monthly radiation
    printMonthlySolar(umodel);
    simulationRan = true;
  }

  if (vm.count("hourly")) {
    // Print hourly radiation
    printHourlySolar(umodel);
    simulationRan = true;
  }

  if (!simulationRan) {
    // Monthly radiation is default and will print if no other simulations did.
    // Print monthly radiation
    printMonthlySolar(umodel);
  }

}


