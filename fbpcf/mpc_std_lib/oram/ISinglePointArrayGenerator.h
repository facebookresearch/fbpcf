/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emmintrin.h>
#include <cstdint>
#include <vector>

namespace fbpcf::mpc_std_lib::oram {

/**
 * a single point array generator allow two parties jointly generate a pair of
 * single point array. The point position is shared by two parties.
 */
class ISinglePointArrayGenerator {
 protected:
  using ArrayType =
      std::vector<std::pair<std::vector<bool>, std::vector<__m128i>>>;

 public:
  virtual ~ISinglePointArrayGenerator() = default;

  /**
   * Generate a batch of single point array, the point position is shared
   * by two parties. This operation will incur roundtrips, needs to run in
   * batch to amortize that overhead
   * @param indexShares this party's shares of the point positions (from less
   * significant to more significant), this is a
   * vector of batches of shared indexes
   * @param length the expected length of the single point arrays
   * @return a vector of batches of length-element single point position arrays
   */
  virtual ArrayType generateSinglePointArrays(
      const std::vector<std::vector<bool>>& indexShares,
      size_t length) = 0;

  virtual std::pair<uint64_t, uint64_t> getTrafficStatistics() const = 0;
};

} // namespace fbpcf::mpc_std_lib::oram
