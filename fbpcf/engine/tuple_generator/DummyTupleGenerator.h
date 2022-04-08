/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include "fbpcf/engine/tuple_generator/ITupleGenerator.h"

namespace fbpcf::engine::tuple_generator::insecure {

/**
 A dummy boolean tuple generator, always generate tuple (0, 0,0 )
 */
class DummyTupleGenerator final : public ITupleGenerator {
 public:
  std::vector<BooleanTuple> getBooleanTuple(uint32_t size) override {
    std::vector<BooleanTuple> result;
    for (size_t i = 0; i < size; i++) {
      result.push_back(BooleanTuple(0, 0, 0));
    }
    return result;
  }

  /**
   * @inherit doc
   */
  std::map<size_t, std::vector<CompositeBooleanTuple>> getCompositeTuple(
      const std::map<size_t, uint32_t>& tupleSizes) override {
    std::map<size_t, std::vector<CompositeBooleanTuple>> result;
    for (auto& countOfTuples : tupleSizes) {
      size_t tupleSize = countOfTuples.first;
      uint32_t tupleCount = countOfTuples.second;

      result.emplace(tupleSize, std::vector<CompositeBooleanTuple>(tupleCount));
      for (int i = 0; i < tupleCount; i++) {
        result.at(tupleSize).at(i) = CompositeBooleanTuple(
            0,
            std::vector<bool>(tupleSize, 0),
            std::vector<bool>(tupleSize, 0));
      }
    }

    return result;
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
    auto boolResult = getBooleanTuple(tupleSizes);
    auto compositeBoolResult = getCompositeTuple(compositeTupleSizes);
    return std::make_pair(
        std::move(boolResult), std::move(compositeBoolResult));
  }

  /**
   * @inherit doc
   */
  bool supportsCompositeTupleGeneration() override {
    return true;
  }

  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    return {0, 0};
  }
};

} // namespace fbpcf::engine::tuple_generator::insecure
