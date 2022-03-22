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

#include "fbpcf/mpc_std_lib/util/util.h"

namespace fbpcf::mpc_std_lib::util {
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

template <typename T>
struct InputType {
  std::vector<uint32_t> indicatorShares;
  std::vector<std::vector<bool>> minuendShares;
  std::vector<T> subtrahendShares;
};

template <typename T, int indicatorWidth>
std::tuple<InputType<T>, InputType<T>, std::vector<T>> generateRandomInputs(
    size_t batchSize) {
  InputType<T> rst0{
      .indicatorShares = std::vector<uint32_t>(batchSize),
      .minuendShares = std::vector<std::vector<bool>>(),
      .subtrahendShares = std::vector<T>(batchSize),
  };
  InputType<T> rst1{
      .indicatorShares = std::vector<uint32_t>(batchSize),
      .minuendShares = std::vector<std::vector<bool>>(),
      .subtrahendShares = std::vector<T>(batchSize),
  };
  std::vector<T> expectedValue(batchSize);

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint32_t> randomIndicator(0, 0xFFFF);
  std::uniform_int_distribution<int32_t> randomBit(0, 1);

  for (size_t i = 0; i < batchSize; i++) {
    auto [minuend, minuendShare0, minuendShare1] = util::getRandomData<T>(e);
    auto subtrahend = std::get<0>(util::getRandomData<T>(e));
    // indicator is 1 or -1;
    int32_t indicator = static_cast<int32_t>(-1) + 2 * randomBit(e);
    expectedValue[i] =
        indicator == 1 ? (minuend - subtrahend) : (subtrahend - minuend);

    rst0.indicatorShares[i] = randomIndicator(e);
    rst1.indicatorShares[i] =
        (rst0.indicatorShares.at(i) - indicator) % (1 << indicatorWidth);

    rst0.subtrahendShares[i] = std::get<0>(util::getRandomData<T>(e));
    rst1.subtrahendShares[i] = rst0.subtrahendShares.at(i) - subtrahend;

    auto tmp0 = util::Adapters<T>::convertToBits(minuendShare0);
    auto tmp1 = util::Adapters<T>::convertToBits(minuendShare1);
    for (size_t j = 0; j < tmp0.size(); j++) {
      if (j >= rst0.minuendShares.size()) {
        rst0.minuendShares.push_back(std::vector<bool>(batchSize));
        rst1.minuendShares.push_back(std::vector<bool>(batchSize));
      }
      rst0.minuendShares[j][i] = tmp0[j];
      rst1.minuendShares[j][i] = tmp1[j];
    }
  }

  return {rst0, rst1, expectedValue};
}

} // namespace fbpcf::mpc_std_lib::util
