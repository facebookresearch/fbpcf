/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <memory>

#include "fbpcf/engine/tuple_generator/DummyArithmeticTupleGenerator.h"
#include "fbpcf/engine/tuple_generator/IArithmeticTupleGeneratorFactory.h"
#include "fbpcf/engine/util/IPrgFactory.h"

namespace fbpcf::engine::tuple_generator::insecure {

/**
 * This factory creates dummy tuple generators
 */

class DummyArithmeticTupleGeneratorFactory final
    : public IArithmeticTupleGeneratorFactory {
 public:
  /**
   * Create a dummy tuple generator;
   */
  std::unique_ptr<IArithmeticTupleGenerator> create() override {
    return std::make_unique<DummyArithmeticTupleGenerator>();
  }
};

} // namespace fbpcf::engine::tuple_generator::insecure
