/**********************************************************************
* Copyright (c) 2008-2015, Alliance for Sustainable Energy.
* All rights reserved.
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**********************************************************************/

#ifndef ISOMODEL_TEST_ISOMODELFIXTURE_HPP
#define ISOMODEL_TEST_ISOMODELFIXTURE_HPP

#include <gtest/gtest.h>

#ifdef ISOMODEL_STANDALONE
#include "../EndUses.hpp"
// Note: Vector.hpp no longer included here as Stage 1 replaced Boost uBLAS 
// with std::vector, making the global Vector typedef standard-compatible.
#else
#include "../../utilities/core/Logger.hpp"
#include "../../utilities/core/FileLogSink.hpp"
#include "../../utilities/core/Path.hpp"
#include "../utilities/data/EndUses.hpp"
#include "../utilities/data/DataEnums.hpp"
#endif

#include <utility>
#include <vector>
#include <string>

class ISOModelFixture : public ::testing::Test {
protected:
	/// initialize for each test
	virtual void SetUp() override;

	/// tear down after each test
	virtual void TearDown() override;

	/// initialize static members
	static void SetUpTestCase();

	/// tear down static members
	static void TearDownTestCase();

	std::vector<std::string> endUseNames;
	std::string test_data_path;

#ifndef ISOMODEL_STANDALONE
	std::vector<std::pair<openstudio::EndUseFuelType, openstudio::EndUseCategoryType>> isoResultsEndUseTypes;

	static std::shared_ptr<openstudio::FileLogSink> logFile;
	REGISTER_LOGGER("IsoModel");
#endif
};

#endif // ISOMODEL_TEST_ISOMODELFIXTURE_HPP