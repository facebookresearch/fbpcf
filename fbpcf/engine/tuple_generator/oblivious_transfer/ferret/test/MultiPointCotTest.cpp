/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <emmintrin.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <future>
#include <memory>
#include <random>

#include "fbpcf/engine/communication/InMemoryPartyCommunicationAgentHost.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/DummyMultiPointCotFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/DummySinglePointCotFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/RegularErrorMultiPointCotFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/SinglePointCotFactory.h"
#include "fbpcf/engine/util/AesPrgFactory.h"
#include "fbpcf/test/TestHelper.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::ferret {

void testMpCot(
    std::unique_ptr<IMultiPointCot> sender,
    std::unique_ptr<IMultiPointCot> receiver) {
  int64_t length = 1024;
  int64_t weight = 16;
  __m128i delta = _mm_set_epi32(1, 1, 1, 1);

  sender->senderInit(delta, length, weight);
  receiver->receiverInit(length, weight);

  auto baseOtSize = sender->getBaseCotNeeds();
  EXPECT_EQ(baseOtSize, receiver->getBaseCotNeeds());

  std::vector<__m128i> baseOTSend(baseOtSize);
  std::vector<__m128i> baseOTReceive(baseOtSize);

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);

  for (int i = 0; i < baseOtSize; i++) {
    baseOTSend[i] = _mm_set_epi32(dist(e), dist(e), dist(e), dist(e) << 1);
    baseOTReceive[i] = baseOTSend[i];

    if (i % 2 == 0) {
      baseOTReceive[i] = _mm_xor_si128(baseOTReceive[i], delta);
    }
  }

  auto senderTask = [](std::unique_ptr<IMultiPointCot> mpcot,
                       std::vector<__m128i>&& baseCot) {
    return mpcot->senderExtend(std::move(baseCot));
  };

  auto receiverTask = [](std::unique_ptr<IMultiPointCot> mpcot,
                         std::vector<__m128i>&& baseCot) {
    return mpcot->receiverExtend(std::move(baseCot));
  };

  auto f0 = std::async(senderTask, std::move(sender), std::move(baseOTSend));
  auto f1 =
      std::async(receiverTask, std::move(receiver), std::move(baseOTReceive));

  auto sendResult = f0.get();
  auto receiveResult = f1.get();

  int count = 0;
  EXPECT_EQ(sendResult.size(), length);
  EXPECT_EQ(receiveResult.size(), length);

  for (int i = 0; i < sendResult.size(); i++) {
    EXPECT_TRUE(
        compareM128i(sendResult[i], receiveResult[i]) ||
        compareM128i(_mm_xor_si128(sendResult[i], delta), receiveResult[i]));
    if (compareM128i(_mm_xor_si128(sendResult[i], delta), receiveResult[i])) {
      count++;
    }
  }
  EXPECT_EQ(count, weight);
}

TEST(MPCotExtenderTest, testDummyMPCot) {
  communication::InMemoryPartyCommunicationAgentHost host;

  std::unique_ptr<communication::IPartyCommunicationAgent> agent0 =
      host.getAgent(0);
  std::unique_ptr<communication::IPartyCommunicationAgent> agent1 =
      host.getAgent(1);

  insecure::DummyMultiPointCotFactory factory(
      std::make_unique<util::AesPrgFactory>(1024));
  auto sender = factory.create(agent0);
  auto receiver = factory.create(agent1);

  testMpCot(std::move(sender), std::move(receiver));
}

TEST(MPCotExtenderTest, testRealMPCotWithDummySpcot) {
  communication::InMemoryPartyCommunicationAgentHost host;

  std::unique_ptr<communication::IPartyCommunicationAgent> agent0 =
      host.getAgent(0);
  std::unique_ptr<communication::IPartyCommunicationAgent> agent1 =
      host.getAgent(1);

  RegularErrorMultiPointCotFactory factory(
      std::make_unique<insecure::DummySinglePointCotFactory>(
          std::make_unique<util::AesPrgFactory>(1024)));

  auto sender = factory.create(agent0);
  auto receiver = factory.create(agent1);

  testMpCot(std::move(sender), std::move(receiver));
}

TEST(MPCotExtenderTest, testRealMPCotWithRealSpcot) {
  communication::InMemoryPartyCommunicationAgentHost host;

  std::unique_ptr<communication::IPartyCommunicationAgent> agent0 =
      host.getAgent(0);
  std::unique_ptr<communication::IPartyCommunicationAgent> agent1 =
      host.getAgent(1);

  RegularErrorMultiPointCotFactory factory(
      std::make_unique<SinglePointCotFactory>());

  auto sender = factory.create(agent0);
  auto receiver = factory.create(agent1);

  testMpCot(std::move(sender), std::move(receiver));
}

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::ferret
