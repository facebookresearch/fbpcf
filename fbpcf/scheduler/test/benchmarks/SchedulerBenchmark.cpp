/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Benchmark.h>

#include "common/init/Init.h"

#include "fbpcf/scheduler/test/benchmarks/AllocatorBenchmark.h"
#include "fbpcf/scheduler/test/benchmarks/WireKeeperBenchmark.h"

namespace fbpcf::scheduler {

// IAllocator benchmarks

BENCHMARK(VectorArenaAllocator_allocate, n) {
  benchmarkAllocate<int64_t>(
      std::make_unique<VectorArenaAllocator<int64_t, unsafe>>(), n);
}

BENCHMARK(VectorArenaAllocator_free, n) {
  benchmarkFree<int64_t>(
      std::make_unique<VectorArenaAllocator<int64_t, unsafe>>(), n);
}

BENCHMARK(VectorArenaAllocator_get, n) {
  benchmarkGet<int64_t>(
      std::make_unique<VectorArenaAllocator<int64_t, unsafe>>(), n);
}

BENCHMARK(UnorderedMapAllocator_allocate, n) {
  benchmarkAllocate<int64_t>(
      std::make_unique<UnorderedMapAllocator<int64_t>>(), n);
}

BENCHMARK(UnorderedMapAllocator_free, n) {
  benchmarkFree<int64_t>(std::make_unique<UnorderedMapAllocator<int64_t>>(), n);
}

BENCHMARK(UnorderedMapAllocator_get, n) {
  benchmarkGet<int64_t>(std::make_unique<UnorderedMapAllocator<int64_t>>(), n);
}

// WireKeeper benchmarks

BENCHMARK(WireKeeperBenchmark_allocateBooleanValue, n) {
  BENCHMARK_WIREKEEPER {
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

int main(int argc, char* argv[]) {
  facebook::initFacebook(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
