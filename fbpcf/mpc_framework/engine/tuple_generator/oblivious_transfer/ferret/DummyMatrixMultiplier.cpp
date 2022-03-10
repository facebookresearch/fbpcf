/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/DummyMatrixMultiplier.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::ferret::insecure {

std::vector<__m128i> DummyMatrixMultiplier::multiplyWithRandomMatrix(
    __m128i /*seed*/,
    int64_t rstLength,
    const std::vector<__m128i>& src) const {
  std::vector<__m128i> rst(rstLength);
  for (int i = 0; i < rstLength; i++) {
    rst[i] = src[i % src.size()];
  }
  return rst;
}

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::ferret::insecure
