/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <emmintrin.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <filesystem>
#include <future>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include <folly/dynamic.h>
#include "fbpcf/engine/communication/InMemoryPartyCommunicationAgentFactory.h"
#include "fbpcf/engine/communication/InMemoryPartyCommunicationAgentHost.h"
#include "fbpcf/engine/communication/SocketPartyCommunicationAgentFactory.h"
#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/engine/communication/test/SocketInTestHelper.h"
#include "fbpcf/engine/communication/test/TlsCommunicationUtils.h"

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
    int totalParty,
    int size,
    std::unique_ptr<IPartyCommunicationAgentFactory> factory,
    std::string testname) {
  // repeat the process multiple times
  for (int t = 0; t < 3; t++) {
    folly::dynamic expectMetrics = folly::dynamic::object;
    for (int i = 0; i < totalParty; i++) {
      std::vector<std::thread> testThreads(0);
      if (i != myId) {
        auto agent =
            factory->create(i, "traffic_to_party_" + std::to_string(i));
        testThreads.push_back(
            std::thread(sendAndReceive, std::move(agent), size));

        expectMetrics.insert(
            testname + "." + "traffic_to_party_" + std::to_string(i),
            folly::dynamic::object("sent_data", size * 10)(
                "received_data", size * 10));
        // the port for syncing new port number is shared across different
        // iterations.
        expectMetrics.insert(
            testname + "." + "Port_number_sync_traffic_with_party_" +
                std::to_string(i),
            folly::dynamic::object(
                "sent_data", myId < i ? sizeof(int) * (t + 1) : 0)(
                "received_data", myId > i ? sizeof(int) * (t + 1) : 0));
      }
      for (auto& thread : testThreads) {
        thread.join();
      }
    }
    if (testname != "") {
      auto metrics = factory->getMetricsCollector()->collectMetrics();
      EXPECT_EQ(metrics, expectMetrics);
    }
  }
}

TEST(InMemoryPartyCommunicationAgentTest, testSendAndReceive) {
  auto factorys = getInMemoryAgentFactory(2);

  int size = 1024;
  auto thread0 =
      std::thread(testAgentFactory, 0, 2, size, std::move(factorys[0]), "");
  auto thread1 =
      std::thread(testAgentFactory, 1, 2, size, std::move(factorys[1]), "");

  thread1.join();
  thread0.join();
}

TEST(SocketPartyCommunicationAgentTest, testSendAndReceiveWithTls) {
  auto createdDir = setUpTlsFiles();

  fbpcf::engine::communication::SocketPartyCommunicationAgent::TlsInfo tlsInfo;
  tlsInfo.certPath = createdDir + "/cert.pem";
  tlsInfo.keyPath = createdDir + "/key.pem";
  tlsInfo.passphrasePath = createdDir + "/passphrase.pem";
  tlsInfo.useTls = true;

  std::vector<std::unique_ptr<SocketPartyCommunicationAgentFactoryForTests>>
      factories(3);
  getSocketFactoriesForMultipleParties(3, tlsInfo, factories);

  int size = 1048576; // 1024 ^ 2
  auto thread0 = std::thread(
      testAgentFactory, 0, 3, size, std::move(factories.at(0)), "Party_0");
  auto thread1 = std::thread(
      testAgentFactory, 1, 3, size, std::move(factories.at(1)), "Party_1");
  auto thread2 = std::thread(
      testAgentFactory, 2, 3, size, std::move(factories.at(2)), "Party_2");

  thread2.join();
  thread1.join();
  thread0.join();

  deleteTlsFiles(createdDir);
}

TEST(SocketPartyCommunicationAgentTest, testSendAndReceiveWithoutTls) {
  fbpcf::engine::communication::SocketPartyCommunicationAgent::TlsInfo tlsInfo;
  tlsInfo.certPath = "";
  tlsInfo.keyPath = "";
  tlsInfo.passphrasePath = "";
  tlsInfo.useTls = false;

  std::vector<std::unique_ptr<SocketPartyCommunicationAgentFactoryForTests>>
      factories(3);
  getSocketFactoriesForMultipleParties(3, tlsInfo, factories);

  int size = 1048576; // 1024 ^ 2

  auto thread0 = std::thread(
      testAgentFactory, 0, 3, size, std::move(factories.at(0)), "Party_0");
  auto thread1 = std::thread(
      testAgentFactory, 1, 3, size, std::move(factories.at(1)), "Party_1");
  auto thread2 = std::thread(
      testAgentFactory, 2, 3, size, std::move(factories.at(2)), "Party_2");

  thread2.join();
  thread1.join();
  thread0.join();
}

TEST(SocketPartyCommunicationAgentTest, testSendAndReceiveWithJammedPort) {
  {
    // jam port 5000
    struct sockaddr_in servAddr;

    memset((char*)&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;

    servAddr.sin_port = htons(5000);
    auto sockfd1 = socket(AF_INET, SOCK_STREAM, 0);
    ::bind(sockfd1, (struct sockaddr*)&servAddr, sizeof(struct sockaddr_in));
  }

  auto port01 = SocketInTestHelper::findNextOpenPort(5000);
  // make sure a port larger than 5000 is found.
  ASSERT_GE(port01, 5000);

  auto port02 = port01 + 4;
  auto port12 = port01 + 8;

  fbpcf::engine::communication::SocketPartyCommunicationAgent::TlsInfo tlsInfo;
  tlsInfo.certPath = "";
  tlsInfo.keyPath = "";
  tlsInfo.passphrasePath = "";
  tlsInfo.useTls = false;

  std::map<int, SocketPartyCommunicationAgentFactory::PartyInfo> partyInfo0 = {
      {1, {"127.0.0.1", port01}}, {2, {"127.0.0.1", port02}}};
  std::map<int, SocketPartyCommunicationAgentFactory::PartyInfo> partyInfo1 = {
      {0, {"127.0.0.1", port01}}, {2, {"127.0.0.1", port12}}};
  std::map<int, SocketPartyCommunicationAgentFactory::PartyInfo> partyInfo2 = {
      {0, {"127.0.0.1", port02}}, {1, {"127.0.0.1", port12}}};

  {
    // jam the ports here
    struct sockaddr_in servAddr;

    memset((char*)&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;

    servAddr.sin_port = htons(port01 + 1);
    auto sockfd1 = socket(AF_INET, SOCK_STREAM, 0);
    ::bind(sockfd1, (struct sockaddr*)&servAddr, sizeof(struct sockaddr_in));

    servAddr.sin_port = htons(port02 + 1);
    auto sockfd2 = socket(AF_INET, SOCK_STREAM, 0);
    ::bind(sockfd2, (struct sockaddr*)&servAddr, sizeof(struct sockaddr_in));

    servAddr.sin_port = htons(port12 + 1);
    auto sockfd3 = socket(AF_INET, SOCK_STREAM, 0);
    ::bind(sockfd3, (struct sockaddr*)&servAddr, sizeof(struct sockaddr_in));
  }

  auto factory1 = std::async([&partyInfo1, &tlsInfo]() {
    return std::make_unique<SocketPartyCommunicationAgentFactory>(
        1, partyInfo1, tlsInfo, "Party_1");
  });

  auto factory2 = std::async([&partyInfo2, &tlsInfo]() {
    return std::make_unique<SocketPartyCommunicationAgentFactory>(
        2, partyInfo2, tlsInfo, "Party_2");
  });

  auto factory0 = std::make_unique<SocketPartyCommunicationAgentFactory>(
      0, partyInfo0, tlsInfo, "Party_0");

  int size = 1048576; // 1024 ^ 2
  auto thread0 =
      std::thread(testAgentFactory, 0, 3, size, std::move(factory0), "Party_0");
  auto thread1 =
      std::thread(testAgentFactory, 1, 3, size, factory1.get(), "Party_1");
  auto thread2 =
      std::thread(testAgentFactory, 2, 3, size, factory2.get(), "Party_2");

  thread2.join();
  thread1.join();
  thread0.join();
}

} // namespace fbpcf::engine::communication
