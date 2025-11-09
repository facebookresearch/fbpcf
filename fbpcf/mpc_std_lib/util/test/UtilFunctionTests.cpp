/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <emmintrin.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <cstddef>
#include <functional>
#include <future>
#include <iterator>
#include <memory>
#include <random>

#include "fbpcf/engine/util/util.h"
#include "fbpcf/mpc_std_lib/util/test/util.h"
#include "fbpcf/mpc_std_lib/util/util.h"
#include "fbpcf/test/TestHelper.h"

namespace fbpcf::mpc_std_lib::util {

template <typename T>
void testConvertingBits() {
  std::random_device rd;
  std::mt19937_64 e(rd());
  for (size_t i = 0; i < 10000 /* repeat this for a lot of times*/; i++) {
    auto [v, _1, _2] = getRandomData<uint32_t>(e);
    auto convertedV = Adapters<uint32_t>::convertFromBits(
        Adapters<uint32_t>::convertToBits(v));
    EXPECT_EQ(v, convertedV);
  }
}

TEST(ConvertingBitsTest, testConvertingBits) {
  testConvertingBits<uint32_t>();
  testConvertingBits<AggregationValue>();
}

TEST(ConvertingBitsTest, testConvertingBitsForM128iVector) {
  std::vector<__m128i> v(10000);
  for (auto& item : v) {
    item = engine::util::getRandomM128iFromSystemNoise();
  }

  auto convertedV = convertFromBits(convertToBits(v));
  testEq(v, convertedV);
}

} // namespace fbpcf::mpc_std_lib::util
