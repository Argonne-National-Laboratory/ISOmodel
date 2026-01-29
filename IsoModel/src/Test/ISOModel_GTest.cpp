// First Commit: 2014-12-05
//
// Authors:
// - Brendan Albano
// - Nick Collier
// - Ralph Muehleisen
//
// Summary:
// This file contains the main function for the ISOModel Google Test suite. It
// is responsible for initializing the test framework and running all
// registered tests.

#include "gtest/gtest.h"

#include "../UserModel.hpp"

using namespace openstudio::isomodel;

std::string test_data_path;

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
