/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <vector>
#include "fbpcf/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_std_lib/oram/IDifferenceCalculator.h"

namespace fbpcf::mpc_std_lib::oram::insecure {

/**
 * See design doc
 * here:https://docs.google.com/document/d/1834nhZzUAR1vYWWdGXB4vKh-U-uYfb9hyel3kA1QtRE/edit?usp=sharing
 * a Difference Calculator computes the difference between a randomly generated
 * value and a desired one, multiplied by a sign indicatorShares
 * (positive/negative one).
 */
template <typename T, int8_t indicatorSumWidth>
class DummyDifferenceCalculator final : public IDifferenceCalculator<T> {
 public:
  DummyDifferenceCalculator(
      bool thisPartyToSetDifference,
      std::unique_ptr<engine::communication::IPartyCommunicationAgent> agent)
      : thisPartyToSetDifference_(thisPartyToSetDifference),
        agent_(std::move(agent)) {}

  /**
   * @inherit doc
   */
  std::vector<T> calculateDifferenceBatch(
      const std::vector<uint32_t>& indicatorShares,
      const std::vector<std::vector<bool>>& minuendShares,
      const std::vector<T>& subtrahendShares) const override;

  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    return agent_->getTrafficStatistics();
  }

 private:
  bool thisPartyToSetDifference_;
  mutable std::unique_ptr<engine::communication::IPartyCommunicationAgent>
      agent_;
};

} // namespace fbpcf::mpc_std_lib::oram::insecure

#include "fbpcf/mpc_std_lib/oram/DummyDifferenceCalculator_impl.h"
