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
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/DummySinglePointCotFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/SinglePointCotFactory.h"
#include "fbpcf/engine/util/AesPrgFactory.h"
#include "fbpcf/test/TestHelper.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::ferret {

void testSpCot(
    std::unique_ptr<ISinglePointCot> sender,
    std::unique_ptr<ISinglePointCot> receiver) {
  __m128i delta = _mm_set_epi32(1, 1, 1, 1);
  int baseOtSize = 12;
  std::vector<__m128i> baseOTSend(baseOtSize);
  std::vector<__m128i> baseOTReceive(baseOtSize);

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);

  auto position = dist(e) & 0xFFF;

  for (int i = 0; i < baseOtSize; i++) {
    baseOTSend[i] = _mm_set_epi32(dist(e), dist(e), dist(e), dist(e) << 1);
    baseOTReceive[i] = baseOTSend[i];

    if (!((position >> (baseOtSize - 1 - i)) & 1)) {
      baseOTReceive[i] = _mm_xor_si128(baseOTReceive[i], delta);
    }
  }

  auto senderTask = [delta](
                        std::unique_ptr<ISinglePointCot> spcot,
                        std::vector<__m128i>&& baseCot) {
    spcot->senderInit(delta);
    return spcot->senderExtend(std::move(baseCot));
  };

  auto receiverTask = [](std::unique_ptr<ISinglePointCot> spcot,
                         std::vector<__m128i>&& baseCot) {
    spcot->receiverInit();
    return spcot->receiverExtend(std::move(baseCot));
  };

  auto f0 = std::async(senderTask, std::move(sender), std::move(baseOTSend));
  auto f1 =
      std::async(receiverTask, std::move(receiver), std::move(baseOTReceive));

  auto sendResult = f0.get();
  auto receiveResult = f1.get();

  for (int i = 0; i < pow(2, baseOtSize); i++) {
    if (i == position) {
      EXPECT_TRUE(
          compareM128i(sendResult[i], _mm_xor_si128(receiveResult[i], delta)));
    } else {
      EXPECT_TRUE(compareM128i(sendResult[i], receiveResult[i]));
    }
  }
}

TEST(SPCotExtenderTest, testDummySPCot) {
  communication::InMemoryPartyCommunicationAgentHost host;

  std::unique_ptr<communication::IPartyCommunicationAgent> agent0 =
      host.getAgent(0);
  std::unique_ptr<communication::IPartyCommunicationAgent> agent1 =
      host.getAgent(1);

  insecure::DummySinglePointCotFactory factory(
      std::make_unique<util::AesPrgFactory>(1024));
  auto sender = factory.create(agent0);
  auto receiver = factory.create(agent1);

  testSpCot(std::move(sender), std::move(receiver));
}

TEST(SPCotExtenderTest, testRealSPCot) {
  communication::InMemoryPartyCommunicationAgentHost host;

  std::unique_ptr<communication::IPartyCommunicationAgent> agent0 =
      host.getAgent(0);
  std::unique_ptr<communication::IPartyCommunicationAgent> agent1 =
      host.getAgent(1);

  SinglePointCotFactory factory;
  auto sender = factory.create(agent0);
  auto receiver = factory.create(agent1);

  testSpCot(std::move(sender), std::move(receiver));
}

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::ferret
