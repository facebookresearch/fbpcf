/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <emmintrin.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <smmintrin.h>
#include <xmmintrin.h>
#include <cstddef>
#include <functional>
#include <future>
#include <iterator>
#include <random>

#include "fbpcf/mpc_framework/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/mpc_framework/mpc_std_lib/oram/DummyObliviousDeltaCalculatorFactory.h"
#include "fbpcf/mpc_framework/mpc_std_lib/oram/ObliviousDeltaCalculatorFactory.h"
#include "fbpcf/mpc_framework/mpc_std_lib/oram/test/util.h"
#include "fbpcf/mpc_framework/test/TestHelper.h"

namespace fbpcf::mpc_std_lib::oram {

struct ObliviousDeltaCalculatorInputType {
  std::vector<__m128i> delta0Shares;
  std::vector<__m128i> delta1Shares;
  std::vector<bool> alphaShares;
};

using OutputType =
    std::tuple<std::vector<__m128i>, std::vector<bool>, std::vector<bool>>;

std::tuple<
    ObliviousDeltaCalculatorInputType,
    ObliviousDeltaCalculatorInputType,
    OutputType>
generateInputs(size_t batchSize) {
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

OutputType obliviousDeltaCalculatorHelper(
    std::unique_ptr<IObliviousDeltaCalculatorFactory> factory,
    std::reference_wrapper<ObliviousDeltaCalculatorInputType> input) {
  auto calculator = factory->create();
  return calculator->calculateDelta(
      input.get().delta0Shares,
      input.get().delta1Shares,
      input.get().alphaShares);
}

void testEq(const OutputType& src1, const OutputType& src2) {
  fbpcf::testEq(std::get<0>(src1), std::get<0>(src2));
  testVectorEq(std::get<1>(src1), std::get<1>(src2));
  testVectorEq(std::get<2>(src1), std::get<2>(src2));
}

void testObliviousDeltaCalculator(
    std::unique_ptr<IObliviousDeltaCalculatorFactory> factory0,
    std::unique_ptr<IObliviousDeltaCalculatorFactory> factory1) {
  size_t batchSize = 16384;
  auto [party0Input, party1Input, expectedOutput] = generateInputs(batchSize);

  auto future0 = std::async(
      obliviousDeltaCalculatorHelper,
      std::move(factory0),
      std::reference_wrapper<ObliviousDeltaCalculatorInputType>(party0Input));

  auto future1 = std::async(
      obliviousDeltaCalculatorHelper,
      std::move(factory1),
      std::reference_wrapper<ObliviousDeltaCalculatorInputType>(party1Input));
  auto rst0 = future0.get();
  auto rst1 = future1.get();
  testEq(rst0, rst1);
  testEq(rst0, expectedOutput);
}

TEST(ObliviousDeltaCalculatorTest, testDummyObliviousDeltaCalculator) {
  auto factories = engine::communication::getInMemoryAgentFactory(2);
  auto factory0 =
      std::make_unique<insecure::DummyObliviousDeltaCalculatorFactory>(
          1, *factories[0]);
  auto factory1 =
      std::make_unique<insecure::DummyObliviousDeltaCalculatorFactory>(
          0, *factories[1]);
  testObliviousDeltaCalculator(std::move(factory0), std::move(factory1));
}

TEST(ObliviousDeltaCalculatorTest, testObliviousDeltaCalculator) {
  auto factories = engine::communication::getInMemoryAgentFactory(2);
  setupRealBackend<0, 1>(*factories[0], *factories[1]);

  auto factory0 =
      std::make_unique<ObliviousDeltaCalculatorFactory<0>>(true, 0, 1);
  auto factory1 =
      std::make_unique<ObliviousDeltaCalculatorFactory<1>>(false, 0, 1);
  testObliviousDeltaCalculator(std::move(factory0), std::move(factory1));
}

} // namespace fbpcf::mpc_std_lib::oram
