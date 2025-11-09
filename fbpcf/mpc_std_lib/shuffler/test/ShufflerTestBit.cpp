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
#include "fbpcf/scheduler/SchedulerHelper.h"
#include "fbpcf/test/TestHelper.h"

namespace fbpcf::mpc_std_lib::shuffler {

template <int schedulerId>
std::vector<bool> task(
    std::unique_ptr<IShuffler<frontend::Bit<true, schedulerId, true>>> shuffler,
    const std::vector<bool>& data) {
  frontend::Bit<true, schedulerId, true> bits(data, 0);
  auto shuffled = shuffler->shuffle(bits, data.size());
  auto rst = shuffled.openToParty(0).getValue();
  return rst;
}

void shufflerTest(
    IShufflerFactory<frontend::Bit<true, 0, true>>& shufflerFactory0,
    IShufflerFactory<frontend::Bit<true, 1, true>>& shufflerFactory1) {
  auto agentFactories = engine::communication::getInMemoryAgentFactory(2);
  setupRealBackend<0, 1>(*agentFactories[0], *agentFactories[1]);
  auto shuffler0 = shufflerFactory0.create();
  auto shuffler1 = shufflerFactory1.create();
  size_t size = 16;
  auto data = util::generateRandomBinary(size);

  auto future0 = std::async(task<0>, std::move(shuffler0), data);
  auto future1 = std::async(task<1>, std::move(shuffler1), data);
  auto rst = future0.get();
  future1.get();
  ASSERT_EQ(data.size(), rst.size());
  size_t numZeros = 0;
  size_t numOnes = 0;
  size_t expectedNumZeros = 0;
  size_t expectedNumOnes = 0;
  for (size_t i = 0; i < data.size(); i++) {
    if (rst.at(i)) {
      numOnes++;
    } else {
      numZeros++;
    }
    if (data.at(i)) {
      expectedNumOnes++;
    } else {
      expectedNumZeros++;
    }
  }
  ASSERT_EQ(numOnes, expectedNumOnes);
  ASSERT_EQ(numZeros, expectedNumZeros);
}

TEST(shufflerTestBit, testNonShuffler) {
  insecure::NonShufflerFactory<frontend::Bit<true, 0, true>> factory0;
  insecure::NonShufflerFactory<frontend::Bit<true, 1, true>> factory1;

  shufflerTest(factory0, factory1);
}

TEST(shufflerTestBit, testPermuteBasedShufflerWithDummyPermuter) {
  PermuteBasedShufflerFactory<frontend::Bit<true, 0, true>> factory0(
      0,
      1,
      std::make_unique<permuter::insecure::DummyPermuterFactory<bool, 0>>(0, 1),
      std::make_unique<engine::util::AesPrgFactory>());
  PermuteBasedShufflerFactory<frontend::Bit<true, 1, true>> factory1(
      1,
      0,
      std::make_unique<permuter::insecure::DummyPermuterFactory<bool, 1>>(1, 0),
      std::make_unique<engine::util::AesPrgFactory>());

  shufflerTest(factory0, factory1);
}

TEST(shufflerTestBit, testPermuteBasedShufflerWithAsWaksmanPermuter) {
  PermuteBasedShufflerFactory<frontend::Bit<true, 0, true>> factory0(
      0,
      1,
      std::make_unique<permuter::AsWaksmanPermuterFactory<bool, 0>>(0, 1),
      std::make_unique<engine::util::AesPrgFactory>());
  PermuteBasedShufflerFactory<frontend::Bit<true, 1, true>> factory1(
      1,
      0,
      std::make_unique<permuter::AsWaksmanPermuterFactory<bool, 1>>(1, 0),
      std::make_unique<engine::util::AesPrgFactory>());

  shufflerTest(factory0, factory1);
}

} // namespace fbpcf::mpc_std_lib::shuffler
