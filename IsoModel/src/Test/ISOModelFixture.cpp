/// @file ISOModelFixture.cpp
/// @brief Google Test fixture with shared UserModel setup.
///
/// Loads the test building YAML and EPW files once in SetUpTestSuite()
/// and provides the configured UserModel to all test cases.
///
/// @author Brendan Albano
/// @author Ralph Muehleisen
/// @date 2015-09-20
/// @copyright Copyright Argonne National Laboratory
#include "ISOModelFixture.hpp"

void ISOModelFixture::SetUp() {
  endUseNames = {"ElecHeat", "ElecCool",    "ElecIntLights", "ElecExtLights", "ElecFans",
                 "ElecPump", "ElecEquipInt", "ElecEquipExt",  "ElectDHW",      "GasHeat",
                 "GasCool",  "GasEquip",     "GasDHW"};
  test_data_path = "test_data";
}

void ISOModelFixture::TearDown() {}

void ISOModelFixture::SetUpTestCase() {}

void ISOModelFixture::TearDownTestCase() {}