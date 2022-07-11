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
#include "tools/cxx/Resources.h"

namespace fbpcf::edit_distance {

const std::vector<std::string> EXPECTED_WORDS = {"temporary", "mug"};
const std::vector<std::string> EXPECTED_SENDER_MESSAGES = {"box", "soft"};
const std::vector<std::string> EXPECTED_GUESS = {"curvy", "grain"};
const std::vector<std::string> EMPTY_VECTOR = {};

const int EXPECTED_THRESHOLD = 100;
const int EXPECTED_DELETE_COST = 35;
const int EXPECTED_INSERT_COST = 30;

TEST(EditDistanceInputReaderTest, testInputReaderPlayer1) {
  boost::filesystem::path dataFilepath =
      build::getResourcePath(std::getenv("DATA_FILE_PATH_1"));
  boost::filesystem::path paramsFilePath =
      build::getResourcePath(std::getenv("PARAMS_FILE_PATH"));
  EditDistanceInputReader input(dataFilepath.native(), paramsFilePath.native());

  testVectorEq(input.getWords(), EXPECTED_WORDS);
  testVectorEq(input.getGuesses(), EMPTY_VECTOR);
  testVectorEq(input.getSenderMessages(), EXPECTED_SENDER_MESSAGES);

  EXPECT_EQ(input.getThreshold(), EXPECTED_THRESHOLD);
  EXPECT_EQ(input.getDeleteCost(), EXPECTED_DELETE_COST);
  EXPECT_EQ(input.getInsertCost(), EXPECTED_INSERT_COST);
  EXPECT_EQ(input.getNumRows(), EXPECTED_WORDS.size());
}

TEST(EditDistanceInputReaderTest, testInputReaderPlayer2) {
  boost::filesystem::path dataFilepath =
      build::getResourcePath(std::getenv("DATA_FILE_PATH_2"));

  boost::filesystem::path paramsFilePath =
      build::getResourcePath(std::getenv("PARAMS_FILE_PATH"));
  EditDistanceInputReader input(dataFilepath.native(), paramsFilePath.native());

  testVectorEq(input.getWords(), EMPTY_VECTOR);
  testVectorEq(input.getGuesses(), EXPECTED_GUESS);
  testVectorEq(input.getSenderMessages(), EMPTY_VECTOR);

  EXPECT_EQ(input.getThreshold(), EXPECTED_THRESHOLD);
  EXPECT_EQ(input.getDeleteCost(), EXPECTED_DELETE_COST);
  EXPECT_EQ(input.getInsertCost(), EXPECTED_INSERT_COST);
  EXPECT_EQ(input.getNumRows(), EXPECTED_WORDS.size());
}
} // namespace fbpcf::edit_distance
