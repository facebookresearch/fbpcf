/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include "fbpcf/engine/tuple_generator/ITupleGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/NullTupleGenerator.h"

namespace fbpcf::engine::tuple_generator {

/**
 * This factory creates null tuple generators
 */

class NullTupleGeneratorFactory final : public ITupleGeneratorFactory {
 public:
  explicit NullTupleGeneratorFactory(
      std::shared_ptr<fbpcf::util::MetricCollector> metricCollector)
      : ITupleGeneratorFactory(metricCollector) {}
  /**
   * Create a null tuple generator;
   */
  std::unique_ptr<ITupleGenerator> create() override {
    return std::make_unique<NullTupleGenerator>();
  }
};

} // namespace fbpcf::engine::tuple_generator
