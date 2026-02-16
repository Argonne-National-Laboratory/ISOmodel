#pragma once
#include <gtest/gtest.h>

#include "../EndUses.hpp"

#include <string>
#include <vector>

class ISOModelFixture : public ::testing::Test {
protected:
  void SetUp() override;
  void TearDown() override;
  static void SetUpTestCase();
  static void TearDownTestCase();

  std::vector<std::string> endUseNames;
  std::string test_data_path;
};

