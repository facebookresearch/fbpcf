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
#include "fbpcf/mpc_std_lib/compactor/DummyCompactor.h"
#include "fbpcf/mpc_std_lib/compactor/DummyCompactorFactory.h"
#include "fbpcf/mpc_std_lib/compactor/ICompactor.h"
#include "fbpcf/mpc_std_lib/compactor/ICompactorFactory.h"
#include "fbpcf/mpc_std_lib/util/test/util.h"
#include "fbpcf/mpc_std_lib/util/util.h"
#include "fbpcf/scheduler/SchedulerHelper.h"
#include "fbpcf/test/TestHelper.h"

namespace fbpcf::mpc_std_lib::compactor {

const int8_t width = 63;
template <int schedulerId>
using SecUnsignedIntBatch = frontend::Integer<
    frontend::Secret<frontend::Batch<frontend::Unsigned<width>>>,
    schedulerId>;
template <int schedulerId>
using SecBitBatch = frontend::Bit<true, schedulerId, true>;

std::vector<bool> generateRandomBinaryVector(size_t size) {
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<int32_t> randomBit(0, 1);

  std::vector<bool> rst(size);
  for (size_t i = 0; i < size; i++) {
    rst[i] = randomBit(e);
  }
  return rst;
}

/*
 * Given a vector, an element i is attributed to a
 * binary label 0/1. Return only elements that are labeled as 1.
 */
std::vector<uint64_t> getOnesValues(
    const std::vector<uint64_t>& src,
    const std::vector<bool>& label) {
  std::vector<uint64_t> rst;
  for (size_t i = 0; i < src.size(); i++) {
    if (label.at(i)) {
      rst.push_back(src[i]);
    }
  }
  return rst;
}

/*
 * It generates metadata and binary labels for inputs to a compaction algorithm
 * and obtain expected results of metadata.
 */
std::tuple<std::vector<uint64_t>, std::vector<bool>, std::vector<uint64_t>>
generateTestData(size_t batchSize) {
  std::vector<uint64_t> testData(batchSize);
  std::iota(testData.begin(), testData.end(), 1);
  auto testLabel = generateRandomBinaryVector(batchSize);
  auto expectedData = getOnesValues(testData, testLabel);

  return {testData, testLabel, expectedData};
}

template <int schedulerId, typename T>
std::tuple<std::vector<T>, std::vector<bool>> task(
    std::unique_ptr<
        ICompactor<SecUnsignedIntBatch<schedulerId>, SecBitBatch<schedulerId>>>
        compactor,
    const std::vector<T>& src,
    const std::vector<bool>& label,
    size_t size,
    bool shouldRevealSize) {
  // generate secret values
  auto secSrc = SecUnsignedIntBatch<schedulerId>(src, 0);
  auto secLabel = SecBitBatch<schedulerId>(label, 0);

  // run a compaction algorithm
  auto [compactifiedSrc, compactifiedLabel] =
      compactor->compaction(secSrc, secLabel, size, shouldRevealSize);

  // get plaintext results
  auto rstSrc = compactifiedSrc.openToParty(0).getValue();
  auto rstLabel = compactifiedLabel.openToParty(0).getValue();

  return {rstSrc, rstLabel};
}

TEST(compactorTest, testDummyCompactor) {
  auto agentFactories = engine::communication::getInMemoryAgentFactory(2);
  setupRealBackend<0, 1>(*agentFactories[0], *agentFactories[1]);

  insecure::DummyCompactorFactory<SecUnsignedIntBatch<0>, SecBitBatch<0>>
      factory0(0, 1);
  insecure::DummyCompactorFactory<SecUnsignedIntBatch<1>, SecBitBatch<1>>
      factory1(1, 0);

  auto compactor0 = factory0.create();
  auto compactor1 = factory1.create();

  size_t batchSize = 5;
  bool shouldRevealSize = true;

  auto [testData, testLabel, expectedData] = generateTestData(batchSize);
  size_t expectedOutputSize = expectedData.size();

  auto future0 = std::async(
      task<0, uint64_t>,
      std::move(compactor0),
      testData,
      testLabel,
      batchSize,
      shouldRevealSize);
  auto future1 = std::async(
      task<1, uint64_t>,
      std::move(compactor1),
      testData,
      testLabel,
      batchSize,
      shouldRevealSize);
  auto [rstData0, rstLabel0] = future0.get();
  future1.get();

  ASSERT_EQ(rstLabel0.size(), expectedOutputSize);
  testVectorEq(rstLabel0, std::vector<bool>(expectedOutputSize, true));
  ASSERT_EQ(rstData0.size(), expectedOutputSize);
  testVectorEq(rstData0, expectedData);
}

} // namespace fbpcf::mpc_std_lib::compactor
