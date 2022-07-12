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
#include <unordered_map>

#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/engine/util/AesPrgFactory.h"
#include "fbpcf/mpc_std_lib/compactor/DummyCompactor.h"
#include "fbpcf/mpc_std_lib/compactor/DummyCompactorFactory.h"
#include "fbpcf/mpc_std_lib/compactor/ICompactor.h"
#include "fbpcf/mpc_std_lib/compactor/ICompactorFactory.h"
#include "fbpcf/mpc_std_lib/compactor/ShuffleBasedCompactor.h"
#include "fbpcf/mpc_std_lib/compactor/ShuffleBasedCompactorFactory.h"
#include "fbpcf/mpc_std_lib/permuter/AsWaksmanPermuterFactory.h"
#include "fbpcf/mpc_std_lib/shuffler/PermuteBasedShufflerFactory.h"
#include "fbpcf/mpc_std_lib/util/test/util.h"
#include "fbpcf/mpc_std_lib/util/util.h"
#include "fbpcf/scheduler/SchedulerHelper.h"
#include "fbpcf/test/TestHelper.h"

namespace fbpcf::mpc_std_lib::compactor {

template <int schedulerId>
using SecUnsignedIntBatch = frontend::Int<false, 32, true, schedulerId, true>;
template <int schedulerId>
using SecBitBatch = frontend::Bit<true, schedulerId, true>;

/* Generate metadata (unsigned int 32-bits values) and binary labels; then
 * compute expected metadata output  */
std::tuple<std::vector<uint32_t>, std::vector<bool>, std::vector<uint32_t>>
generateData(size_t size) {
  std::vector<uint32_t> data(size);
  std::iota(
      data.begin(), data.end(), 0); // assign unique values starting from 0.

  auto label = util::generateRandomBinary(size);

  std::vector<uint32_t> expectedData;
  for (size_t i = 0; i < size; i++) {
    if (label.at(i)) {
      expectedData.push_back(data.at(i));
    }
  }
  return {data, label, expectedData};
}

template <int schedulerId>
std::pair<std::vector<uint32_t>, std::vector<bool>> task(
    std::unique_ptr<
        ICompactor<SecUnsignedIntBatch<schedulerId>, SecBitBatch<schedulerId>>>
        compactor,
    const std::vector<uint32_t>& value,
    const std::vector<bool>& label,
    size_t size,
    bool shouldRevealSize) {
  auto secValue =
      util::MpcAdapters<uint32_t, schedulerId>::processSecretInputs(value, 0);
  auto secLabel =
      util::MpcAdapters<bool, schedulerId>::processSecretInputs(label, 0);
  auto [compactifiedValue, compactifiedLabel] =
      compactor->compaction(secValue, secLabel, size, shouldRevealSize);
  auto rstLabel =
      util::MpcAdapters<bool, schedulerId>::openToParty(compactifiedLabel, 0);
  auto rstValue = util::MpcAdapters<uint32_t, schedulerId>::openToParty(
      compactifiedValue, 0);

  return {rstValue, rstLabel};
}

void compactorTest(
    ICompactorFactory<
        frontend::Int<false, 32, true, 0, true>,
        frontend::Bit<true, 0, true>>& compactorFactory0,
    ICompactorFactory<
        frontend::Int<false, 32, true, 1, true>,
        frontend::Bit<true, 1, true>>& compactorFactory1) {
  auto agentFactories = engine::communication::getInMemoryAgentFactory(2);
  setupRealBackend<0, 1>(*agentFactories[0], *agentFactories[1]);
  auto compactor0 = compactorFactory0.create();
  auto compactor1 = compactorFactory1.create();

  size_t batchSize = 11;
  bool shouldRevealSize = true;

  // Generate test data
  auto [testData, testLabel, expectedData] = generateData(batchSize);
  size_t expectedOutputSize = expectedData.size();

  auto future0 = std::async(
      task<0>,
      std::move(compactor0),
      testData,
      testLabel,
      batchSize,
      shouldRevealSize);
  auto future1 = std::async(
      task<1>,
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
    auto index = rstData0.at(j);
    ASSERT_EQ(rstData0.at(j), testData.at(index));
  }
}

TEST(compactorTest, testDummyCompactor) {
  insecure::DummyCompactorFactory<uint32_t, bool, 0> factory0(0, 1);
  insecure::DummyCompactorFactory<uint32_t, bool, 1> factory1(1, 0);
  compactorTest(factory0, factory1);
}

TEST(compactorTest, testShuffleBasedCompactor) {
  ShuffleBasedCompactorFactory<uint32_t, bool, 0> factory0(
      0,
      1,
      std::make_unique<shuffler::PermuteBasedShufflerFactory<std::pair<
          frontend::Int<false, 32, true, 0, true>,
          frontend::Bit<true, 0, true>>>>(
          0,
          1,
          std::make_unique<
              permuter::AsWaksmanPermuterFactory<std::pair<uint32_t, bool>, 0>>(
              0, 1),
          std::make_unique<engine::util::AesPrgFactory>()));
  ShuffleBasedCompactorFactory<uint32_t, bool, 1> factory1(
      1,
      0,
      std::make_unique<shuffler::PermuteBasedShufflerFactory<std::pair<
          frontend::Int<false, 32, true, 1, true>,
          frontend::Bit<true, 1, true>>>>(
          1,
          0,
          std::make_unique<
              permuter::AsWaksmanPermuterFactory<std::pair<uint32_t, bool>, 1>>(
              1, 0),
          std::make_unique<engine::util::AesPrgFactory>()));

  compactorTest(factory0, factory1);
}

} // namespace fbpcf::mpc_std_lib::compactor
