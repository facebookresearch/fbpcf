/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstddef>
#include <memory>
#include "fbpcf/mpc_framework/mpc_std_lib/oram/IObliviousDeltaCalculator.h"
#include "fbpcf/mpc_framework/mpc_std_lib/util/util.h"

namespace fbpcf::mpc_std_lib::oram {

/**
 * A secure implementation of oblivious delta calculator
 */
template <int schedulerId>
class ObliviousDeltaCalculator final : public IObliviousDeltaCalculator {
  static const size_t kBitsInM128i = 128 /* there are 128 bits in __m128i*/;

 public:
  explicit ObliviousDeltaCalculator(
      bool amIParty0,
      int32_t party0Id,
      int32_t party1Id)
      : amIParty0_(amIParty0), party0Id_(party0Id), party1Id_(party1Id) {}

  /**
   * @inherit doc
   */
  std::tuple<std::vector<__m128i>, std::vector<bool>, std::vector<bool>>
  calculateDelta(
      const std::vector<__m128i>& delta0Shares,
      const std::vector<__m128i>& delta1Shares,
      const std::vector<bool>& alphaShares) const override;

  virtual std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    // this object itself doesn't generate any traffic. All the traffic are
    // outsourced into the mpc game.
    return {0, 0};
  }

 private:
  bool amIParty0_;
  int32_t party0Id_;
  int32_t party1Id_;
};

} // namespace fbpcf::mpc_std_lib::oram

#include "fbpcf/mpc_framework/mpc_std_lib/oram/ObliviousDeltaCalculator_impl.h"
