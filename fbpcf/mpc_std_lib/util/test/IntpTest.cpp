/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <random>

#include "fbpcf/mpc_std_lib/util/util.h"

namespace fbpcf::mpc_std_lib::util {

TEST(IntpTypeTest, testAdd) {
  const int8_t width = 32;
  int64_t largestSigned = std::numeric_limits<int32_t>().max();
  int64_t smallestSigned = std::numeric_limits<int32_t>().min();
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<int32_t> dist(smallestSigned, largestSigned);
  for (int i = 0; i < 1000; i++) {
    auto v1 = dist(e);
    auto v2 = dist(e);
    int32_t v = Intp<true, width>(v1) + Intp<true, width>(v2);
    int32_t expectedV = (uint64_t)v1 + (uint64_t)v2;
    EXPECT_EQ(v, expectedV);
  }
}

TEST(IntpTypeTest, testSubtract) {
  const int8_t width = 32;
  int64_t largestSigned = std::numeric_limits<int32_t>().max();
  int64_t smallestSigned = std::numeric_limits<int32_t>().min();
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<int32_t> dist(smallestSigned, largestSigned);
  for (int i = 0; i < 1000; i++) {
    auto v1 = dist(e);
    auto v2 = dist(e);
    int32_t v = Intp<true, width>(v1) - Intp<true, width>(v2);
    int32_t expectedV = (uint64_t)v1 - (uint64_t)v2;
    EXPECT_EQ(v, expectedV);
  }
}

} // namespace fbpcf::mpc_std_lib::util
