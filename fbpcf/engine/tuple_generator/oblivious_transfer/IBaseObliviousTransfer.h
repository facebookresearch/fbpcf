/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emmintrin.h>
#include <memory>
#include "fbpcf/engine/communication/IPartyCommunicationAgent.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer {

/**
 * A base oblivious transfer interface
 */

class IBaseObliviousTransfer {
 public:
  virtual ~IBaseObliviousTransfer() = default;

  /**
   * run a number of base OT as the sender.
   * @param size number of base OT to run;
   * @return a pair of 0-message vector and 1-message vector
   */
  virtual std::pair<std::vector<__m128i>, std::vector<__m128i>> send(
      size_t size) = 0;

  /**
   * run a number of base OT as the receiver.
   * @param choice the choice bit vector, its length indicates number of base
   * OTs to run.
   * @return chosen-message vector
   */
  virtual std::vector<__m128i> receive(const std::vector<bool>& choice) = 0;

  /**
   * Get the total amount of traffic transmitted.
   * @return a pair of (sent, received) data in bytes.
   */
  virtual std::pair<uint64_t, uint64_t> getTrafficStatistics() const = 0;

  virtual std::unique_ptr<communication::IPartyCommunicationAgent>
  extractCommunicationAgent() = 0;
};

} // namespace fbpcf::engine::tuple_generator::oblivious_transfer
