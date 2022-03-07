/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/IBidirectionObliviousTransfer.h"

namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer::
    insecure {

/**
 * This is a Dummy oblivious transfer object. A Dummy object will realize the
 * desired functionality in an insecure way. Usually they are added as:
  1. baseline for correctness;
  2. a place holder for integration test (if we only want to test some of the
 objects)
  3. a helper when testing other objects;
 */
template <class T>
class DummyBidirectionObliviousTransfer final
    : public IBidirectionObliviousTransfer<T> {
 public:
  explicit DummyBidirectionObliviousTransfer(
      std::unique_ptr<communication::IPartyCommunicationAgent> agent)
      : agent_{std::move(agent)} {}

  /**
   * @inherit doc
   */
  std::vector<T> biDirectionOT(
      const std::vector<T>& input0,
      const std::vector<T>& input1,
      const std::vector<bool>& choice) override;

  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    return agent_->getTrafficStatistics();
  }

 private:
  std::unique_ptr<communication::IPartyCommunicationAgent> agent_;
};

} // namespace
  // fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer::insecure

#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/DummyBidirectionObliviousTransfer_impl.h"
