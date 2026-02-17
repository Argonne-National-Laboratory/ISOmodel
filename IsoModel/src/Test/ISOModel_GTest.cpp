/// @file ISOModel_GTest.cpp
/// @brief Top-level Google Test entry point.
///
/// Includes the test fixture and serves as the main compilation unit
/// for the test suite.
///
/// @author Nick Collier
/// @author Brendan Albano
/// @author Ralph Muehleisen
/// @date 2014-12-05
/// @copyright Copyright Argonne National Laboratory
#include <gtest/gtest.h>

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
