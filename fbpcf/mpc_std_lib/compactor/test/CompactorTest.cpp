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
#include <cstdint>
#include <future>
#include <memory>
#include <random>

#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/engine/util/AesPrgFactory.h"
#include "fbpcf/mpc_std_lib/compactor/DummyCompactor.h"
#include "fbpcf/mpc_std_lib/compactor/DummyCompactorFactory.h"
#include "fbpcf/mpc_std_lib/compactor/ICompactor.h"
#include "fbpcf/mpc_std_lib/compactor/ICompactorFactory.h"
#include "fbpcf/mpc_std_lib/compactor/ShuffleBasedCompactor.h"
#include "fbpcf/mpc_std_lib/compactor/ShuffleBasedCompactorFactory.h"
#include "fbpcf/mpc_std_lib/permuter/AsWaksmanPermuterFactory.h"
#include "fbpcf/mpc_std_lib/shuffler/NonShufflerFactory.h"
#include "fbpcf/mpc_std_lib/shuffler/PermuteBasedShufflerFactory.h"
#include "fbpcf/mpc_std_lib/util/test/util.h"
#include "fbpcf/mpc_std_lib/util/util.h"
#include "fbpcf/scheduler/SchedulerHelper.h"
#include "fbpcf/test/TestHelper.h"

namespace fbpcf::mpc_std_lib::compactor {

const int8_t adIdWidth = 64;
const int8_t convWidth = 32;

using AttributionValue =
    std::pair<util::Intp<false, adIdWidth>, util::Intp<false, convWidth>>;

/* Generate metadata (a pair of unsigned int 64-bits values and 32-bits values)
 * and binary labels; then compute expected metadata output  */
std::tuple<
    std::vector<AttributionValue>,
    std::vector<bool>,
    std::vector<AttributionValue>>
generateData(size_t size) {
  std::vector<AttributionValue> data(size);
  for (size_t i = 0; i < size; i++) {
    data[i] = AttributionValue({i, i});
  }

  auto label = util::generateRandomBinary(size);

  std::vector<AttributionValue> expectedData;
  for (size_t i = 0; i < size; i++) {
    if (label.at(i)) {
      expectedData.push_back(data.at(i));
    }
  }
  return {data, label, expectedData};
}

template <typename T, int schedulerId>
std::pair<std::vector<T>, std::vector<bool>> task(
    std::unique_ptr<ICompactor<
        typename util::SecBatchType<T, schedulerId>::type,
        typename util::SecBatchType<bool, schedulerId>::type>> compactor,
    const std::vector<T>& value,
    const std::vector<bool>& label,
    uint32_t size,
    bool shouldRevealSize) {
  auto secValue =
      util::MpcAdapters<T, schedulerId>::processSecretInputs(value, 0);
  auto secLabel =
      util::MpcAdapters<bool, schedulerId>::processSecretInputs(label, 0);
  auto [compactifiedValue, compactifiedLabel] =
      compactor->compaction(secValue, secLabel, size, shouldRevealSize);
  auto rstLabel =
      util::MpcAdapters<bool, schedulerId>::openToParty(compactifiedLabel, 0);
  auto rstValue =
      util::MpcAdapters<T, schedulerId>::openToParty(compactifiedValue, 0);

  return {rstValue, rstLabel};
}

template <typename T>
void compactorTest(
    ICompactorFactory<
        typename util::SecBatchType<T, 0>::type,
        typename util::SecBatchType<bool, 0>::type>& compactorFactory0,
    ICompactorFactory<
        typename util::SecBatchType<T, 1>::type,
        typename util::SecBatchType<bool, 1>::type>& compactorFactory1) {
  auto agentFactories = engine::communication::getInMemoryAgentFactory(2);
  setupRealBackend<0, 1>(*agentFactories[0], *agentFactories[1]);
  auto compactor0 = compactorFactory0.create();
  auto compactor1 = compactorFactory1.create();

  uint32_t batchSize = 11;
  bool shouldRevealSize = true;

  // Generate test data
  auto [testData, testLabel, expectedData] = generateData(batchSize);
  size_t expectedOutputSize = expectedData.size();

  auto future0 = std::async(
      task<T, 0>,
      std::move(compactor0),
      testData,
      testLabel,
      batchSize,
      shouldRevealSize);
  auto future1 = std::async(
      task<T, 1>,
      std::move(compactor1),
      testData,
      testLabel,
      batchSize,
      shouldRevealSize);
  auto [rstData0, rstLabel0] = future0.get();

  ASSERT_EQ(rstLabel0.size(), expectedOutputSize);
  testVectorEq(rstLabel0, std::vector<bool>(expectedOutputSize, true));
  ASSERT_EQ(rstData0.size(), expectedOutputSize);

  for (size_t j = 0; j < expectedOutputSize; j++) {
    /* check consistency of each record */
    auto index = rstData0.at(j).first;
    ASSERT_EQ(rstData0.at(j).first, testData.at(index).first);
    ASSERT_EQ(rstData0.at(j).second, testData.at(index).second);
  }
}

TEST(compactorTest, testDummyCompactor) {
  insecure::DummyCompactorFactory<AttributionValue, bool, 0> factory0(0, 1);
  insecure::DummyCompactorFactory<AttributionValue, bool, 1> factory1(1, 0);
  compactorTest<AttributionValue>(factory0, factory1);
}

TEST(compactorTest, testNonShufflerBasedCompactor) {
  ShuffleBasedCompactorFactory<AttributionValue, bool, 0> factory0(
      0,
      1,
      std::make_unique<shuffler::insecure::NonShufflerFactory<std::pair<
          typename util::SecBatchType<AttributionValue, 0>::type,
          typename util::SecBatchType<bool, 0>::type>>>());
  ShuffleBasedCompactorFactory<AttributionValue, bool, 1> factory1(
      1,
      0,
      std::make_unique<shuffler::insecure::NonShufflerFactory<std::pair<
          typename util::SecBatchType<AttributionValue, 1>::type,
          typename util::SecBatchType<bool, 1>::type>>>());

  compactorTest<AttributionValue>(factory0, factory1);
}

TEST(compactorTest, testShuffleBasedCompactor) {
  ShuffleBasedCompactorFactory<AttributionValue, bool, 0> factory0(
      0,
      1,
      std::make_unique<shuffler::PermuteBasedShufflerFactory<std::pair<
          typename util::SecBatchType<AttributionValue, 0>::type,
          typename util::SecBatchType<bool, 0>::type>>>(
          0,
          1,
          std::make_unique<permuter::AsWaksmanPermuterFactory<
              std::pair<AttributionValue, bool>,
              0>>(0, 1),
          std::make_unique<engine::util::AesPrgFactory>()));
  ShuffleBasedCompactorFactory<AttributionValue, bool, 1> factory1(
      1,
      0,
      std::make_unique<shuffler::PermuteBasedShufflerFactory<std::pair<
          typename util::SecBatchType<AttributionValue, 1>::type,
          typename util::SecBatchType<bool, 1>::type>>>(
          1,
          0,
          std::make_unique<permuter::AsWaksmanPermuterFactory<
              std::pair<AttributionValue, bool>,
              1>>(1, 0),
          std::make_unique<engine::util::AesPrgFactory>()));

  compactorTest<AttributionValue>(factory0, factory1);
}

} // namespace fbpcf::mpc_std_lib::compactor
