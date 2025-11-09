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
#include <random>

#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/mpc_std_lib/oram/DummyObliviousDeltaCalculatorFactory.h"
#include "fbpcf/mpc_std_lib/oram/ObliviousDeltaCalculatorFactory.h"
#include "fbpcf/mpc_std_lib/oram/test/util.h"
#include "fbpcf/test/TestHelper.h"

namespace fbpcf::mpc_std_lib::oram {

util::ObliviousDeltaCalculatorOutputType obliviousDeltaCalculatorHelper(
    std::unique_ptr<IObliviousDeltaCalculatorFactory> factory,
    std::reference_wrapper<util::ObliviousDeltaCalculatorInputType> input) {
  auto calculator = factory->create();
  return calculator->calculateDelta(
      input.get().delta0Shares,
      input.get().delta1Shares,
      input.get().alphaShares);
}

void testEq(
    const util::ObliviousDeltaCalculatorOutputType& src1,
    const util::ObliviousDeltaCalculatorOutputType& src2) {
  fbpcf::testEq(std::get<0>(src1), std::get<0>(src2));
  testVectorEq(std::get<1>(src1), std::get<1>(src2));
  testVectorEq(std::get<2>(src1), std::get<2>(src2));
}

void testObliviousDeltaCalculator(
    std::unique_ptr<IObliviousDeltaCalculatorFactory> factory0,
    std::unique_ptr<IObliviousDeltaCalculatorFactory> factory1) {
  size_t batchSize = 16384;
  auto [party0Input, party1Input, expectedOutput] =
      util::generateObliviousDeltaCalculatorInputs(batchSize);

  auto future0 = std::async(
      obliviousDeltaCalculatorHelper,
      std::move(factory0),
      std::reference_wrapper<util::ObliviousDeltaCalculatorInputType>(
          party0Input));

  auto future1 = std::async(
      obliviousDeltaCalculatorHelper,
      std::move(factory1),
      std::reference_wrapper<util::ObliviousDeltaCalculatorInputType>(
          party1Input));
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
