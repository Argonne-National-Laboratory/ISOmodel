/*
 * ISOModel_GTest.cpp
 *
 *  Created on: Dec 5, 2014
 *      Author: nick
 */

#ifndef ISOMODEL_TESTENVIRONMENT_H
#define ISOMODEL_TESTENVIRONMENT_H

#include "gtest/gtest.h"
#include "../UserModel.hpp"

using namespace openstudio::isomodel;

// TODO: These global variables don't seem like a great way to share data between the different gtest source files.
// Fix this by figuring out how to replace this functionality using GTest environment variables or something along
// those lines. BAA@2015-07-16.
extern std::string test_data_path; // Defined in ISOModelGTest.cpp
extern std::vector<std::string> endUseNames; // Defined in MonthlyModelTest.cpp

#endif // ISOMODEL_TESTENVIRONMENT_H
