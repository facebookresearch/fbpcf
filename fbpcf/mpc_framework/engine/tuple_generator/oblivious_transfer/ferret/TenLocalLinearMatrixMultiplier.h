/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/IMatrixMultiplier.h"
#include "fbpcf/mpc_framework/engine/util/IPrgFactory.h"

namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer::
    ferret {

/**
 * This lpn calculator uses a built-in 10 local linear code generator.
 * With a given seed, exactly 10 items from src (potentially with duplication)
 * are selected to compose 1 result item.
 */
class TenLocalLinearMatrixMultiplier final : public IMatrixMultiplier {
 public:
  TenLocalLinearMatrixMultiplier() {}

  /**
   * @inherit doc
   */
  std::vector<__m128i> multiplyWithRandomMatrix(
      __m128i seed,
      int64_t rstLength,
      const std::vector<__m128i>& src) const override;
};

} // namespace
  // fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer::ferret
