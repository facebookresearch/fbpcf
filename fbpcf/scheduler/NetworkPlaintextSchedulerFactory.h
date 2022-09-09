/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/engine/communication/AgentMapHelper.h"
#include "fbpcf/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/scheduler/ISchedulerFactory.h"
#include "fbpcf/scheduler/NetworkPlaintextScheduler.h"
#include "fbpcf/scheduler/WireKeeper.h"

namespace fbpcf::scheduler {

template <bool unsafe>
class NetworkPlaintextSchedulerFactory final
    : public ISchedulerFactory<unsafe> {
 public:
  NetworkPlaintextSchedulerFactory(
      int myId,
      engine::communication::IPartyCommunicationAgentFactory&
          communicationAgentFactory)
      : ISchedulerFactory<unsafe>(
            std::make_shared<fbpcf::util::MetricCollector>(
                "network_plaintext_scheduler")),
        myId_(myId),
        communicationAgentFactory_(communicationAgentFactory) {}

  NetworkPlaintextSchedulerFactory(
      int myId,
      engine::communication::IPartyCommunicationAgentFactory&
          communicationAgentFactory,
      std::shared_ptr<fbpcf::util::MetricCollector> metricCollector)
      : ISchedulerFactory<unsafe>(metricCollector),
        myId_(myId),
        communicationAgentFactory_(communicationAgentFactory) {}

  std::unique_ptr<IScheduler> create() override {
    auto numberOfParties = 2;
    auto agentMap = engine::communication::getAgentMap(
        numberOfParties, myId_, communicationAgentFactory_);

    return std::make_unique<NetworkPlaintextScheduler>(
        myId_,
        std::move(agentMap),
        WireKeeper::createWithVectorArena<unsafe>());
  }

 private:
  int myId_;
  engine::communication::IPartyCommunicationAgentFactory&
      communicationAgentFactory_;
};

} // namespace fbpcf::scheduler
