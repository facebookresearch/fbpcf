/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <iterator>
#include "fbpcf/mpc_framework/engine/communication/AgentMapHelper.h"
#include "fbpcf/mpc_framework/engine/communication/InMemoryPartyCommunicationAgentHost.h"
#include "fbpcf/mpc_framework/engine/communication/SecretShareEngineCommunicationAgent.h"
#include "fbpcf/mpc_framework/engine/communication/test/AgentFactoryCreationHelper.h"

namespace fbpcf::engine::communication {

class SecretShareEngineCommunicationAgentTestHelper {
 public:
  std::vector<std::unique_ptr<SecretShareEngineCommunicationAgent>>
  createAgents(int numberOfAgents) {
    auto factories = getInMemoryAgentFactory(numberOfAgents);
    auto startingIndex = factories_.size();
    factories_.insert(
        factories_.end(),
        std::make_move_iterator(factories.begin()),
        std::make_move_iterator(factories.end()));

    std::vector<std::unique_ptr<SecretShareEngineCommunicationAgent>> rst;
    for (int i = 0; i < numberOfAgents; i++) {
      auto agentMap =
          getAgentMap(numberOfAgents, i, *factories_.at(startingIndex + i));
      rst.push_back(std::make_unique<SecretShareEngineCommunicationAgent>(
          i, std::move(agentMap)));
    }
    return rst;
  }

 private:
  std::vector<std::unique_ptr<IPartyCommunicationAgentFactory>> factories_;
};

} // namespace fbpcf::engine::communication
