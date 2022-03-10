/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <vector>
#include "fbpcf/mpc_framework/mpc_std_lib/util/util.h"

namespace fbpcf::mpc_std_lib::oram {

/**
 * a Difference Calculator computes the difference between a randomly generated
 * value and a desired one, multiplied by a sign indicatorShare
 * (positive/negative one).
 */
template <typename T>
class IDifferenceCalculator {
 public:
  virtual ~IDifferenceCalculator() = default;

  /**
   * Compute the difference between two batches of shared values.
   * The first value(s) are XOR-shared.
   * The second value(s) are additive-shared.
   * The batch of difference also need to multiply with a batch of sign
   * indicators (positive/negative one). The indicators are also
   * additively-shared.
   * For each instance in the batch, the formula is
   * result = (indicatorShare_0-indicatorShare_1)*((minuend_0 xor
   * minuend_1)-(subtrahend_0 - subtrahend_1))
   * @param indicatorShares a batch of additively shared indicatorShare, it is
   * guaranteed that indicatorShare_0-indicatorShare_1 \in {-1, 1};
   * @param minuendShares a vector of batches of XOR shared values (from less
   * significant to more significant), they can be converted to secret type T.
   * XOR shares are used here since it’s the easiest way to extract a shared
   * secret from a XOR-secret share engine. In other use cases we may need to
   * adopt other secret share schemes. The vector of vectors is in the following
   * format {[vector of 0-th bit in all shares], [vector of 1-th bit in all
   * shares], [vector of 2-th bit in all shares], ..., [vector of last bit in
   * all shares]}
   * @param subtrahendShares a vector of batches of additively shared values
   * @return (indicatorShares_0-indicatorShares_1)*((minuend_0 xor
   * minuend_1)-(subtrahend_0 - subtrahend_1))
   */

  virtual std::vector<T> calculateDifferenceBatch(
      const std::vector<uint32_t>& indicatorShares,
      const std::vector<std::vector<bool>>& minuendShares,
      const std::vector<T>& subtrahendShares) const = 0;

  virtual std::pair<uint64_t, uint64_t> getTrafficStatistics() const = 0;
};

} // namespace fbpcf::mpc_std_lib::oram
