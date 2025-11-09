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
#include <memory>
#include <random>

#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/mpc_std_lib/oram/DummyObliviousDeltaCalculatorFactory.h"
#include "fbpcf/mpc_std_lib/oram/DummySinglePointArrayGeneratorFactory.h"
#include "fbpcf/mpc_std_lib/oram/ISinglePointArrayGenerator.h"
#include "fbpcf/mpc_std_lib/oram/ISinglePointArrayGeneratorFactory.h"
#include "fbpcf/mpc_std_lib/oram/ObliviousDeltaCalculatorFactory.h"
#include "fbpcf/mpc_std_lib/oram/SinglePointArrayGeneratorFactory.h"
#include "fbpcf/mpc_std_lib/oram/test/util.h"

namespace fbpcf::mpc_std_lib::oram {

std::vector<std::pair<std::vector<bool>, std::vector<__m128i>>>
generateSinglePointArrayHelper(
    std::unique_ptr<ISinglePointArrayGeneratorFactory> factory,
    std::reference_wrapper<std::vector<std::vector<bool>>> indexShares,
    size_t length) {
  auto generator = factory->create();
  return generator->generateSinglePointArrays(indexShares, length);
}

void testSinglePointArrayGenerator(
    std::unique_ptr<ISinglePointArrayGeneratorFactory> factory0,
    std::unique_ptr<ISinglePointArrayGeneratorFactory> factory1) {
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint32_t> dist(8192, 16384);

  size_t length = dist(e);
  size_t width = std::ceil(std::log2(length));
  size_t batchSize = 128;

  std::vector<std::vector<bool>> party0Input(
      width, std::vector<bool>(batchSize));
  std::vector<std::vector<bool>> party1Input(
      width, std::vector<bool>(batchSize));
  std::vector<uint32_t> expectedIndex;
  for (size_t i = 0; i < batchSize; i++) {
    auto [share0, share1, trueValue] =
        util::generateSharedRandomBoolVectorForSinglePointArrayGenerator(
            length);
    for (size_t j = 0; j < width; j++) {
      party0Input[j][i] = share0.at(j);
      party1Input[j][i] = share1.at(j);
    }
    expectedIndex.push_back(trueValue);
  }
  auto future0 = std::async(
      generateSinglePointArrayHelper,
      std::move(factory0),
      std::reference_wrapper<std::vector<std::vector<bool>>>(party0Input),
      length);
  auto future1 = std::async(
      generateSinglePointArrayHelper,
      std::move(factory1),
      std::reference_wrapper<std::vector<std::vector<bool>>>(party1Input),
      length);

  auto result0 = future0.get();
  auto result1 = future1.get();

  for (size_t i = 0; i < batchSize; i++) {
    for (int j = 0; j < length; j++) {
      if (j != expectedIndex[i]) {
        EXPECT_EQ(result0.at(i).first.at(j), result1.at(i).first.at(j));
        EXPECT_TRUE(compareM128i(
            result0.at(i).second.at(j), result1.at(i).second.at(j)));
      } else {
        EXPECT_NE(result0.at(i).first.at(j), result1.at(i).first.at(j));
        EXPECT_FALSE(compareM128i(
            result0.at(i).second.at(j), result1.at(i).second.at(j)));
      }
    }
  }
}

TEST(SinglePointArrayGeneratorTest, testDummySinglePointArrayGenerator) {
  auto factories = engine::communication::getInMemoryAgentFactory(2);
  auto factory0 =
      std::make_unique<insecure::DummySinglePointArrayGeneratorFactory>(
          true, 1, *factories[0]);
  auto factory1 =
      std::make_unique<insecure::DummySinglePointArrayGeneratorFactory>(
          false, 0, *factories[1]);
  testSinglePointArrayGenerator(std::move(factory0), std::move(factory1));
}

TEST(
    SinglePointArrayGeneratorTest,
    testSinglePointArrayGeneratorWithDummyObliviousDeltaCalculator) {
  auto factories = engine::communication::getInMemoryAgentFactory(2);
  auto factory0 = std::make_unique<SinglePointArrayGeneratorFactory>(
      true,
      std::make_unique<insecure::DummyObliviousDeltaCalculatorFactory>(
          1, *factories[0]));
  auto factory1 = std::make_unique<SinglePointArrayGeneratorFactory>(
      false,
      std::make_unique<insecure::DummyObliviousDeltaCalculatorFactory>(
          0, *factories[1]));
  testSinglePointArrayGenerator(std::move(factory0), std::move(factory1));
}

TEST(
    SinglePointArrayGeneratorTest,
    testSinglePointArrayGeneratorWithObliviousDeltaCalculator) {
  auto factories = engine::communication::getInMemoryAgentFactory(2);
  setupRealBackend<0, 1>(*factories[0], *factories[1]);

  auto factory0 = std::make_unique<SinglePointArrayGeneratorFactory>(
      true, std::make_unique<ObliviousDeltaCalculatorFactory<0>>(true, 0, 1));
  auto factory1 = std::make_unique<SinglePointArrayGeneratorFactory>(
      false, std::make_unique<ObliviousDeltaCalculatorFactory<1>>(false, 0, 1));
  testSinglePointArrayGenerator(std::move(factory0), std::move(factory1));
}

} // namespace fbpcf::mpc_std_lib::oram
