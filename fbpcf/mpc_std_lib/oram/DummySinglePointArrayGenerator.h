/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include "fbpcf/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_std_lib/oram/ISinglePointArrayGenerator.h"

namespace fbpcf::mpc_std_lib::oram::insecure {

/**
 * A dummy implementation of single point array generator
 */
class DummySinglePointArrayGenerator final : public ISinglePointArrayGenerator {
 public:
  /**
   * @param thisPartyToSetIndicator whether this party or the peer party will
   * set the indicator.
   */
  DummySinglePointArrayGenerator(
      bool thisPartyToSetIndicator,
      std::unique_ptr<engine::communication::IPartyCommunicationAgent> agent)
      : thisPartyToSetIndicator_(thisPartyToSetIndicator),
        agent_(std::move(agent)) {}

  /**
   * @inherit doc
   */
  std::vector<std::pair<std::vector<bool>, std::vector<__m128i>>>
  generateSinglePointArrays(
      const std::vector<std::vector<bool>>& indexShares,
      size_t length) override;

  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    return agent_->getTrafficStatistics();
  }

 private:
  bool thisPartyToSetIndicator_;
  std::unique_ptr<engine::communication::IPartyCommunicationAgent> agent_;
};

} // namespace fbpcf::mpc_std_lib::oram::insecure
