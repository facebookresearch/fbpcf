/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Benchmark.h>
#include <folly/stop_watch.h>
#include <memory>

#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/IMatrixMultiplierFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/RegularErrorMultiPointCot.h"
#include "fbpcf/mpc_framework/engine/util/AesPrgFactory.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::ferret {

class MatrixMultiplierBenchmark {
 public:
  /**
   * a helper to setup benchmark all matrix multiplier
   * @param factory the factory to create the concrete multiplier
   * @param rstLength the desired output size
   * @param srcLength the input size
   * @return time spent for the multiplication in ms
   */
  void setup(
      std::unique_ptr<IMatrixMultiplierFactory> factory,
      int64_t rstLength,
      int64_t srcLength);

  std::vector<__m128i> benchmark() const;

 private:
  std::unique_ptr<IMatrixMultiplier> multiplier_;
  __m128i seed_;
  int64_t rstLength_;
  std::vector<__m128i> src_;
};

inline void benchmarkMatrixMultiplier(
    std::unique_ptr<IMatrixMultiplierFactory> factory,
    uint64_t n) {
  MatrixMultiplierBenchmark matrixMultiplierBenchmark;
  BENCHMARK_SUSPEND {
    matrixMultiplierBenchmark.setup(
        std::move(factory), kExtendedSize, kBaseSize);
  }

  std::vector<__m128i> rst;
  while (n--) {
    rst = matrixMultiplierBenchmark.benchmark();
  }
  folly::doNotOptimizeAway(rst);
}
} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::ferret
