/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <fbpcf/mpc_std_lib/walr_multiplication/util/NumberMapper.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <limits>
#include <random>
#include <vector>

namespace fbpcf::mpc_std_lib::walr::util {

void testConversionEq(
    const std::vector<double>& input,
    const std::vector<double>& output,
    uint64_t divisor) {
  double tolerance = 1.0 / divisor;
  ASSERT_EQ(input.size(), output.size());

  // (1) When abs(input) >= 1.0, the RELATIVE precision loss should be
  // less than abs(input) / divisor.
  // (2) When abs(input) < 1.0, the ABSOLUTE precision loss should be
  // less than 1 / divisor
  for (size_t i = 0; i < output.size(); ++i) {
    EXPECT_NEAR(
        input[i],
        output[i],
        std::max({std::abs(input[i]), std::abs(output[i]), 1.0}) * tolerance);
  }
}

TEST(numberMapperTest, testBasicConversion) {
  constexpr uint64_t divisor = static_cast<uint64_t>(1e7);
  auto mapper32 = NumberMapper<uint32_t>(divisor);
  constexpr uint64_t groupSize =
      (uint64_t)std::numeric_limits<uint32_t>::max() + 1;
  // Basic conversion
  // 1.0 / 7 = 0.1428571428571
  EXPECT_EQ(mapper32.mapToFixedPointType(1.0 / 7), 1428571);
  EXPECT_EQ(mapper32.mapToFixedPointType(7.0), 70000000);
  EXPECT_EQ(mapper32.mapToFixedPointType(0.0), 0);

  // Converting negative number
  // On the integer group of size 2^32, -k is equivalent to 2^32 - k
  EXPECT_EQ(mapper32.mapToFixedPointType(-1.0 / 7), groupSize - 1428571);
}

TEST(numberMapperTest, testUnsignedConversionPrecision) {
  constexpr uint64_t divisor = static_cast<uint64_t>(1e9);
  auto mapper64 = NumberMapper<uint64_t>(divisor);
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_real_distribution<double> dist(0, 10.1);
  std::vector<double> input(99);
  std::generate(input.begin(), input.end(), [&dist, &e]() { return dist(e); });
  input.push_back(0.0);

  auto output =
      mapper64.mapToUnsignedDouble(mapper64.mapToFixedPointType(input));
  testConversionEq(input, output, divisor);
}

TEST(numberMapperTest, testSignedConversionPrecision) {
  constexpr uint64_t divisor = static_cast<uint64_t>(1e9);
  auto mapper64 = NumberMapper<uint64_t>(divisor);

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_real_distribution<double> dist(-100.1, 100.1);
  std::vector<double> input(99);
  std::generate(input.begin(), input.end(), [&dist, &e]() { return dist(e); });
  input.push_back(0.0);

  auto output = mapper64.mapToSignedDouble(mapper64.mapToFixedPointType(input));
  testConversionEq(input, output, divisor);
}
} // namespace fbpcf::mpc_std_lib::walr::util
