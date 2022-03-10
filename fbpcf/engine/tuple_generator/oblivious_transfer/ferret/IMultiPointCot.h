/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <emmintrin.h>
#include <vector>
#include "fbpcf/engine/util/util.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::ferret {

/**
 * APIs for the ideal multi point cot. See details at
 * https://eprint.iacr.org/2020/924.pdf
 */
class IMultiPointCot {
 public:
  virtual ~IMultiPointCot() = default;

  /**
   * Initialization for the sender, lsb of delta must be 1.
   * Set the parameters to run with.
   * @param delta the global delta, lsb must be 1.
   * @param length number of OT instances to generate per iteration
   * @param weight the hamming weight for the choice vector
   */
  virtual void senderInit(__m128i delta, int64_t length, int64_t weight) = 0;

  /**
   * initialization for the receiver.
   * @param length number of OT instances to generate per iteration
   * @param weight the hamming weight for the choice vector
   */
  virtual void receiverInit(int64_t length, int64_t weight) = 0;

  /**
   * Return the base cot results needed per iteration.
   */
  virtual int getBaseCotNeeds() const = 0;

  /**
   * the sender's extend API. It will generate a vector of __m128i
   * @param baseCot : base cot results needed for this extension
   * @param return the 0-value of the OTs. All of which comes with lsb = 0
   */
  virtual std::vector<__m128i> senderExtend(std::vector<__m128i>&& baseCot) = 0;

  /**
   * the receiver's extend API. It will generate a vector of __m128i
   * @param baseCot : base cot results needed for this extension
   * @param return the b-value of the OTs where b = lsb of __m128i
   */
  virtual std::vector<__m128i> receiverExtend(
      std::vector<__m128i>&& baseCot) = 0;

  /**
   * Get the total amount of traffic transmitted.
   * @return a pair of (sent, received) data in bytes.
   */
  virtual std::pair<uint64_t, uint64_t> getTrafficStatistics() const = 0;
};

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::ferret
