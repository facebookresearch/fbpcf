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
#include "fbpcf/mpc_std_lib/permuter/AsWaksmanPermuter.h"
#include "fbpcf/mpc_std_lib/permuter/AsWaksmanPermuterFactory.h"
#include "fbpcf/mpc_std_lib/permuter/DummyPermuterFactory.h"
#include "fbpcf/mpc_std_lib/util/test/util.h"
#include "fbpcf/scheduler/SchedulerHelper.h"
#include "fbpcf/test/TestHelper.h"

namespace fbpcf::mpc_std_lib::permuter {

std::tuple<std::vector<uint32_t>, std::vector<uint32_t>, std::vector<uint64_t>>
getPermuterTestData(size_t size) {
  auto originalData = util::generateRandomPermutation(size);
  auto order = util::generateRandomPermutation(size);
  auto tmp = order[1];
  order[1] = order[0];
  order[0] = tmp;
  std::vector<uint64_t> expectedResult(size);
  for (size_t i = 0; i < size; i++) {
    expectedResult[i] = originalData.at(order.at(i));
  }
  return {originalData, order, expectedResult};
}

const int kTestIntWidth = 32;

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
  size_t size = 17;
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

TEST(permuterTest, testAsWaksmanPermuter) {
  AsWaksmanPermuterFactory<uint32_t, 0> factory0(0, 1);
  AsWaksmanPermuterFactory<uint32_t, 1> factory1(1, 0);

  permuterTest(factory0, factory1);
}

void testAsWaksmanParameter() {
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint32_t> randomSize(2, 0xFFF);

  size_t size = randomSize(e);
  auto order = util::generateRandomPermutation(size);
  AsWaksmanParameterCalculator calculator(order);
  std::vector<uint32_t> testData(size);
  for (size_t i = 0; i < size; i++) {
    testData[i] = i;
  }
  std::vector<uint32_t> firstHalf(size / 2);
  std::vector<uint32_t> secondHalf(size - size / 2);

  auto firstSwapConditions = calculator.getFirstSwapConditions();
  ASSERT_EQ(firstSwapConditions.size(), size / 2);
  for (size_t i = 0; i < size / 2; i++) {
    firstHalf[i] = testData.at(i);
    secondHalf[i] = testData.at(i + size / 2);
    if (firstSwapConditions.at(i)) {
      auto tmp = secondHalf.at(i);
      secondHalf[i] = firstHalf.at(i);
      firstHalf[i] = tmp;
    }
  }

  if ((size & 1) == 1) {
    secondHalf[size / 2] = testData.at(size - 1);
  }

  std::vector<uint32_t> firstHalfAfterPermute(size / 2);
  auto subOrderFirst = calculator.getFirstSubPermuteOrder();
  for (size_t i = 0; i < size / 2; i++) {
    firstHalfAfterPermute[i] = firstHalf.at(subOrderFirst.at(i));
  }
  std::vector<uint32_t> secondHalfAfterPermute(size - size / 2);
  auto subOrderSecond = calculator.getSecondSubPermuteOrder();
  for (size_t i = 0; i < size - size / 2; i++) {
    secondHalfAfterPermute[i] = secondHalf.at(subOrderSecond.at(i));
  }

  auto secondSwapConditions = calculator.getSecondSwapConditions();
  ASSERT_EQ(secondSwapConditions.size(), (size - 1) / 2);
  for (size_t i = 0; i < (size - 1) / 2; i++) {
    if (secondSwapConditions.at(i)) {
      auto tmp = secondHalfAfterPermute.at(i);
      secondHalfAfterPermute[i] = firstHalfAfterPermute.at(i);
      firstHalfAfterPermute[i] = tmp;
    }
  }
  firstHalfAfterPermute.insert(
      firstHalfAfterPermute.end(),
      secondHalfAfterPermute.begin(),
      secondHalfAfterPermute.end());

  testVectorEq(firstHalfAfterPermute, order);
}

TEST(AsWaksmanParameterTest, testAsWaksmanParameterCalculator) {
  size_t repeat = 100;
  for (size_t i = 0; i < repeat; i++) {
    testAsWaksmanParameter();
  }
}

} // namespace fbpcf::mpc_std_lib::permuter
