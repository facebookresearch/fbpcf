/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Format.h>
#include <gtest/gtest.h>
#include <fstream>
#include "folly/Random.h"
#include "tools/cxx/Resources.h"

#include "../EditDistanceApp.h" // @manual
#include "../Validator.h" // @manual
#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/test/TestHelper.h"

namespace fbpcf::edit_distance {

static std::string getBaseDirFromPath(const std::string& filePath) {
  return filePath.substr(0, filePath.rfind("/") + 1);
}

static void cleanup(std::string file_to_delete) {
  remove(file_to_delete.c_str());
}

static std::string randomOutFile() {
  std::string baseDir = getBaseDirFromPath(__FILE__);
  return folly::sformat(
      "{}test_data/edit_distance_output_{}.json",
      baseDir,
      folly::Random::secureRand64());
}

Validator runTest(
    const char* resultsEnvName,
    std::string outFilepath1,
    std::string outFilepath2) {
  boost::filesystem::path dataFilepath1 =
      build::getResourcePath(std::getenv("DATA_FILE_PATH_1"));

  boost::filesystem::path dataFilepath2 =
      build::getResourcePath(std::getenv("DATA_FILE_PATH_2"));
  boost::filesystem::path paramsFilePath =
      build::getResourcePath(std::getenv("PARAMS_FILE_PATH"));

  boost::filesystem::path resultsFilePath =
      build::getResourcePath(std::getenv(resultsEnvName));

  auto factories = engine::communication::getInMemoryAgentFactory(2);

  auto player0App = EditDistanceApp<0>(
      0,
      std::move(factories[0]),
      dataFilepath1.native(),
      paramsFilePath.native(),
      outFilepath1);

  auto player1App = EditDistanceApp<1>(
      1,
      std::move(factories[1]),
      dataFilepath2.native(),
      paramsFilePath.native(),
      outFilepath2);

  auto future0 = std::async([&player0App]() { player0App.run(); });
  auto future1 = std::async([&player1App]() { player1App.run(); });

  std::vector<std::string> outpaths = {outFilepath1, outFilepath2};

  future0.get();
  future1.get();

  Validator validator(outpaths, resultsFilePath.native());

  return validator;
}

TEST(ValidatorTest, testValidator) {
  std::string outFilepath1 = randomOutFile();
  std::string outFilepath2 = randomOutFile();
  Validator validator =
      runTest("RESULTS_FILE_PATH", outFilepath1, outFilepath2);
  EXPECT_EQ(validator.validate(), Validator::SUCCESS);
  cleanup(outFilepath1);
  cleanup(outFilepath2);
}

TEST(ValidatorTest, testValidatorFails) {
  std::string outFilepath1 = randomOutFile();
  std::string outFilepath2 = randomOutFile();
  Validator validator =
      runTest("INCORRECT_RESULTS_FILE_PATH", outFilepath1, outFilepath2);
  EXPECT_EQ(validator.validate(), Validator::RESULT_MISMATCH);
  cleanup(outFilepath1);
  cleanup(outFilepath2);
}

TEST(ValidatorTest, testValidatorFailsSize) {
  std::string outFilepath1 = randomOutFile();
  std::string outFilepath2 = randomOutFile();
  Validator validator =
      runTest("INCORRECT_SIZE_RESULTS_FILE_PATH", outFilepath1, outFilepath2);
  EXPECT_EQ(validator.validate(), Validator::RESULT_MISMATCH);
  cleanup(outFilepath1);
  cleanup(outFilepath2);
}

TEST(ValidatorTest, testValidatorFailsMessage) {
  std::string outFilepath1 = randomOutFile();
  std::string outFilepath2 = randomOutFile();
  Validator validator = runTest(
      "INCORRECT_MESSAGE_RESULTS_FILE_PATH", outFilepath1, outFilepath2);
  EXPECT_EQ(validator.validate(), Validator::SIZE_MISMATCH);
  cleanup(outFilepath1);
  cleanup(outFilepath2);
}

} // namespace fbpcf::edit_distance
