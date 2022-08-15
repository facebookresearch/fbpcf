/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/scheduler/IScheduler.h"

namespace fbpcf::scheduler {

template <bool unsafe>
class ISchedulerFactory {
 public:
  virtual ~ISchedulerFactory() = default;

  virtual std::unique_ptr<IScheduler> create() = 0;
};

} // namespace fbpcf::scheduler
