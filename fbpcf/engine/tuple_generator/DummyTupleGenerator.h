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
    for (int i = 0; i < size; i++) {
      result.push_back(BooleanTuple(0, 0, 0));
    }
    return result;
  }

  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    return {0, 0};
  }
};

} // namespace fbpcf::engine::tuple_generator::insecure
