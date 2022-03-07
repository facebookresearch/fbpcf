/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <algorithm>
#include <vector>
#include "fbpcf/mpc_framework/mpc_std_lib/oram/IDifferenceCalculator.h"
#include "fbpcf/mpc_framework/mpc_std_lib/util/util.h"

namespace fbpcf::mpc_framework::mpc_std_lib::oram {

/**
 * a difference Calculator compute the difference between a randomly
 * generated value and a desired one, multiplied by a sign indicatorShare
 * (positive/negative one).
 */
template <typename T, int8_t indicatorSumWidth, int schedulerId>
class DifferenceCalculator final : public IDifferenceCalculator<T> {
 public:
  using SecBatchType = typename util::MpcAdapters<T, schedulerId>::SecBatchType;

  DifferenceCalculator(bool amIParty0, int32_t party0Id, int32_t party1Id)
      : amIParty0_(amIParty0), party0Id_(party0Id), party1Id_(party1Id) {}

  /**
   * @inherit doc
   */
  std::vector<T> calculateDifferenceBatch(
      const std::vector<uint32_t>& indicatorShares,
      const std::vector<std::vector<bool>>& minuendShares,
      const std::vector<T>& subtrahendShares) const override;

  std::pair<uint64_t, uint64_t> getTrafficStatistics() const {
    // this object itself doesn't generate any traffic. All the traffic are
    // outsourced into the mpc game.
    return {0, 0};
  }

 private:
  frontend::Bit<true, schedulerId, true> recoverIndicators(
      const std::vector<uint32_t>& indicatorShares) const;

  bool amIParty0_;
  int32_t party0Id_;
  int32_t party1Id_;
};

} // namespace fbpcf::mpc_framework::mpc_std_lib::oram

#include "fbpcf/mpc_framework/mpc_std_lib/oram/DifferenceCalculator_impl.h"
