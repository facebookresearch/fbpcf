/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/engine/ISecretShareEngineFactory.h"
#include "fbpcf/scheduler/ISchedulerFactory.h"
#include "fbpcf/scheduler/LazyScheduler.h"
#include "fbpcf/scheduler/WireKeeper.h"
#include "fbpcf/scheduler/gate_keeper/GateKeeper.h"

namespace fbpcf::scheduler {

template <bool unsafe>
class LazySchedulerFactory final : public ISchedulerFactory<unsafe> {
 public:
  LazySchedulerFactory(engine::ISecretShareEngineFactory& engineFactory)
      : engineFactory_(engineFactory) {}

  std::unique_ptr<IScheduler> create() override {
    std::shared_ptr<IWireKeeper> wireKeeper =
        WireKeeper::createWithVectorArena<unsafe>();

    return std::make_unique<LazyScheduler>(
        engineFactory_.create(),
        wireKeeper,
        std::make_unique<GateKeeper>(wireKeeper));
  }

 private:
  engine::ISecretShareEngineFactory& engineFactory_;
};

} // namespace fbpcf::scheduler
