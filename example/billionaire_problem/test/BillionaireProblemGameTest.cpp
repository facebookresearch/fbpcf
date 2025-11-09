/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "../BillionaireProblemGame.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <functional>
#include <memory>
#include <random>
#include <tuple>
#include <vector>

#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/engine/util/AesPrgFactory.h"
#include "fbpcf/scheduler/IScheduler.h"
#include "fbpcf/scheduler/PlaintextScheduler.h"
#include "fbpcf/scheduler/SchedulerHelper.h"
#include "fbpcf/test/TestHelper.h"

#include "fbpcf/engine/SecretShareEngineFactory.h"

namespace fbpcf::billionaire_problem {

const bool unsafe = true;

void initializeBatch(
    std::vector<uint32_t>& assetBatch,
    std::uniform_int_distribution<uint32_t> dist,
    std::mt19937_64 e) {
  for (auto& item : assetBatch) {
    item = dist(e);
  }
}

template <int schedulerId>
std::pair<bool, uint64_t> runWithScheduler(
    int myId,
    std::shared_ptr<fbpcf::scheduler::ISchedulerFactory<unsafe>>
        schedulerFactory) {
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);

  typename BillionaireProblemGame<schedulerId, false>::AssetsLists myAssets;
  typename BillionaireProblemGame<schedulerId, false>::AssetsLists dummyAssets;

  myAssets.cash = dist(e);
  myAssets.stock = dist(e);
  myAssets.property = dist(e);

  auto scheduler = schedulerFactory->create();

  auto game = std::make_unique<BillionaireProblemGame<schedulerId, false>>(
      std::move(scheduler));

  auto mpcResult = myId == 0 ? game->billionaireProblem(myAssets, dummyAssets)
                             : game->billionaireProblem(dummyAssets, myAssets);

  uint64_t myTotalAssets;
  myTotalAssets = myAssets.cash + myAssets.stock + myAssets.property;

  return {mpcResult, myTotalAssets};
}

void testWithScheduler(
    fbpcf::SchedulerType schedulerType,
    fbpcf::EngineType engineType) {
  auto communicationAgentFactories =
      engine::communication::getInMemoryAgentFactory(2);

  // Creating shared pointers to the communicationAgentFactories
  std::shared_ptr<fbpcf::engine::communication::IPartyCommunicationAgentFactory>
      communicationAgentFactory0 = std::move(communicationAgentFactories[0]);

  std::shared_ptr<fbpcf::engine::communication::IPartyCommunicationAgentFactory>
      communicationAgentFactory1 = std::move(communicationAgentFactories[1]);

  auto schedulerFactory0 = fbpcf::getSchedulerFactory<unsafe>(
      schedulerType, engineType, 0, *communicationAgentFactory0);
  auto schedulerFactory1 = fbpcf::getSchedulerFactory<unsafe>(
      schedulerType, engineType, 1, *communicationAgentFactory1);

  auto future0 =
      std::async(runWithScheduler<0>, 0, std::move(schedulerFactory0));

  auto future1 =
      std::async(runWithScheduler<1>, 1, std::move(schedulerFactory1));

  auto [result, aliceTotal] = future0.get();
  auto [_, bobTotal] = future1.get();

  EXPECT_EQ(aliceTotal < bobTotal, result);
}

TEST(BillionaireProblemTest, testWithNetworkPlaintextScheduler) {
  testWithScheduler(
      fbpcf::SchedulerType::NetworkPlaintext,
      fbpcf::EngineType::EngineWithDummyTuple);
}

TEST(BillionaireProblemTest, testWithEagerScheduler) {
  testWithScheduler(
      fbpcf::SchedulerType::Eager,
      fbpcf::EngineType::EngineWithTupleFromFERRET);
}

TEST(BillionaireProblemTest, testWithLazyScheduler) {
  testWithScheduler(
      fbpcf::SchedulerType::Lazy, fbpcf::EngineType::EngineWithTupleFromFERRET);
}

template <int schedulerId>
std::pair<std::vector<bool>, std::vector<uint64_t>> runBatchWithScheduler(
    int size,
    int myId,
    std::shared_ptr<fbpcf::scheduler::ISchedulerFactory<unsafe>>
        schedulerFactory) {
  auto scheduler = schedulerFactory->create();

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);

  typename BillionaireProblemGame<schedulerId, true>::AssetsLists myAssets = {
      .cash = std::vector<uint32_t>(size),
      .stock = std::vector<uint32_t>(size),
      .property = std::vector<uint32_t>(size),
  };

  typename BillionaireProblemGame<schedulerId, true>::AssetsLists dummyAssets =
      {
          .cash = std::vector<uint32_t>(size),
          .stock = std::vector<uint32_t>(size),
          .property = std::vector<uint32_t>(size),
      };

  initializeBatch(myAssets.cash, dist, e);
  initializeBatch(myAssets.stock, dist, e);
  initializeBatch(myAssets.property, dist, e);

  auto game = std::make_unique<BillionaireProblemGame<schedulerId, true>>(
      std::move(scheduler));

  auto mpcResult = myId == 0 ? game->billionaireProblem(myAssets, dummyAssets)
                             : game->billionaireProblem(dummyAssets, myAssets);

  std::vector<uint64_t> myTotalAssets(size);
  for (size_t i = 0; i < size; i++) {
    myTotalAssets[i] =
        myAssets.cash[i] + myAssets.stock[i] + myAssets.property[i];
  }

  return {mpcResult, myTotalAssets};
}

void testBatchBillionaireProblem(
    fbpcf::SchedulerType schedulerType,
    fbpcf::EngineType engineType) {
  auto communicationAgentFactories =
      engine::communication::getInMemoryAgentFactory(2);

  // Creating shared pointers to the communicationAgentFactories
  std::shared_ptr<fbpcf::engine::communication::IPartyCommunicationAgentFactory>
      communicationAgentFactory0 = std::move(communicationAgentFactories[0]);

  std::shared_ptr<fbpcf::engine::communication::IPartyCommunicationAgentFactory>
      communicationAgentFactory1 = std::move(communicationAgentFactories[1]);

  auto schedulerFactory0 = fbpcf::getSchedulerFactory<unsafe>(
      schedulerType, engineType, 0, *communicationAgentFactory0);
  auto schedulerFactory1 = fbpcf::getSchedulerFactory<unsafe>(
      schedulerType, engineType, 1, *communicationAgentFactory1);

  int size = 16384;

  auto future0 = std::async(
      runBatchWithScheduler<0>, size, 0, std::move(schedulerFactory0));

  auto future1 = std::async(
      runBatchWithScheduler<1>, size, 1, std::move(schedulerFactory1));

  auto [result, aliceTotal] = future0.get();
  auto bobTotal = future1.get().second;

  EXPECT_EQ(aliceTotal.size(), bobTotal.size());
  EXPECT_EQ(aliceTotal.size(), result.size());

  for (size_t i = 0; i < result.size(); i++) {
    EXPECT_EQ(aliceTotal.at(i) < bobTotal.at(i), result.at(i));
  }
}

TEST(BillionaireProblemTest, testBatchWithNetworkPlaintextScheduler) {
  testBatchBillionaireProblem(
      fbpcf::SchedulerType::NetworkPlaintext,
      fbpcf::EngineType::EngineWithDummyTuple);
}

TEST(BillionaireProblemTest, testBatchWithEagerSchedulerAndFERRET) {
  testBatchBillionaireProblem(
      fbpcf::SchedulerType::Eager,
      fbpcf::EngineType::EngineWithTupleFromFERRET);
}

TEST(BillionaireProblemTest, testBatchWithEagerSchedulerAndClassicOT) {
  testBatchBillionaireProblem(
      fbpcf::SchedulerType::Eager,
      fbpcf::EngineType::EngineWithTupleFromClassicOT);
}

TEST(BillionaireProblemTest, testBatchWithLazySchedulerAndFERRET) {
  testBatchBillionaireProblem(
      fbpcf::SchedulerType::Lazy, fbpcf::EngineType::EngineWithTupleFromFERRET);
}

TEST(BillionaireProblemTest, testBatchWithLazySchedulerAndClassicOT) {
  testBatchBillionaireProblem(
      fbpcf::SchedulerType::Lazy,
      fbpcf::EngineType::EngineWithTupleFromClassicOT);
}

} // namespace fbpcf::billionaire_problem
