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

BENCHMARK(WireKeeperBenchmark_allocateBooleanValue, n) {
  folly::BenchmarkSuspender braces;
  auto wireKeeper = WireKeeper::createWithVectorArena<unsafe>();
  braces.dismiss();

  while (n--) {
    wireKeeper->allocateBooleanValue();
  }
}

BENCHMARK(WireKeeperBenchmark_getBooleanValue, n) {
  BENCHMARK_WIREKEEPER {
    wireKeeper->getBooleanValue(wireIds.at(n));
  }
}

BENCHMARK(WireKeeperBenchmark_setBooleanValue, n) {
  BENCHMARK_WIREKEEPER {
    wireKeeper->setBooleanValue(wireIds.at(n), n & 1);
  }
}

BENCHMARK(WireKeeperBenchmark_getFirstAvailableLevel, n) {
  BENCHMARK_WIREKEEPER {
    wireKeeper->getFirstAvailableLevel(wireIds.at(n));
  }
}

BENCHMARK(WireKeeperBenchmark_setFirstAvailableLevel, n) {
  BENCHMARK_WIREKEEPER {
    wireKeeper->setFirstAvailableLevel(wireIds.at(n), n);
  }
}

BENCHMARK(WireKeeperBenchmark_increaseReferenceCount, n) {
  BENCHMARK_WIREKEEPER {
    wireKeeper->increaseReferenceCount(wireIds.at(n));
  }
}

BENCHMARK(WireKeeperBenchmark_decreaseReferenceCount, n) {
  BENCHMARK_WIREKEEPER {
    wireKeeper->decreaseReferenceCount(wireIds.at(n));
  }
}
} // namespace fbpcf::scheduler
