/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <emmintrin.h>
#include <memory>
#include <vector>
#include "fbpcf/engine/util/util.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer {

/**
 * The Random Correlated Oblivious Transfer API.
 * This object works as a wrap to hide all the details about the underlying RCOT
 * extender.
 */

class IRandomCorrelatedObliviousTransfer {
 public:
  virtual ~IRandomCorrelatedObliviousTransfer() = default;

  /**
   * Run a number of random correlated OT.
   * @param size: number of RCOT results to generate
   * @return : the output from RCOT.
   */
  virtual std::vector<__m128i> rcot(int64_t size) = 0;

  /**
   * Get the total amount of traffic transmitted.
   * @return a pair of (sent, received) data in bytes.
   */
  virtual std::pair<uint64_t, uint64_t> getTrafficStatistics() const = 0;
};

} // namespace fbpcf::engine::tuple_generator::oblivious_transfer
