/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <memory>

#include "fbpcf/mpc_framework/engine/tuple_generator/DummyTupleGenerator.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/ITupleGeneratorFactory.h"
#include "fbpcf/mpc_framework/engine/util/IPrgFactory.h"

namespace fbpcf::engine::tuple_generator::insecure {

/**
 * This factory creates dummy tuple generators
 */

class DummyTupleGeneratorFactory final : public ITupleGeneratorFactory {
 public:
  /**
   * Create a dummy tuple generator;
   */
  std::unique_ptr<ITupleGenerator> create() override {
    return std::make_unique<DummyTupleGenerator>();
  }
};

} // namespace fbpcf::engine::tuple_generator::insecure
