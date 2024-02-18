/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstddef>
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

inline std::vector<
    std::unique_ptr<SocketPartyCommunicationAgentFactoryForTests>>&
getSocketFactoriesForMultipleParties(
    int numParties,
    SocketPartyCommunicationAgent::TlsInfo& tlsInfo,
    std::vector<std::unique_ptr<
        communication::SocketPartyCommunicationAgentFactoryForTests>>&
        factories) {
  std::vector<std::map<int, SocketPartyCommunicationAgentFactory::PartyInfo>>
      partyInfoVec(numParties);

  std::map<int, SocketPartyCommunicationAgentFactory::PartyInfo> partyInfo;
  std::vector<std::map<int, int>> boundPorts(numParties);

  // 1. Create partyInfo that contains an entry for all parties
  for (size_t i = 0; i < numParties; i++) {
    partyInfo.insert({i, {"127.0.0.1", 0}});
  }

  // 2. Create a Socket agent for each party, and retrieve its ports
  for (size_t i = 0; i < numParties; i++) {
    partyInfoVec.at(i) = partyInfo;
    factories.at(i) =
        std::make_unique<SocketPartyCommunicationAgentFactoryForTests>(
            i,
            partyInfo,
            tlsInfo,
            std::make_shared<fbpcf::util::MetricCollector>(
                "Party_" + std::to_string(i)));
    boundPorts.at(i) = factories.at(i)->getBoundPorts();
  }

  // 3. Point each party to the bound port for all other parties
  for (size_t i = 0; i < numParties; i++) {
    for (size_t j = 0; j < numParties; j++) {
      if (j > i) {
        partyInfoVec.at(j).at(i).portNo = boundPorts.at(i).at(j);
      }
    }
  }

  // 4. Define all futures to complete networking connection
  auto task =
      [](std::unique_ptr<SocketPartyCommunicationAgentFactoryForTests> factory,
         std::map<int, SocketPartyCommunicationAgentFactory::PartyInfo>
             partyInfo) {
        factory->completeNetworkingSetup(partyInfo);
        return factory;
      };

  std::vector<std::future<
      std::unique_ptr<SocketPartyCommunicationAgentFactoryForTests>>>
      futures(numParties);
  for (size_t i = 0; i < numParties; i++) {
    futures.at(i) =
        std::async(task, std::move(factories.at(i)), partyInfoVec.at(i));
  }

  // 5. Wait for all futures
  for (size_t i = 0; i < numParties; i++) {
    factories.at(i) = futures.at(i).get();
  }

  return factories;
}

inline std::pair<
    std::unique_ptr<communication::IPartyCommunicationAgentFactory>,
    std::unique_ptr<communication::IPartyCommunicationAgentFactory>>
getSocketAgentFactoryPair(
    fbpcf::engine::communication::SocketPartyCommunicationAgent::TlsInfo&
        tlsInfo) {
  std::vector<std::unique_ptr<SocketPartyCommunicationAgentFactoryForTests>>
      factories(2);
  getSocketFactoriesForMultipleParties(2, tlsInfo, factories);

  return {std::move(factories.at(0)), std::move(factories.at(1))};
}

} // namespace fbpcf::engine::communication
