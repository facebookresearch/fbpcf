/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <cmath>
#include <cstddef>
#include <functional>
#include <future>
#include <iterator>
#include <memory>
#include <random>

#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/mpc_std_lib/oram/DifferenceCalculatorFactory.h"
#include "fbpcf/mpc_std_lib/oram/DummyDifferenceCalculatorFactory.h"
#include "fbpcf/mpc_std_lib/oram/test/util.h"
#include "fbpcf/mpc_std_lib/util/test/util.h"
#include "fbpcf/mpc_std_lib/util/util.h"

namespace fbpcf::mpc_std_lib::oram {

template <typename T>
std::vector<T> differenceCalculatorHelper(
    std::unique_ptr<IDifferenceCalculatorFactory<T>> factory,
    const util::InputType<T>& input) {
  auto calculator = factory->create();
  return calculator->calculateDifferenceBatch(
      input.indicatorShares, input.minuendShares, input.subtrahendShares);
}

template <typename T, int indicatorWidth>
void testDifferenceCalculator(
    std::unique_ptr<IDifferenceCalculatorFactory<T>> factory0,
    std::unique_ptr<IDifferenceCalculatorFactory<T>> factory1) {
  size_t batchSize = 16384;

  auto [input0, input1, expectedValue] =
      util::generateRandomInputs<T, indicatorWidth>(batchSize);

  auto future0 = std::async(
      differenceCalculatorHelper<T>,
      std::move(factory0),
      std::reference_wrapper<util::InputType<T>>(input0));
  auto future1 = std::async(
      differenceCalculatorHelper<T>,
      std::move(factory1),
      std::reference_wrapper<util::InputType<T>>(input1));

  auto result0 = future0.get();
  auto result1 = future1.get();

  ASSERT_EQ(result0.size(), batchSize);
  ASSERT_EQ(result1.size(), batchSize);
  ASSERT_EQ(expectedValue.size(), batchSize);

  for (size_t i = 0; i < batchSize; i++) {
    EXPECT_EQ(result0.at(i), result1.at(i));
    EXPECT_EQ(result0.at(i), expectedValue.at(i));
  }
}

TEST(DifferenceCalculatorTest, testDummyDifferenceCalculator) {
  auto factories = engine::communication::getInMemoryAgentFactory(2);
  const int8_t indicatorSumWidth = 16;

  auto uint32Factory0 = std::make_unique<
      insecure::DummyDifferenceCalculatorFactory<uint32_t, indicatorSumWidth>>(
      true, 1, *factories[0]);
  auto uint32Factory1 = std::make_unique<
      insecure::DummyDifferenceCalculatorFactory<uint32_t, indicatorSumWidth>>(
      false, 0, *factories[1]);
  testDifferenceCalculator<uint32_t, indicatorSumWidth>(
      std::move(uint32Factory0), std::move(uint32Factory1));

  auto aggregationValueFactory0 =
      std::make_unique<insecure::DummyDifferenceCalculatorFactory<
          util::AggregationValue,
          indicatorSumWidth>>(true, 1, *factories[0]);
  auto aggregationValueFactory1 =
      std::make_unique<insecure::DummyDifferenceCalculatorFactory<
          util::AggregationValue,
          indicatorSumWidth>>(false, 0, *factories[1]);
  testDifferenceCalculator<util::AggregationValue, 16>(
      std::move(aggregationValueFactory0), std::move(aggregationValueFactory1));
}

TEST(DifferenceCalculatorTest, testDifferenceCalculator) {
  auto factories = engine::communication::getInMemoryAgentFactory(2);
  setupRealBackend<0, 1>(*factories[0], *factories[1]);
  const int8_t indicatorSumWidth = 16;
  testDifferenceCalculator<uint32_t, indicatorSumWidth>(
      std::make_unique<
          DifferenceCalculatorFactory<uint32_t, indicatorSumWidth, 0>>(
          true, 0, 1),
      std::make_unique<
          DifferenceCalculatorFactory<uint32_t, indicatorSumWidth, 1>>(
          false, 0, 1));

  testDifferenceCalculator<util::AggregationValue, indicatorSumWidth>(
      std::make_unique<DifferenceCalculatorFactory<
          util::AggregationValue,
          indicatorSumWidth,
          0>>(true, 0, 1),
      std::make_unique<DifferenceCalculatorFactory<
          util::AggregationValue,
          indicatorSumWidth,
          1>>(false, 0, 1));
}

} // namespace fbpcf::mpc_std_lib::oram
