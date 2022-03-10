/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <vector>

namespace fbpcf::engine::tuple_generator::oblivious_transfer {

/**
 * The Bi-direction Oblivious transfer API.
 * Bi-direction OT means the two parties run two OT instances, take turns to
 * play the role of sender and receiver. This feature is extensively used in
 * SS-based MPC while single-direction OT is never used at all.
 */

template <class T>
class IBidirectionObliviousTransfer {
 public:
  virtual ~IBidirectionObliviousTransfer() = default;

  /**
   * run a number of bidirection OT.
   * @param input0 :  The input for each OT when playing as the sender, the
   * other party will receive this as output if their choice bit is 0;
   * @param input1 : The input for each OT when playing as the sender,  the
   * other party will receive this as output if their choice bit is 1;
   * @param choice: the choice bit for each OT when playing as the receiver
   * @return : the output from OT.
   */
  virtual std::vector<T> biDirectionOT(
      const std::vector<T>& input0,
      const std::vector<T>& input1,
      const std::vector<bool>& choice) = 0;

  /**
   * Get the total amount of traffic transmitted.
   * @return a pair of (sent, received) data in bytes.
   */
  virtual std::pair<uint64_t, uint64_t> getTrafficStatistics() const = 0;
};

} // namespace fbpcf::engine::tuple_generator::oblivious_transfer
