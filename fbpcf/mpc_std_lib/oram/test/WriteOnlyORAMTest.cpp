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
#include "fbpcf/mpc_std_lib/oram/DummySinglePointArrayGenerator.h"
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

struct WritingType {
  std::vector<std::vector<bool>> indexShares;
  std::vector<std::vector<bool>> valueShares;
};

template <typename T>
std::tuple<WritingType, WritingType, std::vector<T>> generateRandomValuesToAdd(
    size_t oramSize,
    size_t batchSize) {
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

template <typename T>
std::pair<std::vector<T>, std::vector<T>> writeOnlyOramHelper(
    std::unique_ptr<IWriteOnlyOramFactory<T>> factory,
    size_t oramSize,
    const WritingType& input) {
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
  size_t batchSize = 16384;

  auto [input0, input1, expectedValue] =
      generateRandomValuesToAdd<T>(oramSize, batchSize);

  auto future0 = std::async(
      writeOnlyOramHelper<T>,
      std::move(factory0),
      oramSize,
      std::reference_wrapper<WritingType>(input0));
  auto future1 = std::async(
      writeOnlyOramHelper<T>,
      std::move(factory1),
      oramSize,
      std::reference_wrapper<WritingType>(input1));

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

  size_t oramSize = 150;
  testWriteOnlyOram<T>(std::move(factory0), std::move(factory1), oramSize);
}

TEST(WriteOnlyORAMTest, TestWriteOnlyORAMWithDummyComponents) {
  runOramTestWithDummyComponents<uint32_t>();
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

  size_t oramSize = 150;
  testWriteOnlyOram<T>(std::move(factory0), std::move(factory1), oramSize);
}

TEST(
    WriteOnlyORAMTest,
    TestWriteOnlyORAMWithRealDifferenceCalculatorAndDummySinglePointArrayGenerator) {
  auto factories = engine::communication::getInMemoryAgentFactory(2);
  setupRealBackend<0, 1>(*factories[0], *factories[1]);

  runOramTestWithRealDifferenceCalculatorAndDummySinglePointArrayGenerator<
      uint32_t>(*factories[0], *factories[1]);
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

  size_t oramSize = 150;
  testWriteOnlyOram<T>(std::move(factory0), std::move(factory1), oramSize);
}

TEST(WriteOnlyORAMTest, TestWriteOnlyORAMWithDummyOblivousDeltaCalculator) {
  auto factories = engine::communication::getInMemoryAgentFactory(2);
  setupRealBackend<0, 1>(*factories[0], *factories[1]);

  runOramTestWithDummyOblivousDeltaCalculator<uint32_t>(
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

  size_t oramSize = 150;
  testWriteOnlyOram<T>(std::move(factory0), std::move(factory1), oramSize);
}

TEST(WriteOnlyORAMTest, TestWriteOnlyORAMWithSecureComponents) {
  auto factories = engine::communication::getInMemoryAgentFactory(2);
  setupRealBackend<0, 1>(*factories[0], *factories[1]);

  runOramTestWithSecureComponents<uint32_t>(*factories[0], *factories[1]);
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

  runLinearOramTestWithSecureComponents<uint32_t>(*factories[0], *factories[1]);

  runLinearOramTestWithSecureComponents<util::AggregationValue>(
      *factories[0], *factories[1]);
}

} // namespace fbpcf::mpc_std_lib::oram
