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

#include "fbpcf/engine/util/util.h"
#include "fbpcf/mpc_std_lib/util/util.h"

namespace fbpcf::mpc_std_lib::util {

inline std::vector<uint32_t> generateRandomPermutation(size_t size) {
  std::vector<uint32_t> rst(size);
  for (size_t i = 0; i < size; i++) {
    rst[i] = i;
  }
  std::random_shuffle(rst.begin(), rst.end());
  return rst;
}

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

struct ObliviousDeltaCalculatorInputType {
  std::vector<__m128i> delta0Shares;
  std::vector<__m128i> delta1Shares;
  std::vector<bool> alphaShares;
};

using ObliviousDeltaCalculatorOutputType =
    std::tuple<std::vector<__m128i>, std::vector<bool>, std::vector<bool>>;

inline std::tuple<
    ObliviousDeltaCalculatorInputType,
    ObliviousDeltaCalculatorInputType,
    ObliviousDeltaCalculatorOutputType>
generateObliviousDeltaCalculatorInputs(size_t batchSize) {
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint64_t> randomUint64(0, 0xFFFFFFFFFFFFFFFF);
  std::uniform_int_distribution<int32_t> randomBit(0, 1);

  std::vector<__m128i> delta0(batchSize);
  std::vector<__m128i> delta1(batchSize);
  std::vector<bool> alpha(batchSize);

  std::vector<__m128i> delta(batchSize);
  std::vector<bool> t0(batchSize);
  std::vector<bool> t1(batchSize);

  std::vector<__m128i> delta0Shares(batchSize);
  std::vector<__m128i> delta1Shares(batchSize);
  std::vector<bool> alphaShares(batchSize);

  for (size_t i = 0; i < batchSize; i++) {
    delta0[i] = _mm_set_epi64x(randomUint64(e), randomUint64(e));
    delta1[i] = _mm_set_epi64x(randomUint64(e), randomUint64(e));
    alpha[i] = randomBit(e);
    if (alpha.at(i) == 0) {
      delta[i] = delta1.at(i);
    } else {
      delta[i] = delta0.at(i);
    }
    t0[i] = engine::util::getLsb(delta0.at(i)) ^ alpha.at(i) ^ 1;
    t1[i] = engine::util::getLsb(delta1.at(i)) ^ alpha.at(i);

    delta0Shares[i] = _mm_set_epi64x(randomUint64(e), randomUint64(e));
    delta1Shares[i] = _mm_set_epi64x(randomUint64(e), randomUint64(e));
    alphaShares[i] = randomBit(e);

    delta0[i] = _mm_xor_si128(delta0Shares.at(i), delta0.at(i));
    delta1[i] = _mm_xor_si128(delta1Shares.at(i), delta1.at(i));
    alpha[i] = alpha[i] ^ alphaShares[i];
  }
  return {
      ObliviousDeltaCalculatorInputType{
          .delta0Shares = delta0Shares,
          .delta1Shares = delta1Shares,
          .alphaShares = alphaShares},
      ObliviousDeltaCalculatorInputType{
          .delta0Shares = delta0,
          .delta1Shares = delta1,
          .alphaShares = alpha,
      },
      {delta, t0, t1}};
}

inline std::tuple<std::vector<bool>, std::vector<bool>, uint32_t>
generateSharedRandomBoolVectorForSinglePointArrayGenerator(size_t length) {
  uint32_t width = std::ceil(std::log2(length));
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint32_t> dist(0, length - 1);
  std::uniform_int_distribution<uint32_t> randomMask(
      0, (uint64_t(1) << width) - 1);

  auto value = dist(e);
  auto mask0 = randomMask(e);
  auto mask1 = mask0 ^ value;
  auto rst0 = util::Adapters<uint32_t>::convertToBits(mask0);
  auto rst1 = util::Adapters<uint32_t>::convertToBits(mask1);
  rst0.erase(rst0.begin() + width, rst0.end());
  rst1.erase(rst1.begin() + width, rst1.end());

  return {rst0, rst1, value};
}

struct WritingType {
  std::vector<std::vector<bool>> indexShares;
  std::vector<std::vector<bool>> valueShares;
};

template <typename T>
inline std::tuple<WritingType, WritingType, std::vector<T>>
generateRandomValuesToAdd(size_t oramSize, size_t batchSize) {
  size_t indexWidth = std::ceil(std::log2(oramSize));

  WritingType input0{
      .indexShares = std::vector<std::vector<bool>>(
          indexWidth, std::vector<bool>(batchSize)),
      .valueShares = std::vector<std::vector<bool>>()};

  WritingType input1{
      .indexShares = std::vector<std::vector<bool>>(
          indexWidth, std::vector<bool>(batchSize)),
      .valueShares = std::vector<std::vector<bool>>()};

  std::vector<T> expectedValue(oramSize, T(0));

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint32_t> randomIndex(0, oramSize - 1);
  std::uniform_int_distribution<uint32_t> randomMask(0, 0xFFFFFFFF);

  for (size_t i = 0; i < batchSize; i++) {
    auto index = randomIndex(e);
    auto [value, share0, share1] = util::getRandomData<T>(e);
    expectedValue[index] += value;

    auto indexShare = randomMask(e);
    auto tmp0 = util::Adapters<uint32_t>::convertToBits(indexShare);
    auto tmp1 = util::Adapters<uint32_t>::convertToBits(index ^ indexShare);
    for (size_t j = 0; j < indexWidth; j++) {
      input0.indexShares[j][i] = tmp0.at(j);
      input1.indexShares[j][i] = tmp1.at(j);
    }

    tmp0 = util::Adapters<T>::convertToBits(share0);
    tmp1 = util::Adapters<T>::convertToBits(share1);
    for (size_t j = 0; j < tmp0.size(); j++) {
      if (j >= input0.valueShares.size()) {
        input0.valueShares.push_back(std::vector<bool>(batchSize));
        input1.valueShares.push_back(std::vector<bool>(batchSize));
      }
      input0.valueShares[j][i] = tmp0[j];
      input1.valueShares[j][i] = tmp1[j];
    }
  }
  return {input0, input1, expectedValue};
}
} // namespace fbpcf::mpc_std_lib::util
