/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <memory>

#include "fbpcf/engine/tuple_generator/DummyTupleGenerator.h"
#include "fbpcf/engine/tuple_generator/ITupleGeneratorFactory.h"
#include "fbpcf/engine/util/IPrgFactory.h"

namespace fbpcf::engine::tuple_generator::insecure {

/**
 * This factory creates dummy tuple generators
 */

class DummyTupleGeneratorFactory final : public ITupleGeneratorFactory {
 public:
  explicit DummyTupleGeneratorFactory(
      std::shared_ptr<fbpcf::util::MetricCollector> metricCollector)
      : ITupleGeneratorFactory(metricCollector) {}
  /**
   * Create a dummy tuple generator;
   */
  std::unique_ptr<ITupleGenerator> create() override {
    auto recorder = std::make_shared<TuplesMetricRecorder>();
    metricCollector_->addNewRecorder("tuple_generator", recorder);
    return std::make_unique<DummyTupleGenerator>(recorder);
  }
};

} // namespace fbpcf::engine::tuple_generator::insecure
