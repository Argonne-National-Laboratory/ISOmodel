/**********************************************************************
*  Copyright (c) 2008-2015, Alliance for Sustainable Energy.
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

#include "ISOModelFixture.hpp"

#include <resources.hxx>

void ISOModelFixture::SetUp() {
  endUseNames = {"ElecHeat",
                 "ElecCool",
                 "ElecIntLights",
                 "ElecExtLights",
                 "ElecFans",
                 "ElecPump",
                 "ElecEquipInt",
                 "ElecEquipExt",
                 "ElectDHW",
                 "GasHeat",
                 "GasCool",
                 "GasEquip",
                 "GasDHW"};

#ifdef ISOMODEL_STANDALONE
  // Set up stuff for standalone.
#else
  test_data_path = resourcesPath().string() + "/isomodel";

  isoResultsEndUseTypes = {
    {openstudio::EndUseFuelType::Electricity, openstudio::EndUseCategoryType::Heating},
    {openstudio::EndUseFuelType::Electricity, openstudio::EndUseCategoryType::Cooling},
    {openstudio::EndUseFuelType::Electricity, openstudio::EndUseCategoryType::InteriorLights},
    {openstudio::EndUseFuelType::Electricity, openstudio::EndUseCategoryType::ExteriorLights},
    {openstudio::EndUseFuelType::Electricity, openstudio::EndUseCategoryType::Fans},
    {openstudio::EndUseFuelType::Electricity, openstudio::EndUseCategoryType::Pumps},
    {openstudio::EndUseFuelType::Electricity, openstudio::EndUseCategoryType::InteriorEquipment},
    {openstudio::EndUseFuelType::Electricity, openstudio::EndUseCategoryType::ExteriorEquipment},
    {openstudio::EndUseFuelType::Electricity, openstudio::EndUseCategoryType::WaterSystems},
    {openstudio::EndUseFuelType::Gas, openstudio::EndUseCategoryType::Heating},
    {openstudio::EndUseFuelType::Gas, openstudio::EndUseCategoryType::Cooling},
    {openstudio::EndUseFuelType::Gas, openstudio::EndUseCategoryType::InteriorEquipment},
    {openstudio::EndUseFuelType::Gas, openstudio::EndUseCategoryType::WaterSystems}
  };
#endif
}

void ISOModelFixture::TearDown() {}

void ISOModelFixture::SetUpTestCase() {
  // set up logging
  openstudio::Logger::instance().standardOutLogger().disable();
  logFile = std::shared_ptr<openstudio::FileLogSink>(new openstudio::FileLogSink(openstudio::toPath("./ISOModelFixture.log")));
  
}

void ISOModelFixture::TearDownTestCase() {
  logFile->disable();
}

std::shared_ptr<openstudio::FileLogSink> ISOModelFixture::logFile;