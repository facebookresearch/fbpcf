/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include "fbpcf/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_std_lib/oram/IObliviousDeltaCalculator.h"

namespace fbpcf::mpc_std_lib::oram::insecure {

/**
 * A dummy implementation of oblivious delta calculator
 */
class DummyObliviousDeltaCalculator final : public IObliviousDeltaCalculator {
 public:
  explicit DummyObliviousDeltaCalculator(
      std::unique_ptr<engine::communication::IPartyCommunicationAgent> agent)
      : agent_(std::move(agent)) {}

  /**
   * @inherit doc
   */
  std::tuple<std::vector<__m128i>, std::vector<bool>, std::vector<bool>>
  calculateDelta(
      const std::vector<__m128i>& delta0Shares,
      const std::vector<__m128i>& delta1Shares,
      const std::vector<bool>& alphaShares) const override;

  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    return agent_->getTrafficStatistics();
  }

 private:
  std::unique_ptr<engine::communication::IPartyCommunicationAgent> agent_;
};

} // namespace fbpcf::mpc_std_lib::oram::insecure
