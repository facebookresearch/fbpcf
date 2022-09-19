/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <memory>

#include "fbpcf/engine/tuple_generator/ITupleGenerator.h"
#include "fbpcf/util/MetricCollector.h"

namespace fbpcf::engine::tuple_generator {

/**
 * This is the API for a tuple generator that generates tuples.
 */

class ITupleGeneratorFactory {
 public:
  explicit ITupleGeneratorFactory(
      std::shared_ptr<fbpcf::util::MetricCollector> metricCollector)
      : metricCollector_(metricCollector) {}
  virtual ~ITupleGeneratorFactory() = default;

  /**
   * Create a tuple generator with all party.
   */
  virtual std::unique_ptr<ITupleGenerator> create() = 0;

 protected:
  std::shared_ptr<fbpcf::util::MetricCollector> metricCollector_;
};

} // namespace fbpcf::engine::tuple_generator
