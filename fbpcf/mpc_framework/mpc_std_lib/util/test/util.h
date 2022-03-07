/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emmintrin.h>
#include <smmintrin.h>
#include <functional>
#include <random>
#include <stdexcept>

#include "fbpcf/mpc_framework/mpc_std_lib/util/util.h"
#include "fbpcf/mpc_framework/test/TestHelper.h"

namespace fbpcf::mpc_framework::mpc_std_lib::util {
// the 3 returned valeus are: true value, first share, second share
template <typename T>
std::tuple<T, T, T> getRandomData(std::mt19937_64& e);

template <>
inline std::tuple<uint32_t, uint32_t, uint32_t> getRandomData<uint32_t>(
    std::mt19937_64& e) {
  std::uniform_int_distribution<uint32_t> randomValue(0, 0xFFFFFF);
  std::uniform_int_distribution<uint32_t> randomMask(0, 0xFFFFFFFF);
  uint32_t value = randomValue(e);
  uint32_t share0 = randomMask(e);
  uint32_t share1 = value ^ share0;
  return {value, share0, share1};
}

template <>
inline std::tuple<AggregationValue, AggregationValue, AggregationValue>
getRandomData<AggregationValue>(std::mt19937_64& e) {
  std::uniform_int_distribution<uint32_t> randomValue(0, 0xFFFFFF);
  std::uniform_int_distribution<uint32_t> randomMask(0, 0xFFFFFFFF);

  AggregationValue value;
  value.conversionCount = randomValue(e);
  value.conversionValue = randomValue(e);

  AggregationValue valueShare0;
  valueShare0.conversionCount = randomMask(e);
  valueShare0.conversionValue = randomMask(e);

  AggregationValue valueShare1;
  valueShare1.conversionCount =
      value.conversionCount ^ valueShare0.conversionCount;
  valueShare1.conversionValue =
      value.conversionValue ^ valueShare0.conversionValue;
  return {value, valueShare0, valueShare1};
}

} // namespace fbpcf::mpc_framework::mpc_std_lib::util
