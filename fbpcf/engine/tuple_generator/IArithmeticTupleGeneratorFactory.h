/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <memory>

#include "fbpcf/engine/tuple_generator/IArithmeticTupleGenerator.h"

namespace fbpcf::engine::tuple_generator {

/**
 * This is the API for an arithmetic tuple generator that generates integer
 * tuples.
 */

class IArithmeticTupleGeneratorFactory {
 public:
  virtual ~IArithmeticTupleGeneratorFactory() = default;

  /**
   * Create a tuple generator with all party.
   */
  virtual std::unique_ptr<IArithmeticTupleGenerator> create() = 0;
};

} // namespace fbpcf::engine::tuple_generator
