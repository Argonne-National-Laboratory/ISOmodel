/*
 * ISOModel_GTest.cpp
 *
 *  Created on: Dec 5, 2014
 *      Author: nick
 */

#include "gtest/gtest.h"
#include "TestEnvironment.h"

#include "../Properties.hpp"
#include "../UserModel.hpp"

using namespace openstudio::isomodel;

std::string test_data_path;

int main(int argc, char** argv)
{
  if (argc < 2) {
    std::cout << "Usage: isomodel_unit_tests test_data_directory" << std::endl;
    return 0;
  } else {
    ::testing::InitGoogleTest(&argc, argv);
    test_data_path = argv[argc - 1];
    std::cout << test_data_path << std::endl;
    return RUN_ALL_TESTS();
  }

}

