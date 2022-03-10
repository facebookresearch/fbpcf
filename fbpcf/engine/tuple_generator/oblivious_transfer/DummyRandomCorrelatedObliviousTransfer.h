/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/engine/tuple_generator/oblivious_transfer/IFlexibleRandomCorrelatedObliviousTransfer.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::insecure {

/**
 * A Dummy Random Correlated Oblivious Transfer. It only outputs 0 vectors;
 */

class DummyRandomCorrelatedObliviousTransfer final
    : public IFlexibleRandomCorrelatedObliviousTransfer {
 public:
  explicit DummyRandomCorrelatedObliviousTransfer(
      std::unique_ptr<communication::IPartyCommunicationAgent> agent)
      : agent_(std::move(agent)) {}

  /**
   * @inherit doc
   */
  std::vector<__m128i> rcot(int64_t size) override {
    return std::vector<__m128i>(size, _mm_set_epi32(0, 0, 0, 0));
  }

  /**
   * @inherit doc
   */
  std::unique_ptr<communication::IPartyCommunicationAgent>
  extractCommunicationAgent() override {
    return std::move(agent_);
  }
  /**
   * @inherit doc
   */
  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    return {0, 0};
  }

 private:
  std::unique_ptr<communication::IPartyCommunicationAgent> agent_;
};

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::insecure
