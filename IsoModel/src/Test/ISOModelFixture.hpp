/// @file ISOModelFixture.hpp
/// @brief Google Test fixture with shared UserModel setup.
///
/// Loads the test building YAML and EPW files once in SetUpTestSuite()
/// and provides the configured UserModel to all test cases.
///
/// @author Brendan Albano
/// @author Ralph Muehleisen
/// @date 2015-09-20
/// @copyright Copyright Argonne National Laboratory
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

