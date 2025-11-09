/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "../EditDistanceApp.h" // @manual
#include <folly/Format.h>
#include <folly/json.h>
#include <gtest/gtest.h>
#include "../EditDistanceResults.h" // @manual
#include "../MPCTypes.h" // @manual
#include "./Constants.h" // @manual
#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/io/api/FileIOWrappers.h"
#include "fbpcf/test/TestHelper.h"
#include "folly/Random.h"
#include "tools/cxx/Resources.h"

namespace fbpcf::edit_distance {

static std::string getBaseDirFromPath(const std::string& filePath) {
  return filePath.substr(0, filePath.rfind("/") + 1);
}

static void cleanup(std::string file_to_delete) {
  remove(file_to_delete.c_str());
}

TEST(EditDistanceAppTest, testApp) {
  boost::filesystem::path dataFilepath1 =
      build::getResourcePath(std::getenv("DATA_FILE_PATH_1"));

  boost::filesystem::path dataFilepath2 =
      build::getResourcePath(std::getenv("DATA_FILE_PATH_2"));
  boost::filesystem::path paramsFilePath =
      build::getResourcePath(std::getenv("PARAMS_FILE_PATH"));

  std::string baseDir = getBaseDirFromPath(__FILE__);

  std::string outFilepath1 = folly::sformat(
      "{}test_data/edit_distance_output_{}.json",
      baseDir,
      folly::Random::secureRand64());
  std::string outFilepath2 = folly::sformat(
      "{}test_data/edit_distance_output_{}.json",
      baseDir,
      folly::Random::secureRand64());

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

  future0.get();
  future1.get();

  auto player0SharesJson = io::FileIOWrappers::readFile(outFilepath1);
  auto player1SharesJson = io::FileIOWrappers::readFile(outFilepath2);

  std::vector<folly::dynamic> shares = {
      folly::parseJson(player0SharesJson), folly::parseJson(player1SharesJson)};

  EditDistanceResults results(shares);
  testVectorEq(results.editDistances, kExpectedDistances);
  testVectorEq(results.receiverMessages, kExpectedReceiverMessages);

  cleanup(outFilepath1);
  cleanup(outFilepath2);
}

} // namespace fbpcf::edit_distance
