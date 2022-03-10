/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <emmintrin.h>
#include <cstdint>
#include <memory>
#include <vector>
#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_framework/engine/util/util.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::ferret {

/**
 * This is the API of a Rcot extender. It can extend k Rcot results to n Rcot
 * results.
 */
class IRcotExtender {
 public:
  virtual ~IRcotExtender() = default;

  /**
   * Sender's API to init the extender.
   * @param delta : the global difference
   * @param extendedSize : number of Rcots expected in extended output.
   * @param baseSize : number of Rcot results used as the base for extension.
   * @param weight: the weight parameter, used for the extension algorithm.
   * @return Rcot needed for each iteration.
   */
  virtual int senderInit(
      __m128i delta,
      int64_t extendedSize,
      int64_t baseSize,
      int64_t weight) = 0;

  /**
   * Receiver's API to init the extender.
   * @param extendedSize : number of Rcots expected in extended output.
   * @param baseSize : number of Rcot results used as the base for extension.
   * @param weight: the weight parameter, used for the extension algorithm.
   * @return extra base Rcot needed for the first iteration.
   */
  virtual int
  receiverInit(int64_t extendedSize, int64_t baseSize, int64_t weight) = 0;

  /**
   * Set the communication agent. Note that an extender usually needs a one-time
   * bootstrapper. Thus we let the bootstrapper to use the agent first.
   */
  virtual void setCommunicationAgent(
      std::unique_ptr<communication::IPartyCommunicationAgent> agent) = 0;

  /**
   * Indicates how many base COT are needed for extension.
   */
  virtual int getBaseCotSize() const = 0;

  /**
   * Sender's API to extend the base Rcot into dstLength Rcots.
   * @param baseRcot : the starting point of the Rcot extension
   * @return : the extended Rcots
   */
  virtual std::vector<__m128i> senderExtendRcot(
      std::vector<__m128i>&& baseRcot) = 0;

  /**
   * Receiver's API to extend the base Rcot into dstLength Rcots.
   * @param baseRcot : the starting point of the Rcot extension
   * @return : the extended Rcots
   */
  virtual std::vector<__m128i> receiverExtendRcot(
      std::vector<__m128i>&& baseRcot) = 0;

  /**
   * Get the total amount of traffic transmitted.
   * @return a pair of (sent, received) data in bytes.
   */
  virtual std::pair<uint64_t, uint64_t> getTrafficStatistics() const = 0;
};

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::ferret
