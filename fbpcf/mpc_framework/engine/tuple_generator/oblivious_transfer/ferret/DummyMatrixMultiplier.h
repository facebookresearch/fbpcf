/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/IMatrixMultiplier.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::ferret::insecure {

/**
 * This is merely a dummy calculator - it won't actually generate any random
 * matrix for multiplication.Instead, it will only repeat the input to reach the
 * expected output length.
 */
class DummyMatrixMultiplier final : public IMatrixMultiplier {
 public:
  /**
   * @inherit doc
   */
  std::vector<__m128i> multiplyWithRandomMatrix(
      __m128i seed,
      int64_t rstLength,
      const std::vector<__m128i>& src) const override;
};

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::ferret::insecure
