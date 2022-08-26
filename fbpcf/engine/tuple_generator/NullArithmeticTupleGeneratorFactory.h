/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <memory>

#include "fbpcf/engine/tuple_generator/IArithmeticTupleGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/NullArithmeticTupleGenerator.h"

namespace fbpcf::engine::tuple_generator {

/**
 * This factory creates null tuple generators
 */

class NullArithmeticTupleGeneratorFactory final
    : public IArithmeticTupleGeneratorFactory {
 public:
  /**
   * Create a null tuple generator;
   */
  std::unique_ptr<IArithmeticTupleGenerator> create() override {
    return std::make_unique<NullArithmeticTupleGenerator>();
  }
};

} // namespace fbpcf::engine::tuple_generator
