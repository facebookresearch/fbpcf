/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <memory>

#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/IProductShareGenerator.h"

namespace fbpcf::engine::tuple_generator::insecure {

/**
 * This is a Dummy product shares generator. It will generate shares with
 * plaintext communication.
 */

class DummyProductShareGenerator final : public IProductShareGenerator {
 public:
  explicit DummyProductShareGenerator(
      std::unique_ptr<communication::IPartyCommunicationAgent> agent)
      : agent_{std::move(agent)} {}

  /**
   * @inherit doc
   */
  std::vector<bool> generateBooleanProductShares(
      const std::vector<bool>& left,
      const std::vector<bool>& right) override;

  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    return agent_->getTrafficStatistics();
  }

 private:
  std::unique_ptr<communication::IPartyCommunicationAgent> agent_;
};

} // namespace fbpcf::engine::tuple_generator::insecure
