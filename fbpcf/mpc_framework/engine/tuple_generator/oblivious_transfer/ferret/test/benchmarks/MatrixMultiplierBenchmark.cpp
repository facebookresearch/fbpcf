/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>
#include <random>

#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/test/benchmarks/MatrixMultiplierBenchmark.h"

namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer::
    ferret {

void MatrixMultiplierBenchmark::setup(
    std::unique_ptr<IMatrixMultiplierFactory> factory,
    int64_t rstLength,
    int64_t srcLength) {
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);

  multiplier_ = factory->create();
  rstLength_ = rstLength;
  // for benchmark, the concrete value doesn't matter;
  seed_ = _mm_set_epi64x(dist(e), dist(e));
  src_ = std::vector<__m128i>(srcLength);
  for (auto& item : src_) {
    item = _mm_set_epi64x(dist(e), dist(e));
  }
}

std::vector<__m128i> MatrixMultiplierBenchmark::benchmark() const {
  return multiplier_->multiplyWithRandomMatrix(seed_, rstLength_, src_);
}

} // namespace
  // fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer::ferret
