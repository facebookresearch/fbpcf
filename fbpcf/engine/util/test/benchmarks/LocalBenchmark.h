/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Benchmark.h>

namespace fbpcf::engine::util {

/**
 * A parent class for local benchmarks that don't need to measure network use.
 */
class LocalBenchmark {
 public:
  virtual ~LocalBenchmark() = default;

  void runBenchmark(unsigned int n) {
    BENCHMARK_SUSPEND {
      setup();
    }

    run(n);

    BENCHMARK_SUSPEND {
      teardown();
    }
  }

 protected:
  virtual void setup() {}
  virtual void teardown() {}
  virtual void run(unsigned int n) = 0;
};

} // namespace fbpcf::engine::util
