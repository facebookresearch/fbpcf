/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <future>
#include <memory>
#include <random>
#include <thread>
#include "fbpcf/engine/communication/InMemoryPartyCommunicationAgentHost.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/DummyBaseObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/NpBaseObliviousTransfer.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/NpBaseObliviousTransferFactory.h"
#include "fbpcf/test/TestHelper.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer {

void testBaseObliviousTransfer(
    std::unique_ptr<IBaseObliviousTransferFactory> factory0,
    std::unique_ptr<IBaseObliviousTransferFactory> factory1) {
  communication::InMemoryPartyCommunicationAgentHost host;

  auto agent0 = host.getAgent(0);
  auto agent1 = host.getAgent(1);

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint8_t> dist(0, 1);

  size_t size = 128;

  std::vector<bool> choice(size);
  for (size_t i = 0; i < size; i++) {
    choice[i] = dist(e);
  }

  auto senderTask =
      [size](
          std::unique_ptr<IBaseObliviousTransferFactory> factory,
          std::unique_ptr<communication::IPartyCommunicationAgent> agent) {
        auto ot = factory->create(std::move(agent));

        auto [m0, m1] = ot->send(size);
        EXPECT_EQ(m0.size(), size);
        EXPECT_EQ(m1.size(), size);

        return std::make_pair(m0, m1);
      };

  auto receiverTask =
      [choice](
          std::unique_ptr<IBaseObliviousTransferFactory> factory,
          std::unique_ptr<communication::IPartyCommunicationAgent> agent) {
        auto ot = factory->create(std::move(agent));

        auto rst = ot->receive(choice);
        EXPECT_EQ(rst.size(), choice.size());

        return rst;
      };

  auto f0 = std::async(senderTask, std::move(factory0), std::move(agent0));
  auto f1 = std::async(receiverTask, std::move(factory1), std::move(agent1));

  auto [resultSend0, resultSend1] = f0.get();
  std::vector<std::vector<__m128i>> resultSend({resultSend0, resultSend1});
  auto resultReceive = f1.get();

  for (int i = 0; i < resultReceive.size(); i++) {
    EXPECT_TRUE(
        compareM128i(resultSend.at(choice[i]).at(i), resultReceive.at(i)));
  }
}

TEST(BaseObliviousTransferTest, testDummyBaseOT) {
  testBaseObliviousTransfer(
      std::make_unique<insecure::DummyBaseObliviousTransferFactory>(),
      std::make_unique<insecure::DummyBaseObliviousTransferFactory>());
}

class NpBaseOTTestHelper final : public NpBaseObliviousTransfer {
 public:
  explicit NpBaseOTTestHelper(
      std::unique_ptr<communication::IPartyCommunicationAgent> agent)
      : NpBaseObliviousTransfer(std::move(agent)) {}

  FRIEND_TEST(BaseObliviousTransferTest, TestNpBaseOTSendAndReceivePoints);
};

TEST(BaseObliviousTransferTest, TestNpBaseOTSendAndReceivePoints) {
  communication::InMemoryPartyCommunicationAgentHost host;
  auto agent0 = host.getAgent(0);
  auto agent1 = host.getAgent(1);

  NpBaseOTTestHelper ot0(std::move(agent0));
  NpBaseOTTestHelper ot1(std::move(agent1));

  // test sending a random point works as expected
  {
    std::unique_ptr<BN_CTX, std::function<void(BN_CTX*)>> ctx(
        BN_CTX_new(), BN_CTX_free);

    auto pointToSend = ot0.generateRandomPoint();

    ot0.sendPoint(*pointToSend);
    auto point = ot1.receivePoint();

    EXPECT_EQ(
        EC_POINT_cmp(
            ot0.group_.get(), pointToSend.get(), point.get(), ctx.get()),
        0);
    EXPECT_EQ(
        EC_POINT_cmp(
            ot1.group_.get(), pointToSend.get(), point.get(), ctx.get()),
        0);
    auto hash0 = ot0.hashPoint(*pointToSend, 0);
    auto hash1 = ot1.hashPoint(*point, 0);
    EXPECT_TRUE(compareM128i(hash0, hash1));
  }
}

TEST(BaseObliviousTransferTest, testNpBaseOT) {
  testBaseObliviousTransfer(
      std::make_unique<NpBaseObliviousTransferFactory>(),
      std::make_unique<NpBaseObliviousTransferFactory>());
}

} // namespace fbpcf::engine::tuple_generator::oblivious_transfer
