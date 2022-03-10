/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emmintrin.h>
#include <vector>

namespace fbpcf::mpc_std_lib::oram {

/**
 * an oblivious delta calculator obliviously choose between two secret-shared
 * secrets and compute the lsbs of the shared secrets.
 */
class IObliviousDeltaCalculator {
 public:
  virtual ~IObliviousDeltaCalculator() = default;

  /**
   * This object operates in batches.
   * Compute mux(alpha, delta1, delta0) and lsb(delta0) xor alpha xor 1 and
   * lsb(delta1) xor alpha.
   * @param delta0Shares this party’s shares of delta0
   * @param delta1Shares this party’s shares of delta1
   * @param alphaShares this party’s shares of alpha
   * @return a batch of delta, a batch of delta_t0 and a batch of delta_t1
   */
  virtual std::tuple<std::vector<__m128i>, std::vector<bool>, std::vector<bool>>
  calculateDelta(
      const std::vector<__m128i>& delta0Shares,
      const std::vector<__m128i>& delta1Shares,
      const std::vector<bool>& alphaShares) const = 0;

  virtual std::pair<uint64_t, uint64_t> getTrafficStatistics() const = 0;
};

} // namespace fbpcf::mpc_std_lib::oram
