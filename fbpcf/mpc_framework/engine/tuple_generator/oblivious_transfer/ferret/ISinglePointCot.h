/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <emmintrin.h>
#include <smmintrin.h>
#include <vector>
#include "fbpcf/mpc_framework/engine/util/util.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::ferret {

/**
 * APIs for the ideal single point cot. This is a randomized one, meaning the
 * position will be randomly selected, instead of provided by the receiver. See
 * details at https://eprint.iacr.org/2020/924.pdf
 */
class ISinglePointCot {
 public:
  virtual ~ISinglePointCot() = default;

  /**
   * initialization for the sender, lsb of delta must be 1.
   * @param delta the global delta, lsb must be 1.
   */
  virtual void senderInit(__m128i delta) = 0;

  /**
   * initialization for the receiver.
   */
  virtual void receiverInit() = 0;

  /**
   * the sender's extend API. It will generate a vector of __m128i
   * @param baseCot : base cot results needed for this extension
   * @return the 0-value of the OTs. All of which comes with lsb = 0.
   * number of OT is determined by input size, e.g. the size of output is 2
   * raise to size of input.
   */
  virtual std::vector<__m128i> senderExtend(std::vector<__m128i>&& baseCot) = 0;

  /**
   * the receiver's extend API. It will generate a vector of __m128i
   * @param baseCot : base cot results needed for this extension
   * @return the b-value of the OTs where b = lsb of __m128i
   * number of OT is determined by input size, e.g. the size of output is 2
   * raise to size of input. The choice position is determined by the 1s in the
   * baseCot. E.g. if the choice bit in the baseCot is 1,0,0,1,0 then the
   * choice position in the extension result is (01101)_2 = 13.
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
