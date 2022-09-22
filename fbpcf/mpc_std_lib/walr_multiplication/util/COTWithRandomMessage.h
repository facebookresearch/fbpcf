/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emmintrin.h>
#include <fbpcf/engine/tuple_generator/oblivious_transfer/IRandomCorrelatedObliviousTransfer.h>
#include "fbpcf/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/engine/util/util.h"

namespace fbpcf::mpc_std_lib::walr::util {

class COTWithRandomMessage {
 public:
  // Sender constructor
  COTWithRandomMessage(
      __m128i delta,
      std::unique_ptr<engine::communication::IPartyCommunicationAgent> agent,
      std::unique_ptr<engine::tuple_generator::oblivious_transfer::
                          IRandomCorrelatedObliviousTransfer> rcot)
      : delta_(delta),
        role_(engine::util::Role::sender),
        agent_(std::move(agent)),
        rcot_(std::move(rcot)) {}

  // Receiver constructor
  COTWithRandomMessage(
      std::unique_ptr<engine::communication::IPartyCommunicationAgent> agent,
      std::unique_ptr<engine::tuple_generator::oblivious_transfer::
                          IRandomCorrelatedObliviousTransfer> rcot)
      : role_(engine::util::Role::receiver),
        agent_(std::move(agent)),
        rcot_(std::move(rcot)) {}

  /**
   * run a number of COTwRM as the sender.
   * @param size number of COTwR to run;
   * @return a pair of 0-message vector and 1-message vector
   */
  std::pair<std::vector<__m128i>, std::vector<__m128i>> send(size_t size);

  /**
   * run a number of COTwRM as the receiver.
   * @param choice the choice bit vector, its length indicates number of
   * COTwRMs to run.
   * @return chosen-message vector
   */
  std::vector<__m128i> receive(const std::vector<bool>& choice);

  std::unique_ptr<engine::communication::IPartyCommunicationAgent>
  extractCommunicationAgent() {
    return std::move(agent_);
  }

  /**
   * Get the total amount of traffic transmitted.
   * @return a pair of (sent, received) data in bytes.
   */
  std::pair<uint64_t, uint64_t> getTrafficStatistics() const {
    auto rcotTraffic = rcot_->getTrafficStatistics();
    auto nonRcotTraffic = agent_->getTrafficStatistics();
    return {
        rcotTraffic.first + nonRcotTraffic.first,
        rcotTraffic.second + nonRcotTraffic.second};
  }

 private:
  __m128i delta_;
  engine::util::Role role_;
  std::unique_ptr<engine::communication::IPartyCommunicationAgent> agent_;
  std::unique_ptr<engine::tuple_generator::oblivious_transfer::
                      IRandomCorrelatedObliviousTransfer>
      rcot_;
};

} // namespace fbpcf::mpc_std_lib::walr::util
