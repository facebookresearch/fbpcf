/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include "fbpcf/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/IBaseObliviousTransfer.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::insecure {

/**
 * This is a dummy base oblivious transfer object. An ideal object will realize
 * the desired functionality in an insecure way. Usually they are added as:
  1. baseline for correctness;
  2. a place holder for integration test (if we only want to test some of the
 objects)
  3. a helper when testing other objects;
 */
class DummyBaseObliviousTransfer final : public IBaseObliviousTransfer {
 public:
  explicit DummyBaseObliviousTransfer(
      std::unique_ptr<communication::IPartyCommunicationAgent> agent)
      : agent_{std::move(agent)} {}

  /**
   * @inherit doc
   */
  std::pair<std::vector<__m128i>, std::vector<__m128i>> send(
      size_t size) override;

  /**
   * @inherit doc
   */
  std::vector<__m128i> receive(const std::vector<bool>& choice) override;

  /**
   * @inherit doc
   */
  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    return agent_->getTrafficStatistics();
  }

  /**
   * @inherit doc
   */
  std::unique_ptr<communication::IPartyCommunicationAgent>
  extractCommunicationAgent() override {
    return std::move(agent_);
  }

 private:
  std::unique_ptr<communication::IPartyCommunicationAgent> agent_;
};

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::insecure
