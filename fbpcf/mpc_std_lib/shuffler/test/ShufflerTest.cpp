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
#include <unordered_map>

#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/engine/util/AesPrgFactory.h"
#include "fbpcf/mpc_std_lib/permuter/AsWaksmanPermuterFactory.h"
#include "fbpcf/mpc_std_lib/permuter/DummyPermuterFactory.h"
#include "fbpcf/mpc_std_lib/shuffler/NonShufflerFactory.h"
#include "fbpcf/mpc_std_lib/shuffler/PermuteBasedShufflerFactory.h"
#include "fbpcf/mpc_std_lib/util/test/util.h"
#include "fbpcf/scheduler/SchedulerHelper.h"
#include "fbpcf/test/TestHelper.h"

namespace fbpcf::mpc_std_lib::shuffler {

const int testIntWidth = 32;

template <int schedulerId>
std::vector<uint64_t> task(
    std::unique_ptr<IShuffler<
        frontend::Int<false, testIntWidth, true, schedulerId, true>>> shuffler,
    const std::vector<uint32_t>& data) {
  frontend::Int<false, testIntWidth, true, schedulerId, true> ints(data, 0);
  auto shuffled = shuffler->shuffle(ints, data.size());
  auto rst = shuffled.openToParty(0).getValue();
  return rst;
}

void shufflerTest(
    IShufflerFactory<frontend::Int<false, testIntWidth, true, 0, true>>&
        shufflerFactory0,
    IShufflerFactory<frontend::Int<false, testIntWidth, true, 1, true>>&
        shufflerFactory1) {
  auto agentFactories = engine::communication::getInMemoryAgentFactory(2);
  setupRealBackend<0, 1>(*agentFactories[0], *agentFactories[1]);
  auto shuffler0 = shufflerFactory0.create();
  auto shuffler1 = shufflerFactory1.create();
  size_t size = 16;
  auto data = util::generateRandomPermutation(size);
  auto future0 = std::async(task<0>, std::move(shuffler0), data);
  auto future1 = std::async(task<1>, std::move(shuffler1), data);
  auto rst = future0.get();
  future1.get();
  ASSERT_EQ(data.size(), rst.size());

  for (size_t i = 0; i < data.size(); i++) {
    ASSERT_TRUE(std::find(rst.begin(), rst.end(), data.at(i)) != rst.end());
  }
}

TEST(shufflerTest, testNonShuffler) {
  insecure::NonShufflerFactory<
      frontend::Int<false, testIntWidth, true, 0, true>>
      factory0;
  insecure::NonShufflerFactory<
      frontend::Int<false, testIntWidth, true, 1, true>>
      factory1;

  shufflerTest(factory0, factory1);
}

TEST(shufflerTest, testPermuteBasedShufflerWithDummyPermuter) {
  PermuteBasedShufflerFactory<frontend::Int<false, testIntWidth, true, 0, true>>
      factory0(
          0,
          1,
          std::make_unique<permuter::insecure::DummyPermuterFactory<
              frontend::Int<false, testIntWidth, true, 0, true>>>(0, 1),
          std::make_unique<engine::util::AesPrgFactory>());
  PermuteBasedShufflerFactory<frontend::Int<false, testIntWidth, true, 1, true>>
      factory1(
          1,
          0,
          std::make_unique<permuter::insecure::DummyPermuterFactory<
              frontend::Int<false, testIntWidth, true, 1, true>>>(1, 0),
          std::make_unique<engine::util::AesPrgFactory>());

  shufflerTest(factory0, factory1);
}

TEST(shufflerTest, testPermuteBasedShufflerWithAsWaksmanPermuter) {
  PermuteBasedShufflerFactory<frontend::Int<false, testIntWidth, true, 0, true>>
      factory0(
          0,
          1,
          std::make_unique<permuter::AsWaksmanPermuterFactory<uint32_t, 0>>(
              0, 1),
          std::make_unique<engine::util::AesPrgFactory>());
  PermuteBasedShufflerFactory<frontend::Int<false, testIntWidth, true, 1, true>>
      factory1(
          1,
          0,
          std::make_unique<permuter::AsWaksmanPermuterFactory<uint32_t, 1>>(
              1, 0),
          std::make_unique<engine::util::AesPrgFactory>());

  shufflerTest(factory0, factory1);
}

} // namespace fbpcf::mpc_std_lib::shuffler
