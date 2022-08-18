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
#include "fbpcf/scheduler/WireKeeper.h"
#include "fbpcf/test/TestHelper.h"

#include "fbpcf/engine/SecretShareEngineFactory.h"
#include "fbpcf/engine/communication/InMemoryPartyCommunicationAgentFactory.h"
#include "fbpcf/scheduler/EagerScheduler.h"

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
    std::reference_wrapper<
        engine::communication::IPartyCommunicationAgentFactory> factory,
    SchedulerCreator schedulerCreator) {
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);

  typename BillionaireProblemGame<schedulerId, false>::AssetsLists myAssets;
  typename BillionaireProblemGame<schedulerId, false>::AssetsLists dummyAssets;

  myAssets.cash = dist(e);
  myAssets.stock = dist(e);
  myAssets.property = dist(e);

  auto scheduler = schedulerCreator(myId, factory);

  auto game = std::make_unique<BillionaireProblemGame<schedulerId, false>>(
      std::move(scheduler));

  auto mpcResult = myId == 0 ? game->billionaireProblem(myAssets, dummyAssets)
                             : game->billionaireProblem(dummyAssets, myAssets);

  uint64_t myTotalAssets;
  myTotalAssets = myAssets.cash + myAssets.stock + myAssets.property;

  return {mpcResult, myTotalAssets};
}

void testWithScheduler(SchedulerCreator schedulerCreator) {
  auto factories = engine::communication::getInMemoryAgentFactory(2);

  auto future0 = std::async(
      runWithScheduler<0>,
      0,
      std::reference_wrapper<
          engine::communication::IPartyCommunicationAgentFactory>(
          *factories[0]),
      schedulerCreator);

  auto future1 = std::async(
      runWithScheduler<1>,
      1,
      std::reference_wrapper<
          engine::communication::IPartyCommunicationAgentFactory>(
          *factories[1]),
      schedulerCreator);

  auto [result, aliceTotal] = future0.get();
  auto [_, bobTotal] = future1.get();

  EXPECT_EQ(aliceTotal < bobTotal, result);
}

TEST(BillionaireProblemTest, testWithNetworkPlaintextScheduler) {
  testWithScheduler(fbpcf::getSchedulerCreator<unsafe>(
      fbpcf::SchedulerType::NetworkPlaintext));
}

TEST(BillionaireProblemTest, testWithEagerScheduler) {
  testWithScheduler(scheduler::createEagerSchedulerWithRealEngine);
}

TEST(BillionaireProblemTest, testWithLazyScheduler) {
  testWithScheduler(scheduler::createLazySchedulerWithRealEngine);
}

template <int schedulerId>
std::pair<std::vector<bool>, std::vector<uint64_t>> runBatchWithScheduler(
    int size,
    int myId,
    std::reference_wrapper<
        engine::communication::IPartyCommunicationAgentFactory> factory,
    SchedulerCreator schedulerCreator) {
  auto scheduler = schedulerCreator(myId, factory);
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

void testBatchBillionaireProblem(fbpcf::SchedulerCreator schedulerCreator) {
  auto factories = engine::communication::getInMemoryAgentFactory(2);

  int size = 16384;

  auto future0 = std::async(
      runBatchWithScheduler<0>,
      size,
      0,
      std::reference_wrapper<
          engine::communication::IPartyCommunicationAgentFactory>(
          *factories[0]),
      schedulerCreator);

  auto future1 = std::async(
      runBatchWithScheduler<1>,
      size,
      1,
      std::reference_wrapper<
          engine::communication::IPartyCommunicationAgentFactory>(
          *factories[1]),
      schedulerCreator);

  auto [result, aliceTotal] = future0.get();
  auto bobTotal = future1.get().second;

  EXPECT_EQ(aliceTotal.size(), bobTotal.size());
  EXPECT_EQ(aliceTotal.size(), result.size());

  for (size_t i = 0; i < result.size(); i++) {
    EXPECT_EQ(aliceTotal.at(i) < bobTotal.at(i), result.at(i));
  }
}

TEST(BillionaireProblemTest, testBatchWithNetworkPlaintextScheduler) {
  testBatchBillionaireProblem(fbpcf::getSchedulerCreator<unsafe>(
      fbpcf::SchedulerType::NetworkPlaintext));
}

TEST(BillionaireProblemTest, testBatchWithEagerSchedulerAndFERRET) {
  testBatchBillionaireProblem(scheduler::createEagerSchedulerWithRealEngine);
}

TEST(BillionaireProblemTest, testBatchWithEagerSchedulerAndClassicOT) {
  testBatchBillionaireProblem(scheduler::createEagerSchedulerWithClassicOT);
}

TEST(BillionaireProblemTest, testBatchWithLazySchedulerAndFERRET) {
  testBatchBillionaireProblem(scheduler::createLazySchedulerWithRealEngine);
}

TEST(BillionaireProblemTest, testBatchWithLazySchedulerAndClassicOT) {
  testBatchBillionaireProblem(scheduler::createLazySchedulerWithClassicOT);
}

} // namespace fbpcf::billionaire_problem
