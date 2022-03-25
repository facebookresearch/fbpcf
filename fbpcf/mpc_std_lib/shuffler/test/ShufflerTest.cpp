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
#include "fbpcf/mpc_std_lib/util/util.h"
#include "fbpcf/scheduler/SchedulerHelper.h"
#include "fbpcf/test/TestHelper.h"

namespace fbpcf::mpc_std_lib::shuffler {

const int testIntWidth = 32;

template <int schedulerId>
std::vector<std::vector<bool>> task(
    std::unique_ptr<IShuffler<frontend::BitString<true, schedulerId, true>>>
        shuffler,
    const std::vector<std::vector<bool>>& data) {
  frontend::BitString<true, schedulerId, true> bits(data, 0);
  auto shuffled = shuffler->shuffle(bits, data.size());
  auto rst = shuffled.openToParty(0).getValue();
  return rst;
}

void shufflerTest(
    IShufflerFactory<frontend::BitString<true, 0, true>>& shufflerFactory0,
    IShufflerFactory<frontend::BitString<true, 1, true>>& shufflerFactory1) {
  auto agentFactories = engine::communication::getInMemoryAgentFactory(2);
  setupRealBackend<0, 1>(*agentFactories[0], *agentFactories[1]);
  auto shuffler0 = shufflerFactory0.create();
  auto shuffler1 = shufflerFactory1.create();
  size_t size = 16;
  auto data = util::generateRandomPermutation(size);
  std::vector<std::vector<bool>> bitData(size);
  for (size_t i = 0; i < size; i++) {
    bitData[i] = util::Adapters<uint32_t>::convertToBits(data.at(i));
  }
  auto future0 = std::async(task<0>, std::move(shuffler0), bitData);
  auto future1 = std::async(task<1>, std::move(shuffler1), bitData);
  auto bitRst = future0.get();
  future1.get();
  ASSERT_EQ(data.size(), bitRst.size());
  std::vector<uint32_t> rst(size);
  for (size_t i = 0; i < size; i++) {
    rst[i] = util::Adapters<uint32_t>::convertFromBits(bitRst.at(i));
  }

  for (size_t i = 0; i < data.size(); i++) {
    ASSERT_TRUE(std::find(rst.begin(), rst.end(), data.at(i)) != rst.end());
  }
}

TEST(shufflerTest, testNonShuffler) {
  insecure::NonShufflerFactory<frontend::BitString<true, 0, true>> factory0;
  insecure::NonShufflerFactory<frontend::BitString<true, 1, true>> factory1;

  shufflerTest(factory0, factory1);
}

TEST(shufflerTest, testPermuteBasedShufflerWithDummyPermuter) {
  PermuteBasedShufflerFactory<frontend::BitString<true, 0, true>> factory0(
      0,
      1,
      std::make_unique<permuter::insecure::DummyPermuterFactory<
          frontend::BitString<true, 0, true>>>(0, 1),
      std::make_unique<engine::util::AesPrgFactory>());
  PermuteBasedShufflerFactory<frontend::BitString<true, 1, true>> factory1(
      1,
      0,
      std::make_unique<permuter::insecure::DummyPermuterFactory<
          frontend::BitString<true, 1, true>>>(1, 0),
      std::make_unique<engine::util::AesPrgFactory>());

  shufflerTest(factory0, factory1);
}

TEST(shufflerTest, testPermuteBasedShufflerWithAsWaksmanPermuter) {
  PermuteBasedShufflerFactory<frontend::BitString<true, 0, true>> factory0(
      0,
      1,
      std::make_unique<
          permuter::AsWaksmanPermuterFactory<std::vector<bool>, 0>>(0, 1),
      std::make_unique<engine::util::AesPrgFactory>());
  PermuteBasedShufflerFactory<frontend::BitString<true, 1, true>> factory1(
      1,
      0,
      std::make_unique<
          permuter::AsWaksmanPermuterFactory<std::vector<bool>, 1>>(1, 0),
      std::make_unique<engine::util::AesPrgFactory>());

  shufflerTest(factory0, factory1);
}

} // namespace fbpcf::mpc_std_lib::shuffler
