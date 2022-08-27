/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include "fbpcf/engine/tuple_generator/IArithmeticTupleGenerator.h"

namespace fbpcf::engine::tuple_generator {

/**
 A null integer tuple generator, throws exception when the secret share engine
 tries to generate integer tuples with this generator
 */
class NullArithmeticTupleGenerator final : public IArithmeticTupleGenerator {
 public:
  std::vector<IntegerTuple> getIntegerTuple(uint32_t size) override {
    if (size == 0) {
      return std::vector<IntegerTuple>();
    }
    throw std::runtime_error(
        "The secret share engine is not configured with integer tuple generator.");
  }

  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    return {0, 0};
  }
};

} // namespace fbpcf::engine::tuple_generator
