/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/engine/ISecretShareEngineFactory.h"
#include "fbpcf/engine/SecretShareEngineFactory.h"
#include "fbpcf/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/scheduler/ISchedulerFactory.h"
#include "fbpcf/scheduler/LazyScheduler.h"
#include "fbpcf/scheduler/WireKeeper.h"
#include "fbpcf/scheduler/gate_keeper/GateKeeper.h"
#include "fbpcf/util/MetricCollector.h"

namespace fbpcf::scheduler {

template <bool unsafe>
class LazySchedulerFactory final : public ISchedulerFactory<unsafe> {
 public:
  LazySchedulerFactory(
      std::unique_ptr<fbpcf::engine::ISecretShareEngineFactory> engineFactory)
      : ISchedulerFactory<unsafe>(
            std::make_shared<fbpcf::util::MetricCollector>("lazy_scheduler")),
        engineFactory_(std::move(engineFactory)) {}

  LazySchedulerFactory(
      std::unique_ptr<fbpcf::engine::ISecretShareEngineFactory> engineFactory,
      std::shared_ptr<fbpcf::util::MetricCollector> metricCollector)
      : ISchedulerFactory<unsafe>(metricCollector),
        engineFactory_(std::move(engineFactory)) {}

  std::unique_ptr<IScheduler> create() override {
    std::shared_ptr<IWireKeeper> wireKeeper =
        WireKeeper::createWithVectorArena<unsafe>();

    return std::make_unique<LazyScheduler>(
        engineFactory_->create(),
        wireKeeper,
        std::make_unique<GateKeeper>(wireKeeper));
  }

 private:
  std::unique_ptr<fbpcf::engine::ISecretShareEngineFactory> engineFactory_;
};

template <bool unsafe>
inline std::unique_ptr<LazySchedulerFactory<unsafe>>
getLazySchedulerFactoryWithInsecureEngine(
    int myId,
    engine::communication::IPartyCommunicationAgentFactory&
        communicationAgentFactory,
    std::shared_ptr<fbpcf::util::MetricCollector> metricCollector =
        std::make_shared<fbpcf::util::MetricCollector>(
            "default_metric_collector")) {
  std::unique_ptr<engine::ISecretShareEngineFactory> engineFactory =
      engine::getInsecureEngineFactoryWithDummyTupleGenerator(
          myId, 2, communicationAgentFactory);

  return std::make_unique<LazySchedulerFactory<unsafe>>(
      std::move(engineFactory), metricCollector);
}

inline std::unique_ptr<LazySchedulerFactory</* unsafe */ true>>
getLazySchedulerFactoryWithClassicOT(
    int myId,
    engine::communication::IPartyCommunicationAgentFactory&
        communicationAgentFactory,
    std::shared_ptr<fbpcf::util::MetricCollector> metricCollector =
        std::make_shared<fbpcf::util::MetricCollector>(
            "default_metric_collector")) {
  std::unique_ptr<engine::ISecretShareEngineFactory> engineFactory =
      engine::getSecureEngineFactoryWithClassicOt(
          myId, 2, communicationAgentFactory);

  return std::make_unique<LazySchedulerFactory</* unsafe */ true>>(
      std::move(engineFactory), metricCollector);
}

inline std::unique_ptr<LazySchedulerFactory</* unsafe */ true>>
getLazySchedulerFactoryWithRealEngine(
    int myId,
    engine::communication::IPartyCommunicationAgentFactory&
        communicationAgentFactory,
    std::shared_ptr<fbpcf::util::MetricCollector> metricCollector =
        std::make_shared<fbpcf::util::MetricCollector>(
            "default_metric_collector")) {
  std::unique_ptr<engine::ISecretShareEngineFactory> engineFactory =
      engine::getSecureEngineFactoryWithFERRET(
          myId, 2, communicationAgentFactory);

  return std::make_unique<LazySchedulerFactory</* unsafe */ true>>(
      std::move(engineFactory), metricCollector);
}

} // namespace fbpcf::scheduler
