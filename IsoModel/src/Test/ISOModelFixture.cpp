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