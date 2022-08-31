/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include "fbpcf/engine/tuple_generator/ITupleGenerator.h"

namespace fbpcf::engine::tuple_generator {

/**
 A null boolean tuple generator, throws exception when the secret share engine
 tries to generate boolean tuples with this generator
 */
class NullTupleGenerator final : public ITupleGenerator {
 public:
  std::vector<BooleanTuple> getBooleanTuple(uint32_t size) override {
    if (size == 0) {
      return std::vector<BooleanTuple>();
    }
    throw std::runtime_error(
        "The secret share engine is not configured with boolean tuple generator.");
  }

  /**
   * @inherit doc
   */
  std::map<size_t, std::vector<CompositeBooleanTuple>> getCompositeTuple(
      const std::map<size_t, uint32_t>& tupleSizes) override {
    if (tupleSizes.empty()) {
      return std::map<size_t, std::vector<CompositeBooleanTuple>>();
    }
    throw std::runtime_error(
        "The secret share engine is not configured with boolean tuple generator.");
  }

  /**
   * @inherit doc
   */
  std::pair<
      std::vector<BooleanTuple>,
      std::map<size_t, std::vector<CompositeBooleanTuple>>>
  getNormalAndCompositeBooleanTuples(
      uint32_t tupleSizes,
      const std::map<size_t, uint32_t>& compositeTupleSizes) override {
    if (tupleSizes == 0 && compositeTupleSizes.empty()) {
      return std::pair<
          std::vector<BooleanTuple>,
          std::map<size_t, std::vector<CompositeBooleanTuple>>>();
    }
    throw std::runtime_error(
        "The secret share engine is not configured with boolean tuple generator.");
  }

  /**
   * @inherit doc
   */
  bool supportsCompositeTupleGeneration() override {
    return false;
  }

  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    throw std::runtime_error(
        "The secret share engine is not configured with boolean tuple generator.");
  }
};

} // namespace fbpcf::engine::tuple_generator
