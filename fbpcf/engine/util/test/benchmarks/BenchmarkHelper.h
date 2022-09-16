/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <random>
#include <stdexcept>
#include <utility>

#include "fbpcf/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/engine/communication/SocketPartyCommunicationAgentFactory.h"
#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/engine/communication/test/SocketInTestHelper.h"
#include "folly/Random.h"
#include "folly/logging/xlog.h"

namespace fbpcf::engine::util {

inline std::vector<bool> getRandomBoolVector(size_t size) {
  auto result = std::vector<bool>(size);
  for (auto i = 0; i < size; ++i) {
    result[i] = folly::Random::secureOneIn(2);
  }
  return result;
}

inline std::pair<
    std::unique_ptr<communication::IPartyCommunicationAgent>,
    std::unique_ptr<communication::IPartyCommunicationAgent>>
getSocketAgents() {
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<int> intDistro(10000, 25000);

  auto port = communication::SocketInTestHelper::findNextOpenPort(5000);
  std::map<int, communication::SocketPartyCommunicationAgentFactory::PartyInfo>
      partyInfo0 = {{1, {"127.0.0.1", port}}};
  std::map<int, communication::SocketPartyCommunicationAgentFactory::PartyInfo>
      partyInfo1 = {{0, {"127.0.0.1", port}}};

  fbpcf::engine::communication::SocketPartyCommunicationAgent::TlsInfo tlsInfo;
  tlsInfo.certPath = "";
  tlsInfo.keyPath = "";
  tlsInfo.passphrasePath = "";
  tlsInfo.useTls = false;

  auto factory1Future = std::async([&partyInfo1, &tlsInfo]() {
    return std::make_unique<
        communication::SocketPartyCommunicationAgentFactory>(
        1, partyInfo1, tlsInfo, "party_1_unit_test_traffic");
  });
  auto factory0 =
      std::make_unique<communication::SocketPartyCommunicationAgentFactory>(
          0, partyInfo0, tlsInfo, "party_0_unit_test_traffic");
  auto factory1 = factory1Future.get();

  auto task = [](std::unique_ptr<communication::IPartyCommunicationAgentFactory>
                     factory,
                 int myId) {
    return factory->create(1 - myId, "benchmark_traffic");
  };

  auto createSocketAgent0 = std::async(task, std::move(factory0), 0);
  auto createSocketAgent1 = std::async(task, std::move(factory1), 1);

  auto agent0 = createSocketAgent0.get();
  auto agent1 = createSocketAgent1.get();

  return {std::move(agent0), std::move(agent1)};
}

// A wrapper class around the existing SocketPartyCommunicationAgentFactory,
// with retries added to prevent flakiness when binding to unavailable ports.
class BenchmarkSocketAgentFactory final
    : public communication::IPartyCommunicationAgentFactory {
 public:
  struct AgentsByParty {
    // The agents for party i are stored at index i of the vector.
    std::vector<
        std::queue<std::unique_ptr<communication::IPartyCommunicationAgent>>>
        agents;
    std::unique_ptr<std::mutex> mutex;
  };

  BenchmarkSocketAgentFactory(
      int myId,
      std::shared_ptr<AgentsByParty> agentsByParty)
      : communication::IPartyCommunicationAgentFactory("benchmark_traffic"),
        myId_(myId),
        agentsByParty_(agentsByParty) {}

  // The socket agents are stored in a queue.
  // If a party calls create() and the queue is empty, create a new pair of
  // socket agents and add it to the queue. If the queue is not empty, take the
  // socket agent from the front of the queue. This code uses a mutex to ensure
  // there are no race conditions.
  // NOTE: This API is not thread-safe if one party is creating new agents in
  // multiple threads.
  std::unique_ptr<communication::IPartyCommunicationAgent> create(
      int id,
      std::string) override {
    std::lock_guard<std::mutex> lock(*agentsByParty_->mutex);

    if (agentsByParty_->agents.at(myId_).empty()) {
      auto [agent0, agent1] = getSocketAgents();
      agentsByParty_->agents.at(0).push(std::move(agent0));
      agentsByParty_->agents.at(1).push(std::move(agent1));
    }

    auto agent = std::move(agentsByParty_->agents.at(myId_).front());
    agentsByParty_->agents.at(myId_).pop();
    return agent;
  }

 private:
  int myId_;
  std::shared_ptr<AgentsByParty> agentsByParty_;
};

inline std::pair<
    std::unique_ptr<communication::IPartyCommunicationAgentFactory>,
    std::unique_ptr<communication::IPartyCommunicationAgentFactory>>
getSocketAgentFactories() {
  fbpcf::engine::communication::SocketPartyCommunicationAgent::TlsInfo tlsInfo;
  tlsInfo.certPath = "";
  tlsInfo.keyPath = "";
  tlsInfo.passphrasePath = "";
  tlsInfo.useTls = false;

  return communication::getSocketAgentFactoryPair(tlsInfo);
};

} // namespace fbpcf::engine::util
