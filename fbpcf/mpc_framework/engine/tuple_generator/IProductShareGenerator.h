/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <cstdint>
#include <vector>

namespace fbpcf::engine::tuple_generator {

/**
 * This object generates product shares, namely, a party holds bits a1, b1 and
 * the other party holds bits a2, b2. This object will generate the shares of
 * a1&b2 ^ a2&b1 for the two parties
 */

class IProductShareGenerator {
 public:
  virtual ~IProductShareGenerator() = default;

  /**
   * @param left the array of one factor
   * @param right the array of another factor
   * @return the share of the products
   */
  virtual std::vector<bool> generateBooleanProductShares(
      const std::vector<bool>& left,
      const std::vector<bool>& right) = 0;

  /**
   * Get the total amount of traffic transmitted.
   * @return a pair of (sent, received) data in bytes.
   */
  virtual std::pair<uint64_t, uint64_t> getTrafficStatistics() const = 0;
};

} // namespace fbpcf::engine::tuple_generator
