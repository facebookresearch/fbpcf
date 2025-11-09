/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <emmintrin.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <cmath>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>

#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/engine/util/AesPrgFactory.h"
#include "fbpcf/mpc_std_lib/oram/DifferenceCalculatorFactory.h"
#include "fbpcf/mpc_std_lib/oram/DummyDifferenceCalculator.h"
#include "fbpcf/mpc_std_lib/oram/DummyDifferenceCalculatorFactory.h"
#include "fbpcf/mpc_std_lib/oram/DummyObliviousDeltaCalculatorFactory.h"
#include "fbpcf/mpc_std_lib/oram/DummySinglePointArrayGeneratorFactory.h"
#include "fbpcf/mpc_std_lib/oram/LinearOramFactory.h"
#include "fbpcf/mpc_std_lib/oram/ObliviousDeltaCalculatorFactory.h"
#include "fbpcf/mpc_std_lib/oram/SinglePointArrayGeneratorFactory.h"
#include "fbpcf/mpc_std_lib/oram/WriteOnlyOram.h"
#include "fbpcf/mpc_std_lib/oram/WriteOnlyOramFactory.h"
#include "fbpcf/mpc_std_lib/oram/test/util.h"
#include "fbpcf/mpc_std_lib/util/test/util.h"
#include "fbpcf/mpc_std_lib/util/util.h"

namespace fbpcf::mpc_std_lib::oram {

template <typename T>
std::pair<std::vector<T>, std::vector<T>> writeOnlyOramHelper(
    std::unique_ptr<IWriteOnlyOramFactory<T>> factory,
    size_t oramSize,
    const util::WritingType& input) {
  auto oram = factory->create(oramSize);
  oram->obliviousAddBatch(input.indexShares, input.valueShares);
  std::vector<T> rst(oramSize);
  std::vector<T> secRst(oramSize);
  for (size_t i = 0; i < oramSize; i++) {
    rst[i] = oram->publicRead(i, IWriteOnlyOram<T>::Alice);
    secRst[i] = oram->secretRead(i);
  }
  return std::make_pair(rst, secRst);
}

template <typename T>
void testWriteOnlyOram(
    std::unique_ptr<IWriteOnlyOramFactory<T>> factory0,
    std::unique_ptr<IWriteOnlyOramFactory<T>> factory1,
    size_t oramSize) {
  size_t batchSize = oramSize * 30;

  auto [input0, input1, expectedValue] =
      util::generateRandomValuesToAdd<T>(oramSize, batchSize);

  auto future0 = std::async(
      writeOnlyOramHelper<T>,
      std::move(factory0),
      oramSize,
      std::reference_wrapper<util::WritingType>(input0));
  auto future1 = std::async(
      writeOnlyOramHelper<T>,
      std::move(factory1),
      oramSize,
      std::reference_wrapper<util::WritingType>(input1));

  auto result0 = future0.get();
  auto result1 = future1.get();

  ASSERT_EQ(result0.first.size(), oramSize);
  ASSERT_EQ(result0.second.size(), oramSize);
  ASSERT_EQ(expectedValue.size(), oramSize);

  for (size_t i = 0; i < oramSize; i++) {
    EXPECT_EQ(result0.first.at(i), expectedValue.at(i));
    EXPECT_EQ(result1.first.at(i), T(0));
    EXPECT_EQ(result0.second.at(i) + result1.second.at(i), expectedValue.at(i));
  }
}

template <typename T>
void runOramTestWithDummyComponents() {
  const int8_t indicatorSumWidth = 12;

  auto factories = engine::communication::getInMemoryAgentFactory(2);

  auto factory0 = std::make_unique<WriteOnlyOramFactory<T>>(
      IWriteOnlyOram<T>::Alice,
      1,
      *factories[0],
      std::make_unique<insecure::DummySinglePointArrayGeneratorFactory>(
          true, 1, *factories[0]),
      std::make_unique<
          insecure::DummyDifferenceCalculatorFactory<T, indicatorSumWidth>>(
          true, 1, *factories[0]));

  auto factory1 = std::make_unique<WriteOnlyOramFactory<T>>(
      IWriteOnlyOram<T>::Bob,
      0,
      *factories[1],
      std::make_unique<insecure::DummySinglePointArrayGeneratorFactory>(
          false, 0, *factories[1]),
      std::make_unique<
          insecure::DummyDifferenceCalculatorFactory<T, indicatorSumWidth>>(
          false, 0, *factories[1]));

  size_t oramSize = 10; // use a smaller number due to performance issue.
  testWriteOnlyOram<T>(std::move(factory0), std::move(factory1), oramSize);
}

TEST(WriteOnlyORAMTest, TestWriteOnlyORAMWithDummyComponents) {
  runOramTestWithDummyComponents<util::TestIntp>();
  runOramTestWithDummyComponents<util::AggregationValue>();
}

template <typename T>
void runOramTestWithRealDifferenceCalculatorAndDummySinglePointArrayGenerator(
    engine::communication::IPartyCommunicationAgentFactory& agentFactory0,
    engine::communication::IPartyCommunicationAgentFactory& agentFactory1) {
  const int8_t indicatorSumWidth = 12;
  auto factory0 = std::make_unique<WriteOnlyOramFactory<T>>(
      IWriteOnlyOram<T>::Alice,
      1,
      agentFactory0,
      std::make_unique<insecure::DummySinglePointArrayGeneratorFactory>(
          true, 1, agentFactory0),
      std::make_unique<DifferenceCalculatorFactory<T, indicatorSumWidth, 0>>(
          true, 0, 1));

  auto factory1 = std::make_unique<WriteOnlyOramFactory<T>>(
      IWriteOnlyOram<T>::Bob,
      0,
      agentFactory1,
      std::make_unique<insecure::DummySinglePointArrayGeneratorFactory>(
          false, 0, agentFactory1),
      std::make_unique<DifferenceCalculatorFactory<T, indicatorSumWidth, 1>>(
          false, 0, 1));

  size_t oramSize = 30;
  testWriteOnlyOram<T>(std::move(factory0), std::move(factory1), oramSize);
}

TEST(
    WriteOnlyORAMTest,
    TestWriteOnlyORAMWithRealDifferenceCalculatorAndDummySinglePointArrayGenerator) {
  auto factories = engine::communication::getInMemoryAgentFactory(2);
  setupRealBackend<0, 1>(*factories[0], *factories[1]);

  runOramTestWithRealDifferenceCalculatorAndDummySinglePointArrayGenerator<
      util::TestIntp>(*factories[0], *factories[1]);
  runOramTestWithRealDifferenceCalculatorAndDummySinglePointArrayGenerator<
      util::AggregationValue>(*factories[0], *factories[1]);
}

template <typename T>
void runOramTestWithDummyOblivousDeltaCalculator(
    engine::communication::IPartyCommunicationAgentFactory& agentFactory0,
    engine::communication::IPartyCommunicationAgentFactory& agentFactory1) {
  const int8_t indicatorSumWidth = 12;
  auto factory0 = std::make_unique<WriteOnlyOramFactory<T>>(
      IWriteOnlyOram<T>::Alice,
      1,
      agentFactory0,
      std::make_unique<SinglePointArrayGeneratorFactory>(
          true,
          std::make_unique<insecure::DummyObliviousDeltaCalculatorFactory>(
              1, agentFactory0)),
      std::make_unique<DifferenceCalculatorFactory<T, indicatorSumWidth, 0>>(
          true, 0, 1));

  auto factory1 = std::make_unique<WriteOnlyOramFactory<T>>(
      IWriteOnlyOram<T>::Bob,
      0,
      agentFactory1,
      std::make_unique<SinglePointArrayGeneratorFactory>(
          false,
          std::make_unique<insecure::DummyObliviousDeltaCalculatorFactory>(
              0, agentFactory1)),
      std::make_unique<DifferenceCalculatorFactory<T, indicatorSumWidth, 1>>(
          false, 0, 1));

  size_t oramSize = 30;
  testWriteOnlyOram<T>(std::move(factory0), std::move(factory1), oramSize);
}

TEST(WriteOnlyORAMTest, TestWriteOnlyORAMWithDummyOblivousDeltaCalculator) {
  auto factories = engine::communication::getInMemoryAgentFactory(2);
  setupRealBackend<0, 1>(*factories[0], *factories[1]);

  runOramTestWithDummyOblivousDeltaCalculator<util::TestIntp>(
      *factories[0], *factories[1]);
  runOramTestWithDummyOblivousDeltaCalculator<util::AggregationValue>(
      *factories[0], *factories[1]);
}

template <typename T>
void runOramTestWithSecureComponents(
    engine::communication::IPartyCommunicationAgentFactory& agentFactory0,
    engine::communication::IPartyCommunicationAgentFactory& agentFactory1) {
  const int8_t indicatorSumWidth = 12;

  auto factory0 = getSecureWriteOnlyOramFactory<T, indicatorSumWidth, 0>(
      true, 0, 1, agentFactory0);

  auto factory1 = getSecureWriteOnlyOramFactory<T, indicatorSumWidth, 1>(
      false, 0, 1, agentFactory1);

  size_t oramSize = 30;
  testWriteOnlyOram<T>(std::move(factory0), std::move(factory1), oramSize);
}

TEST(WriteOnlyORAMTest, TestWriteOnlyORAMWithSecureComponents) {
  auto factories = engine::communication::getInMemoryAgentFactory(2);
  setupRealBackend<0, 1>(*factories[0], *factories[1]);

  runOramTestWithSecureComponents<util::TestIntp>(*factories[0], *factories[1]);
  runOramTestWithSecureComponents<util::AggregationValue>(
      *factories[0], *factories[1]);
}

template <typename T>
void runLinearOramTestWithSecureComponents(
    engine::communication::IPartyCommunicationAgentFactory& agentFactory0,
    engine::communication::IPartyCommunicationAgentFactory& agentFactory1) {
  auto factory0 = getSecureLinearOramFactory<T, 0>(true, 0, 1, agentFactory0);

  auto factory1 = getSecureLinearOramFactory<T, 1>(false, 0, 1, agentFactory1);

  size_t oramSize = 4;
  testWriteOnlyOram<T>(std::move(factory0), std::move(factory1), oramSize);
}

TEST(LinearORAMTest, TestLinearOram) {
  auto factories = engine::communication::getInMemoryAgentFactory(2);
  setupRealBackend<0, 1>(*factories[0], *factories[1]);

  runLinearOramTestWithSecureComponents<util::TestIntp>(
      *factories[0], *factories[1]);

  runLinearOramTestWithSecureComponents<util::AggregationValue>(
      *factories[0], *factories[1]);
}

} // namespace fbpcf::mpc_std_lib::oram
