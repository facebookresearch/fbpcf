/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/engine/ISecretShareEngineFactory.h"
#include "fbpcf/scheduler/EagerScheduler.h"
#include "fbpcf/scheduler/ISchedulerFactory.h"
#include "fbpcf/scheduler/WireKeeper.h"

namespace fbpcf::scheduler {

template <bool unsafe>
class EagerSchedulerFactory final : public ISchedulerFactory<unsafe> {
 public:
  EagerSchedulerFactory(engine::ISecretShareEngineFactory& engineFactory)
      : engineFactory_(engineFactory) {}

  std::unique_ptr<IScheduler> create() override {
    return std::make_unique<EagerScheduler>(
        engineFactory_.create(), WireKeeper::createWithVectorArena<unsafe>());
  }

 private:
  engine::ISecretShareEngineFactory& engineFactory_;
};

} // namespace fbpcf::scheduler
