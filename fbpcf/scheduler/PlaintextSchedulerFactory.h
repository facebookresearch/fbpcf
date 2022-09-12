/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/scheduler/IScheduler.h"
#include "fbpcf/scheduler/ISchedulerFactory.h"
#include "fbpcf/scheduler/PlaintextScheduler.h"
#include "fbpcf/scheduler/WireKeeper.h"

namespace fbpcf::scheduler {

template <bool unsafe>
class PlaintextSchedulerFactory final : public ISchedulerFactory<unsafe> {
 public:
  PlaintextSchedulerFactory()
      : ISchedulerFactory<unsafe>(
            std::make_shared<fbpcf::util::MetricCollector>(
                "plaintext_scheduler")) {}

  PlaintextSchedulerFactory(
      std::shared_ptr<fbpcf::util::MetricCollector> metricCollector)
      : ISchedulerFactory<unsafe>(metricCollector) {}
  std::unique_ptr<IScheduler> create() override {
    return std::make_unique<PlaintextScheduler>(
        WireKeeper::createWithVectorArena<unsafe>());
  }
};

} // namespace fbpcf::scheduler
