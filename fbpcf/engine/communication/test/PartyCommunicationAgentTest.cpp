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
#include <stdexcept>
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
  tlsInfo.rootCaCertPath = createdDir + "/ca_cert.pem";
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

TEST(
    SocketPartyCommunicationAgentTest,
    testSendAndReceiveWithTlsDifferentCaCert) {
  auto createdDir = setUpTlsFiles();
  auto dummyCreatedDir = setUpTlsFiles();

  /*
    Here, we create two root CA certs. We pass one in as the
    trusted cert, but we use the other one to sign the server
    certificate. We should see an exception.
   */
  fbpcf::engine::communication::SocketPartyCommunicationAgent::TlsInfo tlsInfo;
  tlsInfo.certPath = dummyCreatedDir + "/cert.pem";
  tlsInfo.keyPath = dummyCreatedDir + "/key.pem";
  tlsInfo.passphrasePath = dummyCreatedDir + "/passphrase.pem";
  tlsInfo.rootCaCertPath = createdDir + "/ca_cert.pem";
  tlsInfo.useTls = true;

  std::vector<std::unique_ptr<SocketPartyCommunicationAgentFactoryForTests>>
      factories(2);
  EXPECT_THROW(
      getSocketFactoriesForMultipleParties(2, tlsInfo, factories),
      std::runtime_error);

  deleteTlsFiles(createdDir);
  deleteTlsFiles(dummyCreatedDir);
}

TEST(SocketPartyCommunicationAgentTest, testSendAndReceiveWithTlsNoCaCert) {
  auto createdDir = setUpTlsFiles();

  /*
    In this case, we don't specify a root CA certificate,
    which should result in an exception.
   */
  fbpcf::engine::communication::SocketPartyCommunicationAgent::TlsInfo tlsInfo;
  tlsInfo.certPath = createdDir + "/cert.pem";
  tlsInfo.keyPath = createdDir + "/key.pem";
  tlsInfo.passphrasePath = createdDir + "/passphrase.pem";
  tlsInfo.useTls = true;

  std::vector<std::unique_ptr<SocketPartyCommunicationAgentFactoryForTests>>
      factories(2);
  EXPECT_THROW(
      getSocketFactoriesForMultipleParties(2, tlsInfo, factories),
      std::runtime_error);

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
  fbpcf::engine::communication::SocketPartyCommunicationAgent::TlsInfo tlsInfo;
  tlsInfo.certPath = "";
  tlsInfo.keyPath = "";
  tlsInfo.passphrasePath = "";
  tlsInfo.useTls = false;

  std::vector<std::unique_ptr<SocketPartyCommunicationAgentFactoryForTests>>
      factories(3);
  getSocketFactoriesForMultipleParties(3, tlsInfo, factories);

  std::vector<std::map<int, int>> boundPorts(3);
  for (size_t i = 0; i < factories.size(); i++) {
    boundPorts.at(i) = factories.at(i)->getBoundPorts();
  }

  auto createAgent = [](IPartyCommunicationAgentFactory* factory,
                        int myId,
                        int otherParty) {
    XLOGF(INFO, "Creating agent. MyID: {}, otherPartyID: {}", myId, otherParty);
    return factory->create(otherParty, "");
  };

  // we expect the below to not time out. if it returns
  // that means an agent has successfully been created

  // create agent between party 0 and 1
  auto agent01 = std::async(createAgent, factories.at(0).get(), 0, 1);
  auto agent10 = std::async(createAgent, factories.at(1).get(), 1, 0);

  // create agent between party 0 and 2
  auto agent02 = std::async(createAgent, factories.at(0).get(), 0, 2);
  auto agent20 = std::async(createAgent, factories.at(2).get(), 2, 0);

  // create agent between party 1 and 2
  auto agent12 = std::async(createAgent, factories.at(1).get(), 1, 2);
  auto agent21 = std::async(createAgent, factories.at(2).get(), 2, 1);

  EXPECT_NE(agent01.get(), nullptr);
  EXPECT_NE(agent10.get(), nullptr);
  EXPECT_NE(agent02.get(), nullptr);
  EXPECT_NE(agent20.get(), nullptr);
  EXPECT_NE(agent12.get(), nullptr);
  EXPECT_NE(agent21.get(), nullptr);
}

TEST(SocketPartyCommunicationAgentTest, testTimeout) {
  fbpcf::engine::communication::SocketPartyCommunicationAgent::TlsInfo tlsInfo;
  tlsInfo.certPath = "";
  tlsInfo.keyPath = "";
  tlsInfo.passphrasePath = "";
  tlsInfo.useTls = false;

  /* We use not-so-reliable ports since we expect the constructors to throw.
   * It's OK if those ports are occupied. */
  auto port01 = SocketInTestHelper::findNextOpenPort(5000);
  // make sure a port larger than 5000 is found.
  ASSERT_GE(port01, 5000);

  auto port02 = port01 + 4;
  auto port12 = port01 + 8;

  std::map<int, SocketPartyCommunicationAgentFactory::PartyInfo> partyInfo0 = {
      {1, {"127.0.0.1", port01}}, {2, {"127.0.0.1", port02}}};
  std::map<int, SocketPartyCommunicationAgentFactory::PartyInfo> partyInfo1 = {
      {0, {"127.0.0.1", port01}}, {2, {"127.0.0.1", port12}}};
  std::map<int, SocketPartyCommunicationAgentFactory::PartyInfo> partyInfo2 = {
      {0, {"127.0.0.1", port02}}, {1, {"127.0.0.1", port12}}};

  EXPECT_THROW(
      std::make_unique<SocketPartyCommunicationAgentFactory>(
          1,
          partyInfo1,
          tlsInfo,
          std::make_shared<fbpcf::util::MetricCollector>("Party_1"),
          10),
      std::runtime_error);

  EXPECT_THROW(
      std::make_unique<SocketPartyCommunicationAgentFactory>(
          2,
          partyInfo2,
          tlsInfo,
          std::make_shared<fbpcf::util::MetricCollector>("Party_2"),
          10),
      std::runtime_error);

  EXPECT_THROW(
      std::make_unique<SocketPartyCommunicationAgentFactory>(
          0,
          partyInfo0,
          tlsInfo,
          std::make_shared<fbpcf::util::MetricCollector>("Party_0"),
          10),
      std::runtime_error);
}

TEST(UtilTest, TestGetTlsInfoFromArguments) {
  auto tlsInfo = getTlsInfoFromArgs(
      false,
      "cert_path",
      "server_cert_path",
      "private_key_path",
      "passphrase_path");

  EXPECT_FALSE(tlsInfo.useTls);
  EXPECT_STREQ(tlsInfo.rootCaCertPath.c_str(), "");
  EXPECT_STREQ(tlsInfo.certPath.c_str(), "");
  EXPECT_STREQ(tlsInfo.keyPath.c_str(), "");
  EXPECT_STREQ(tlsInfo.passphrasePath.c_str(), "");

  const char* home_dir = std::getenv("HOME");
  if (home_dir == nullptr) {
    home_dir = "";
  }

  std::string home_dir_string(home_dir);

  tlsInfo = getTlsInfoFromArgs(
      true,
      "cert_path",
      "server_cert_path",
      "private_key_path",
      "passphrase_path");

  EXPECT_TRUE(tlsInfo.useTls);
  EXPECT_STREQ(
      tlsInfo.rootCaCertPath.c_str(), (home_dir_string + "/cert_path").c_str());
  EXPECT_STREQ(
      tlsInfo.certPath.c_str(),
      (home_dir_string + "/server_cert_path").c_str());
  EXPECT_STREQ(
      tlsInfo.keyPath.c_str(), (home_dir_string + "/private_key_path").c_str());
  EXPECT_STREQ(
      tlsInfo.passphrasePath.c_str(),
      (home_dir_string + "/passphrase_path").c_str());
}

} // namespace fbpcf::engine::communication
