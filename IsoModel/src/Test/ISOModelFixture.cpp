// First Commit: 2015-09-20
//
// Authors:
// - Brendan Albano
// - Ralph Muehleisen
//
// Summary:
// Implements the `ISOModelFixture` class, providing common setup and
// teardown logic for the Google Test suite. This includes initializing
// test data paths, end-use name vectors, and configuring logging for the
// tests.

#include "ISOModelFixture.hpp"

#ifndef ISOMODEL_STANDALONE
#include <resources.hxx>
#endif

void ISOModelFixture::SetUp() {
  endUseNames = {"ElecHeat", "ElecCool", "ElecIntLights", "ElecExtLights",
                 "ElecFans", "ElecPump", "ElecEquipInt",  "ElecEquipExt",
                 "ElectDHW", "GasHeat",  "GasCool",       "GasEquip",
                 "GasDHW"};

#ifdef ISOMODEL_STANDALONE
  test_data_path = "test_data";
#else
  test_data_path = resourcesPath().string() + "/isomodel";

  isoResultsEndUseTypes = {{openstudio::EndUseFuelType::Electricity,
                            openstudio::EndUseCategoryType::Heating},
                           {openstudio::EndUseFuelType::Electricity,
                            openstudio::EndUseCategoryType::Cooling},
                           {openstudio::EndUseFuelType::Electricity,
                            openstudio::EndUseCategoryType::InteriorLights},
                           {openstudio::EndUseFuelType::Electricity,
                            openstudio::EndUseCategoryType::ExteriorLights},
                           {openstudio::EndUseFuelType::Electricity,
                            openstudio::EndUseCategoryType::Fans},
                           {openstudio::EndUseFuelType::Electricity,
                            openstudio::EndUseCategoryType::Pumps},
                           {openstudio::EndUseFuelType::Electricity,
                            openstudio::EndUseCategoryType::InteriorEquipment},
                           {openstudio::EndUseFuelType::Electricity,
                            openstudio::EndUseCategoryType::ExteriorEquipment},
                           {openstudio::EndUseFuelType::Electricity,
                            openstudio::EndUseCategoryType::WaterSystems},
                           {openstudio::EndUseFuelType::Gas,
                            openstudio::EndUseCategoryType::Heating},
                           {openstudio::EndUseFuelType::Gas,
                            openstudio::EndUseCategoryType::Cooling},
                           {openstudio::EndUseFuelType::Gas,
                            openstudio::EndUseCategoryType::InteriorEquipment},
                           {openstudio::EndUseFuelType::Gas,
                            openstudio::EndUseCategoryType::WaterSystems}};
#endif
}

void ISOModelFixture::TearDown() {}

void ISOModelFixture::SetUpTestCase() {
#ifndef ISOMODEL_STANDALONE
  // set up logging
  openstudio::Logger::instance().standardOutLogger().disable();
  logFile = std::shared_ptr<openstudio::FileLogSink>(
      new openstudio::FileLogSink(openstudio::toPath("./ISOModelFixture.log")));
#endif
}

void ISOModelFixture::TearDownTestCase() {
#ifndef ISOMODEL_STANDALONE
  logFile->disable();
#endif
}

#ifndef ISOMODEL_STANDALONE
std::shared_ptr<openstudio::FileLogSink> ISOModelFixture::logFile;
#endif