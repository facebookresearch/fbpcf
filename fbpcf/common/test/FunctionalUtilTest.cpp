/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/common/FunctionalUtil.h"

#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace fbpcf::functional {
TEST(FunctionalUtilTest, TestMap) {
  std::vector<int> input{1, 2, 3};
  std::vector<std::string> expectedOutput{"1", "2", "3"};

  auto output = map<int, std::string>(
      input, [](const auto& x) { return std::to_string(x); });
  EXPECT_EQ(expectedOutput, output);
}

TEST(FunctionalUtilTest, TestReduce) {
  std::vector<int> input{1, 2, 3};
  int expectedOutput = 6;

  auto output =
      reduce<int>(input, [](const auto& x, const auto& y) { return x + y; });
  EXPECT_EQ(expectedOutput, output);
}

TEST(FunctionalUtilTest, TestReduceWithException) {
  std::vector<int> input;

  EXPECT_THROW(
      reduce<int>(input, [](const auto& x, const auto& y) { return x + y; }),
      std::invalid_argument);
}
} // namespace fbpcf::functional
