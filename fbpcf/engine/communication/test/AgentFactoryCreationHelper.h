/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include "fbpcf/engine/communication/InMemoryPartyCommunicationAgentFactory.h"

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

} // namespace fbpcf::engine::communication
