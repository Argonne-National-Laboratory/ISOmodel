#ifndef ISOMODEL_TEST_ISOMODELFIXTURE_HPP
#define ISOMODEL_TEST_ISOMODELFIXTURE_HPP

#include <gtest/gtest.h>

#ifdef ISOMODEL_STANDALONE
#include "../EndUses.hpp"
// Note: Vector.hpp no longer included here as Stage 1 replaced Boost uBLAS
// with std::vector, making the global Vector typedef standard-compatible.
#else
#include "../../utilities/core/FileLogSink.hpp"
#include "../../utilities/core/Logger.hpp"
#include "../../utilities/core/Path.hpp"
#include "../utilities/data/DataEnums.hpp"
#include "../utilities/data/EndUses.hpp"
#endif

#include <string>
#include <utility>
#include <vector>

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
  std::vector<std::pair<openstudio::EndUseFuelType, openstudio::EndUseCategoryType>>
      isoResultsEndUseTypes;

  static std::shared_ptr<openstudio::FileLogSink> logFile;
  REGISTER_LOGGER("IsoModel");
#endif
};

#endif // ISOMODEL_TEST_ISOMODELFIXTURE_HPP