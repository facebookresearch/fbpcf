/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <cmath>
#include <future>
#include <memory>
#include <random>

#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/engine/util/AesPrgFactory.h"
#include "fbpcf/mpc_std_lib/permuter/AsWaksmanPermuterFactory.h"
#include "fbpcf/mpc_std_lib/permuter/DummyPermuterFactory.h"
#include "fbpcf/mpc_std_lib/shuffler/NonShufflerFactory.h"
#include "fbpcf/mpc_std_lib/shuffler/PermuteBasedShufflerFactory.h"
#include "fbpcf/mpc_std_lib/util/test/util.h"
#include "fbpcf/mpc_std_lib/util/util.h"
#include "fbpcf/scheduler/SchedulerHelper.h"
#include "fbpcf/test/TestHelper.h"

namespace fbpcf::mpc_std_lib::shuffler {

/*
 * We run unit tests on a pair type of std::pair<uint32_t, bool>.
 */

/*
 * Generate a vector of pairs, where each pair is composed of an id-value
 * (uint32_t) and its true/false label. The i-th pair has an id value of i. This
 * is used as a running example to test shuffler on a specific input type of
 * std::pair<uint32_t, bool> .
 */
std::vector<std::pair<uint32_t, bool>> generateTestData(size_t size) {
  std::vector<uint32_t> uint32Vec(size);
  std::iota(
      uint32Vec.begin(),
      uint32Vec.end(),
      0); // generate unique values starting from 0.
  auto binaryVec = util::generateRandomBinary(size);

  std::vector<std::pair<uint32_t, bool>> rst(size);
  for (size_t i = 0; i < size; i++) {
    rst[i] = {uint32Vec.at(i), binaryVec.at(i)};
  }
  return rst;
}

template <int schedulerId>
std::vector<std::pair<uint32_t, bool>> task(
    std::unique_ptr<IShuffler<std::pair<
        frontend::Int<false, 32, true, schedulerId, true>,
        frontend::Bit<true, schedulerId, true>>>> shuffler,
    const std::vector<std::pair<uint32_t, bool>>& data) {
  auto secretData = util::MpcAdapters<std::pair<uint32_t, bool>, schedulerId>::
      processSecretInputs(data, 0);
  auto shuffled = shuffler->shuffle(secretData, data.size());
  auto rst =
      util::MpcAdapters<std::pair<uint32_t, bool>, schedulerId>::openToParty(
          shuffled, 0);
  return rst;
}

void shufflerTest(
    IShufflerFactory<std::pair<
        frontend::Int<false, 32, true, 0, true>,
        frontend::Bit<true, 0, true>>>& shufflerFactory0,
    IShufflerFactory<std::pair<
        frontend::Int<false, 32, true, 1, true>,
        frontend::Bit<true, 1, true>>>& shufflerFactory1) {
  auto agentFactories = engine::communication::getInMemoryAgentFactory(2);
  setupRealBackend<0, 1>(*agentFactories[0], *agentFactories[1]);
  auto shuffler0 = shufflerFactory0.create();
  auto shuffler1 = shufflerFactory1.create();
  size_t size = 16;
  auto data = generateTestData(size);

  auto future0 = std::async(task<0>, std::move(shuffler0), data);
  auto future1 = std::async(task<1>, std::move(shuffler1), data);
  auto rst = future0.get();
  future1.get();

  ASSERT_EQ(rst.size(), size);
  for (size_t i = 0; i < data.size(); i++) {
    auto index = rst.at(i).first;
    // check consistency of each pair
    ASSERT_EQ(rst.at(i).first, data.at(index).first);
    ASSERT_EQ(rst.at(i).second, data.at(index).second);
  }
}

TEST(shufflerTestPair, testNonShuffler) {
  insecure::NonShufflerFactory<std::pair<
      frontend::Int<false, 32, true, 0, true>,
      frontend::Bit<true, 0, true>>>
      factory0;
  insecure::NonShufflerFactory<std::pair<
      frontend::Int<false, 32, true, 1, true>,
      frontend::Bit<true, 1, true>>>
      factory1;

  shufflerTest(factory0, factory1);
}

TEST(shufflerTestPair, testPermuteBasedShufflerWithDummyPermuter) {
  PermuteBasedShufflerFactory<std::pair<
      frontend::Int<false, 32, true, 0, true>,
      frontend::Bit<true, 0, true>>>
      factory0(
          0,
          1,
          std::make_unique<permuter::insecure::DummyPermuterFactory<
              std::pair<uint32_t, bool>,
              0>>(0, 1),
          std::make_unique<engine::util::AesPrgFactory>());
  PermuteBasedShufflerFactory<std::pair<
      frontend::Int<false, 32, true, 1, true>,
      frontend::Bit<true, 1, true>>>
      factory1(
          1,
          0,
          std::make_unique<permuter::insecure::DummyPermuterFactory<
              std::pair<uint32_t, bool>,
              1>>(1, 0),
          std::make_unique<engine::util::AesPrgFactory>());

  shufflerTest(factory0, factory1);
}

TEST(shufflerTestPair, testPermuteBasedShufflerWithAsWaksmanPermuter) {
  PermuteBasedShufflerFactory<std::pair<
      frontend::Int<false, 32, true, 0, true>,
      frontend::Bit<true, 0, true>>>
      factory0(
          0,
          1,
          std::make_unique<
              permuter::AsWaksmanPermuterFactory<std::pair<uint32_t, bool>, 0>>(
              0, 1),
          std::make_unique<engine::util::AesPrgFactory>());
  PermuteBasedShufflerFactory<std::pair<
      frontend::Int<false, 32, true, 1, true>,
      frontend::Bit<true, 1, true>>>
      factory1(
          1,
          0,
          std::make_unique<
              permuter::AsWaksmanPermuterFactory<std::pair<uint32_t, bool>, 1>>(
              1, 0),
          std::make_unique<engine::util::AesPrgFactory>());

  shufflerTest(factory0, factory1);
}

} // namespace fbpcf::mpc_std_lib::shuffler
