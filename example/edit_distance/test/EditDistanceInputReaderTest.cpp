/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "../EditDistanceInputReader.h" // @manual
#include <fbpcf/test/TestHelper.h>
#include <gtest/gtest.h>
#include <cstdlib>
#include "./Constants.h" // @manual
#include "tools/cxx/Resources.h"

namespace fbpcf::edit_distance {

TEST(EditDistanceInputReaderTest, testInputReaderPlayer1) {
  using namespace fbcpf::edit_distance;

  boost::filesystem::path dataFilepath =
      build::getResourcePath(std::getenv("DATA_FILE_PATH_1"));
  boost::filesystem::path paramsFilePath =
      build::getResourcePath(std::getenv("PARAMS_FILE_PATH"));
  EditDistanceInputReader input(dataFilepath.native(), paramsFilePath.native());

  testVectorEq(input.getWords(), kExpectedWords);
  testVectorEq(input.getGuesses(), kEmptyVector);
  testVectorEq(input.getSenderMessages(), kExpectedSenderMessages);

  EXPECT_EQ(input.getThreshold(), kExpectedThreshold);
  EXPECT_EQ(input.getDeleteCost(), kExpectedDeleteCost);
  EXPECT_EQ(input.getInsertCost(), kExpectedInsertCost);
  EXPECT_EQ(input.getNumRows(), kExpectedWords.size());
}

TEST(EditDistanceInputReaderTest, testInputReaderPlayer2) {
  using namespace fbcpf::edit_distance;

  boost::filesystem::path dataFilepath =
      build::getResourcePath(std::getenv("DATA_FILE_PATH_2"));

  boost::filesystem::path paramsFilePath =
      build::getResourcePath(std::getenv("PARAMS_FILE_PATH"));
  EditDistanceInputReader input(dataFilepath.native(), paramsFilePath.native());

  testVectorEq(input.getWords(), kEmptyVector);
  testVectorEq(input.getGuesses(), kExpectedGuesses);
  testVectorEq(input.getSenderMessages(), kEmptyVector);

  EXPECT_EQ(input.getThreshold(), kExpectedThreshold);
  EXPECT_EQ(input.getDeleteCost(), kExpectedDeleteCost);
  EXPECT_EQ(input.getInsertCost(), kExpectedInsertCost);
  EXPECT_EQ(input.getNumRows(), kExpectedWords.size());
}
} // namespace fbpcf::edit_distance
