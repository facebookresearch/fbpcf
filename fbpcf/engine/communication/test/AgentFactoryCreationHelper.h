/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <future>
#include <memory>

#include "fbpcf/engine/communication/InMemoryPartyCommunicationAgentFactory.h"
#include "fbpcf/engine/communication/test/SocketPartyCommunicationAgentFactoryForTests.h"

namespace fbpcf::engine::communication {

inline std::vector<std::unique_ptr<IPartyCommunicationAgentFactory>>
getInMemoryAgentFactory(int numberOfParty) {
  auto maps = std::vector<std::map<
      int,
      std::shared_ptr<InMemoryPartyCommunicationAgentFactory::HostInfo>>>(
      numberOfParty);
  std::vector<std::unique_ptr<IPartyCommunicationAgentFactory>> rst;

  for (int i = 0; i < numberOfParty; i++) {
    for (int j = i + 1; j < numberOfParty; j++) {
      auto info =
          std::make_shared<InMemoryPartyCommunicationAgentFactory::HostInfo>();
      info->mutex = std::make_unique<std::mutex>();
      maps[i].emplace(j, info);
      maps[j].emplace(i, info);
    }
  }
  for (int i = 0; i < numberOfParty; i++) {
    rst.push_back(std::make_unique<InMemoryPartyCommunicationAgentFactory>(
        i, std::move(maps[i])));
  }
  return rst;
}

inline std::pair<
    std::unique_ptr<communication::IPartyCommunicationAgentFactory>,
    std::unique_ptr<communication::IPartyCommunicationAgentFactory>>
getSocketAgentFactoryPair(
    fbpcf::engine::communication::SocketPartyCommunicationAgent::TlsInfo&
        tlsInfo) {
  std::map<
      int,
      fbpcf::engine::communication::SocketPartyCommunicationAgentFactory::
          PartyInfo>
      partyInfosAlice({{1, {"", 0}}});

  auto communicationAgentFactoryAlice =
      std::make_unique<fbpcf::engine::communication::
                           SocketPartyCommunicationAgentFactoryForTests>(
          0,
          partyInfosAlice,
          tlsInfo,
          std::make_shared<fbpcf::util::MetricCollector>(
              "aggregation_test_traffic"));

  std::map<
      int,
      fbpcf::engine::communication::SocketPartyCommunicationAgentFactory::
          PartyInfo>
      partyInfosBob({{0, {"127.0.0.1", 0}}});

  auto boundPorts = communicationAgentFactoryAlice->getBoundPorts();
  partyInfosBob.at(0).portNo =
      boundPorts.at(1); // tell partner how to connect to publisher

  auto communicationAgentFactoryBob =
      std::make_unique<fbpcf::engine::communication::
                           SocketPartyCommunicationAgentFactoryForTests>(
          1,
          partyInfosBob,
          tlsInfo,
          std::make_shared<fbpcf::util::MetricCollector>(
              "aggregation_test_traffic"));

  auto task =
      [](std::unique_ptr<fbpcf::engine::communication::
                             SocketPartyCommunicationAgentFactoryForTests>
             factory) {
        factory->completeNetworkingSetup();
        return factory;
      };

  auto futureAlice =
      std::async(task, std::move(communicationAgentFactoryAlice));
  auto futureBob = std::async(task, std::move(communicationAgentFactoryBob));

  return {futureAlice.get(), futureBob.get()};
};

} // namespace fbpcf::engine::communication
