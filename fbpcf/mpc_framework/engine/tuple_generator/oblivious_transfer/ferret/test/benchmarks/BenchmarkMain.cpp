/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Benchmark.h>
#include <vector>
#include "common/init/Init.h"

#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/DummyMatrixMultiplierFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/TenLocalLinearMatrixMultiplierFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/test/benchmarks/CotBenchmark.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/test/benchmarks/MatrixMultiplierBenchmark.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::ferret {

BENCHMARK(DummyMatrixMultiplier, n) {
  benchmarkMatrixMultiplier(
      std::make_unique<insecure::DummyMatrixMultiplierFactory>(), n);
}

BENCHMARK(TenLocalLinearMatrixMultiplier, n) {
  benchmarkMatrixMultiplier(
      std::make_unique<TenLocalLinearMatrixMultiplierFactory>(), n);
}

BENCHMARK_COUNTERS(SinglePointCot, counters) {
  SinglePointCotBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

BENCHMARK_COUNTERS(RegularErrorMultiPointCot, counters) {
  RegularErrorMultiPointCotBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

BENCHMARK_COUNTERS(RcotExtender, counters) {
  RcotExtenderBenchmark benchmark;
  benchmark.runBenchmark(counters);
}
} // namespace fbpcf::engine::tuple_generator::oblivious_transfer::ferret

int main(int argc, char* argv[]) {
  facebook::initFacebook(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
