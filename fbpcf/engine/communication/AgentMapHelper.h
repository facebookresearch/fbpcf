/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <map>
#include <memory>

#include "fbpcf/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/engine/communication/IPartyCommunicationAgentFactory.h"

namespace fbpcf::engine::communication {

/**
 * Create an agent map for party with 'myId' with entries for all the other
 * parties.
 */
inline std::map<int, std::unique_ptr<IPartyCommunicationAgent>> getAgentMap(
    int numberOfParties,
    int myId,
    IPartyCommunicationAgentFactory& agentFactory) {
  std::map<int, std::unique_ptr<communication::IPartyCommunicationAgent>>
      agentMap;
  for (int i = 0; i < numberOfParties; i++) {
    if (i != myId) {
      agentMap.emplace(i, agentFactory.create(i));
    }
  }
  return agentMap;
}

} // namespace fbpcf::engine::communication
