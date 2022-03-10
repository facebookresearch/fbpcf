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
#include <random>
#include <stdexcept>

#include "fbpcf/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/engine/communication/SocketPartyCommunicationAgentFactory.h"
#include "folly/logging/xlog.h"

namespace fbpcf::engine::util {

inline std::pair<
    std::unique_ptr<communication::IPartyCommunicationAgent>,
    std::unique_ptr<communication::IPartyCommunicationAgent>>
getSocketAgents() {
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<int> intDistro(10000, 25000);

  // Since we're using random port numbers, there's a chance we'll encounter a
  // bind error. Add some retries to reduce the flakiness.
  auto retries = 5;
  while (retries--) {
    try {
      std::map<
          int,
          communication::SocketPartyCommunicationAgentFactory::PartyInfo>
          partyInfo = {
              {0, {"127.0.0.1", intDistro(e)}},
              {1, {"127.0.0.1", intDistro(e)}}};
      auto factory0 =
          std::make_unique<communication::SocketPartyCommunicationAgentFactory>(
              0, partyInfo);
      auto factory1 =
          std::make_unique<communication::SocketPartyCommunicationAgentFactory>(
              1, partyInfo);

      auto task =
          [](std::unique_ptr<communication::IPartyCommunicationAgentFactory>
                 factory,
             int myId) { return factory->create(1 - myId); };

      auto createSocketAgent0 = std::async(task, std::move(factory0), 0);
      auto createSocketAgent1 = std::async(task, std::move(factory1), 1);

      auto agent0 = createSocketAgent0.get();
      auto agent1 = createSocketAgent1.get();

      return {std::move(agent0), std::move(agent1)};
    } catch (...) {
      XLOG(INFO) << "Failed to create socket agents. " << retries
                 << " retries remaining.";
    }
  }

  throw std::runtime_error("Failed to create socket agents. Out of retries.");
}

} // namespace fbpcf::engine::util
