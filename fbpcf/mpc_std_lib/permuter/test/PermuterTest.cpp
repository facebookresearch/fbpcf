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
#include "fbpcf/mpc_std_lib/permuter/DummyPermuterFactory.h"
#include "fbpcf/mpc_std_lib/util/test/util.h"
#include "fbpcf/scheduler/SchedulerHelper.h"
#include "fbpcf/test/TestHelper.h"

namespace fbpcf::mpc_std_lib::permuter {

std::tuple<std::vector<uint32_t>, std::vector<uint32_t>, std::vector<uint64_t>>
getPermuterTestData(size_t size) {
  auto originalData = util::generateRandomPermutation(size);
  auto order = util::generateRandomPermutation(size);
  std::vector<uint64_t> expectedResult(size);
  for (size_t i = 0; i < size; i++) {
    expectedResult[i] = originalData.at(order.at(i));
  }
  return {originalData, order, expectedResult};
}

const int kTestIntWidth = 22;

std::pair<std::vector<uint64_t>, std::vector<uint64_t>> party0Task(
    std::unique_ptr<
        IPermuter<frontend::Int<false, kTestIntWidth, true, 0, true>>> permuter,
    const std::vector<uint32_t>& data,
    const std::vector<uint32_t>& order) {
  frontend::Int<false, kTestIntWidth, true, 0, true> ints(data, 0);
  auto permuted0 = permuter->permute(ints, data.size(), order);
  auto permuted1 = permuter->permute(ints, data.size());
  auto rst0 = permuted0.openToParty(0).getValue();
  auto rst1 = permuted1.openToParty(0).getValue();
  return {rst0, rst1};
}

void party1Task(
    std::unique_ptr<
        IPermuter<frontend::Int<false, kTestIntWidth, true, 1, true>>> permuter,
    const std::vector<uint32_t>& data,
    const std::vector<uint32_t>& order) {
  frontend::Int<false, kTestIntWidth, true, 1, true> ints(data, 0);
  auto permuted0 = permuter->permute(ints, data.size());
  auto permuted1 = permuter->permute(ints, data.size(), order);
  permuted0.openToParty(0);
  permuted1.openToParty(0);
}

void permuterTest(
    IPermuterFactory<frontend::Int<false, kTestIntWidth, true, 0, true>>&
        permuterFactory0,
    IPermuterFactory<frontend::Int<false, kTestIntWidth, true, 1, true>>&
        permuterFactory1) {
  auto agentFactories = engine::communication::getInMemoryAgentFactory(2);
  setupRealBackend<0, 1>(*agentFactories[0], *agentFactories[1]);
  auto permuter0 = permuterFactory0.create();
  auto permuter1 = permuterFactory1.create();
  size_t size = 16;
  auto [originalData, order, expectedOutput] = getPermuterTestData(size);
  auto future0 =
      std::async(party0Task, std::move(permuter0), originalData, order);
  auto future1 =
      std::async(party1Task, std::move(permuter1), originalData, order);
  auto [rst0, rst1] = future0.get();
  future1.get();
  testVectorEq(rst0, expectedOutput);
  testVectorEq(rst1, expectedOutput);
}

TEST(permuterTest, testDummyPermuter) {
  insecure::DummyPermuterFactory<
      frontend::Int<false, kTestIntWidth, true, 0, true>>
      factory0(0, 1);
  insecure::DummyPermuterFactory<
      frontend::Int<false, kTestIntWidth, true, 1, true>>
      factory1(1, 0);

  permuterTest(factory0, factory1);
}

} // namespace fbpcf::mpc_std_lib::permuter
