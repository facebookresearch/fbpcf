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
#include "fbpcf/mpc_std_lib/permuter/AsWaksmanPermuter.h"
#include "fbpcf/mpc_std_lib/permuter/AsWaksmanPermuterFactory.h"
#include "fbpcf/mpc_std_lib/permuter/DummyPermuterFactory.h"
#include "fbpcf/mpc_std_lib/util/test/util.h"
#include "fbpcf/scheduler/SchedulerHelper.h"
#include "fbpcf/test/TestHelper.h"

namespace fbpcf::mpc_std_lib::permuter {

std::tuple<std::vector<bool>, std::vector<uint32_t>, std::vector<bool>>
getPermuterTestDataBinary(size_t size) {
  auto originalData = util::generateRandomBinary(size);
  auto order = util::generateRandomPermutation(size);
  std::vector<bool> expectedResult(size);
  for (size_t i = 0; i < size; i++) {
    expectedResult[i] = originalData.at(order.at(i));
  }
  return {originalData, order, expectedResult};
}

std::pair<std::vector<bool>, std::vector<bool>> party0Task(
    std::unique_ptr<IPermuter<frontend::Bit<true, 0, true>>> permuter,
    const std::vector<bool>& data,
    const std::vector<uint32_t>& order) {
  frontend::Bit<true, 0, true> bits(data, 0);
  auto permuted0 = permuter->permute(bits, data.size(), order);
  auto permuted1 = permuter->permute(bits, data.size());
  auto rst0 = permuted0.openToParty(0).getValue();
  auto rst1 = permuted1.openToParty(0).getValue();
  return {rst0, rst1};
}

void party1Task(
    std::unique_ptr<IPermuter<frontend::Bit<true, 1, true>>> permuter,
    const std::vector<bool>& data,
    const std::vector<uint32_t>& order) {
  frontend::Bit<true, 1, true> bits(data, 0);
  auto permuted0 = permuter->permute(bits, data.size());
  auto permuted1 = permuter->permute(bits, data.size(), order);
  permuted0.openToParty(0).getValue();
  permuted1.openToParty(0).getValue();
}

void permuterTestWithEagerScheduler(
    IPermuterFactory<frontend::Bit<true, 0, true>>& permuterFactory0,
    IPermuterFactory<frontend::Bit<true, 1, true>>& permuterFactory1) {
  auto agentFactories = engine::communication::getInMemoryAgentFactory(2);
  setupRealBackend<0, 1>(*agentFactories[0], *agentFactories[1]);
  auto permuter0 = permuterFactory0.create();
  auto permuter1 = permuterFactory1.create();
  size_t size = 17;
  auto [originalData, order, expectedOutput] = getPermuterTestDataBinary(size);
  auto future0 =
      std::async(party0Task, std::move(permuter0), originalData, order);
  auto future1 =
      std::async(party1Task, std::move(permuter1), originalData, order);
  auto [rst0, rst1] = future0.get();
  future1.get();
  testVectorEq(rst0, expectedOutput);
  testVectorEq(rst1, expectedOutput);
}

void permuterTestWithLazyScheduler(
    IPermuterFactory<frontend::Bit<true, 0, true>>& permuterFactory0,
    IPermuterFactory<frontend::Bit<true, 1, true>>& permuterFactory1) {
  auto agentFactories = engine::communication::getInMemoryAgentFactory(2);
  setupRealBackendWithLazyScheduler<0, 1>(
      *agentFactories[0], *agentFactories[1]);
  auto permuter0 = permuterFactory0.create();
  auto permuter1 = permuterFactory1.create();

  size_t size = 17;
  auto [originalData, order, expectedOutput] = getPermuterTestDataBinary(size);

  auto future0 =
      std::async(party0Task, std::move(permuter0), originalData, order);
  auto future1 =
      std::async(party1Task, std::move(permuter1), originalData, order);
  auto [rst0, rst1] = future0.get();
  future1.get();
  testVectorEq(rst0, expectedOutput);
  testVectorEq(rst1, expectedOutput);
}

TEST(permuterTestBit, testDummyPermuter) {
  insecure::DummyPermuterFactory<bool, 0> factory0(0, 1);
  insecure::DummyPermuterFactory<bool, 1> factory1(1, 0);

  permuterTestWithEagerScheduler(factory0, factory1);
  permuterTestWithLazyScheduler(factory0, factory1);
}

TEST(permuterTestBit, testAsWaksmanPermuter) {
  AsWaksmanPermuterFactory<bool, 0> factory0(0, 1);
  AsWaksmanPermuterFactory<bool, 1> factory1(1, 0);

  permuterTestWithEagerScheduler(factory0, factory1);
  permuterTestWithLazyScheduler(factory0, factory1);
}

} // namespace fbpcf::mpc_std_lib::permuter
