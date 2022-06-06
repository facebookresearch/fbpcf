/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fbpcf/engine/communication/SocketPartyCommunicationAgentFactory.h>
#include <iostream>
#include <memory>
#include "fbpcf/engine/communication/InMemoryPartyCommunicationAgentFactory.h"
#include "folly/Random.h"

namespace fbpcf::engine::communication {

inline std::vector<std::unique_ptr<IPartyCommunicationAgentFactory>>
getInMemoryAgentFactory(int numberOfParty) {
  auto maps = std::vector<std::map<
      int,
      std::shared_ptr<InMemoryPartyCommunicationAgentFactory::HostInfo>>>(
      numberOfParty);
  std::vector<std::unique_ptr<IPartyCommunicationAgentFactory>> result;

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
    result.push_back(std::make_unique<InMemoryPartyCommunicationAgentFactory>(
        i, std::move(maps[i])));
  }
  return result;
}

inline std::vector<std::unique_ptr<IPartyCommunicationAgentFactory>>
getSocketAgentFactory(int numberOfParty) {
  auto maps = std::vector<
      std::map<int, SocketPartyCommunicationAgentFactory::PartyInfo>>(
      numberOfParty);
  for (int i = 0; i < numberOfParty; i++) {
    int port = 5000 + folly::Random::rand32() % 1000;
    for (int j = i; j < numberOfParty; j++) {
      SocketPartyCommunicationAgentFactory::PartyInfo partyInfo = {
          "localhost", port};
      if (i == j) {
        continue;
      }
      maps[j].emplace(i, partyInfo);
      maps[i].emplace(j, partyInfo);
    }
  }

  std::vector<std::unique_ptr<IPartyCommunicationAgentFactory>> result;
  for (int i = 0; i < numberOfParty; i++) {
    auto m = maps[i];
    std::cerr << "entry " << i << "\n";
    for (auto const& x : m) {
      std::cerr << x.first << ":[" << x.second.address << "," << x.second.portNo
                << "]";
    }
    std::cerr << "\n";
  }

  for (int i = 0; i < numberOfParty; i++) {
    result.push_back(
        std::make_unique<SocketPartyCommunicationAgentFactory>(i, maps[i]));
  }
  return result;
}

} // namespace fbpcf::engine::communication
