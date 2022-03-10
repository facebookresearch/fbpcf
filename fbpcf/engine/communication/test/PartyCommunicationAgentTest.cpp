/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <emmintrin.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

#include "fbpcf/engine/communication/InMemoryPartyCommunicationAgentFactory.h"
#include "fbpcf/engine/communication/InMemoryPartyCommunicationAgentHost.h"
#include "fbpcf/engine/communication/SocketPartyCommunicationAgentFactory.h"
#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"

namespace fbpcf::engine::communication {

void sendAndReceive(std::unique_ptr<IPartyCommunicationAgent> agent, int size) {
  int repeat = 10;
  for (int t = 0; t < repeat; t++) {
    std::vector<unsigned char> sendData(size);
    for (int i = 0; i < size; i++) {
      sendData[i] = i & 0xFF;
    }
    agent->send(sendData);
    auto receivedData = agent->receive(size);
    EXPECT_EQ(receivedData.size(), size);
    for (int i = 0; i < size; i++) {
      EXPECT_EQ(receivedData[i], i & 0xFF);
    }
    auto traffic = agent->getTrafficStatistics();
    EXPECT_EQ(traffic.first, size * (t + 1));
    EXPECT_EQ(traffic.second, size * (t + 1));
  }
}

void testAgentFactory(
    int myId,
    int size,
    std::unique_ptr<IPartyCommunicationAgentFactory> factory) {
  auto agent = factory->create(1 - myId);
  sendAndReceive(std::move(agent), size);
}

TEST(InMemoryPartyCommunicationAgentTest, testSendAndReceive) {
  auto factorys = getInMemoryAgentFactory(2);

  int size = 1024;
  auto thread0 = std::thread(testAgentFactory, 0, size, std::move(factorys[0]));
  auto thread1 = std::thread(testAgentFactory, 1, size, std::move(factorys[1]));

  thread1.join();
  thread0.join();
}

TEST(SocketPartyCommunicationAgentTest, testSendAndReceive) {
  std::random_device rd;
  std::default_random_engine defEngine(rd());
  std::uniform_int_distribution<int> intDistro(10000, 25000);

  /*
   * We use random ports rather than constants because, during
   * stress runs, we get errors when trying to bind to the
   * same port multiple times.
   */
  std::map<int, SocketPartyCommunicationAgentFactory::PartyInfo> partyInfo = {
      {0, {"127.0.0.1", intDistro(defEngine)}},
      {1, {"127.0.0.1", intDistro(defEngine)}}};
  auto factory0 =
      std::make_unique<SocketPartyCommunicationAgentFactory>(0, partyInfo);
  auto factory1 =
      std::make_unique<SocketPartyCommunicationAgentFactory>(1, partyInfo);

  int size = 1024;
  auto thread0 = std::thread(testAgentFactory, 0, size, std::move(factory0));
  auto thread1 = std::thread(testAgentFactory, 1, size, std::move(factory1));

  thread1.join();
  thread0.join();
}

} // namespace fbpcf::engine::communication
