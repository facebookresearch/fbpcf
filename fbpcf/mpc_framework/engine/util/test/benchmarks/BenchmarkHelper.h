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

#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_framework/engine/communication/SocketPartyCommunicationAgentFactory.h"

namespace fbpcf::mpc_framework::engine::util {

inline std::pair<
    std::unique_ptr<communication::IPartyCommunicationAgent>,
    std::unique_ptr<communication::IPartyCommunicationAgent>>
getSocketAgents() {
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<int> intDistro(10000, 25000);

  std::map<int, communication::SocketPartyCommunicationAgentFactory::PartyInfo>
      partyInfo = {
          {0, {"127.0.0.1", intDistro(e)}}, {1, {"127.0.0.1", intDistro(e)}}};
  auto factory0 =
      std::make_unique<communication::SocketPartyCommunicationAgentFactory>(
          0, partyInfo);
  auto factory1 =
      std::make_unique<communication::SocketPartyCommunicationAgentFactory>(
          1, partyInfo);

  auto task = [](std::unique_ptr<communication::IPartyCommunicationAgentFactory>
                     factory,
                 int myId) { return factory->create(1 - myId); };

  auto createSocketAgent0 = std::async(task, std::move(factory0), 0);
  auto createSocketAgent1 = std::async(task, std::move(factory1), 1);

  auto agent0 = createSocketAgent0.get();
  auto agent1 = createSocketAgent1.get();

  return {std::move(agent0), std::move(agent1)};
}

} // namespace fbpcf::mpc_framework::engine::util
