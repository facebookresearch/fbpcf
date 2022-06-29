/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <assert.h>
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>
#include "fbpcf/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/engine/communication/InMemoryPartyCommunicationAgentHost.h"

namespace fbpcf::engine::communication {

/**
 * An communication factory API
 */
class InMemoryPartyCommunicationAgentFactory final
    : public IPartyCommunicationAgentFactory {
 public:
  struct HostInfo {
    std::unique_ptr<std::mutex> mutex;
    std::vector<std::unique_ptr<InMemoryPartyCommunicationAgentHost>> hosts;
  };

  InMemoryPartyCommunicationAgentFactory(
      int myId,
      std::map<int, std::shared_ptr<HostInfo>>&& sharedHosts)
      : IPartyCommunicationAgentFactory("in_memory_traffic"),
        myId_(myId),
        sharedHosts_(sharedHosts) {
    for (auto& item : sharedHosts_) {
      createdAgentCount_.emplace(item.first, 0);
    }
  }

  /**
   * @inherit doc
   */
  std::unique_ptr<IPartyCommunicationAgent> create(int id, std::string)
      override {
    if (sharedHosts_.find(id) == sharedHosts_.end() ||
        createdAgentCount_.find(id) == createdAgentCount_.end()) {
      throw std::runtime_error(" can't connect to this party!");
    }

    int index = createdAgentCount_[id]++;

    auto& pair = *sharedHosts_.find(id)->second;

    pair.mutex->lock();
    if (pair.hosts.size() < index) {
      throw std::runtime_error("unexpected situation!");
    }
    if (pair.hosts.size() == index) {
      pair.hosts.push_back(
          std::make_unique<InMemoryPartyCommunicationAgentHost>());
    }

    assert(pair.hosts.size() >= index + 1);
    auto agent = pair.hosts[index]->getAgent(myId_ < id ? 0 : 1);
    pair.mutex->unlock();
    return agent;
  }

 private:
  int myId_;
  std::map<int, std::shared_ptr<HostInfo>> sharedHosts_;
  std::map<int, int> createdAgentCount_;
};

} // namespace fbpcf::engine::communication
