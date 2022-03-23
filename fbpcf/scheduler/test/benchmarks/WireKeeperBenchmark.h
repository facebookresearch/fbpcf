/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Benchmark.h>

#include "fbpcf/scheduler/WireKeeper.h"

const bool unsafe = true;

namespace fbpcf::scheduler {

#define BENCHMARK_WIREKEEPER                                     \
  folly::BenchmarkSuspender braces;                              \
  auto wireKeeper = WireKeeper::createWithVectorArena<unsafe>(); \
  std::vector<IScheduler::WireId<IScheduler::Boolean>> wireIds;  \
  for (auto i = 0; i < n; i++) {                                 \
    wireIds.push_back(wireKeeper->allocateBooleanValue());       \
  }                                                              \
  braces.dismiss();                                              \
  while (n--)

} // namespace fbpcf::scheduler
