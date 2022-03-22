/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Benchmark.h>

#include "fbpcf/scheduler/UnorderedMapAllocator.h"
#include "fbpcf/scheduler/VectorArenaAllocator.h"

namespace fbpcf::scheduler {

const bool unsafe = true;

template <typename T>
inline void benchmarkAllocate(std::unique_ptr<IAllocator<T>> allocator, int n) {
  while (n--) {
    allocator->allocate(n);
  }
}

template <typename T>
inline void benchmarkFree(std::unique_ptr<IAllocator<T>> allocator, int n) {
  uint64_t ref;
  BENCHMARK_SUSPEND {
    auto count = n;
    while (count--) {
      ref = allocator->allocate(count);
    }
  }

  while (n--) {
    allocator->free(ref--);
  }
}

template <typename T>
inline void benchmarkGet(std::unique_ptr<IAllocator<T>> allocator, int n) {
  uint64_t ref;
  BENCHMARK_SUSPEND {
    auto count = n;
    while (count--) {
      ref = allocator->allocate(count);
    }
  }

  while (n--) {
    allocator->get(ref--);
  }
}

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
} // namespace fbpcf::scheduler
