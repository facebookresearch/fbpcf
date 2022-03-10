/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/mpc_framework/mpc_std_lib/util/util.h"

namespace fbpcf::mpc_std_lib::oram {

template <typename T, int8_t indicatorSumWidth, int schedulerId>
std::vector<T> DifferenceCalculator<T, indicatorSumWidth, schedulerId>::
    calculateDifferenceBatch(
        const std::vector<uint32_t>& indicatorShares,
        const std::vector<std::vector<bool>>& minuendShares,
        const std::vector<T>& subtrahendShares) const {
  auto indicator = recoverIndicators(indicatorShares);

  // one of the following call is to process this party's input, the other one
  // is to process peer's.
  // it doesn't matter which one corresponding to which party.
  // when processing this party's input, subtrahendShares provides meta-data
  // info as well as its content; when processing peer's input,
  // subtrahendShares only provides meta-data info, the content doesn't
  // matter.
  auto subtrahendShares1 =
      util::MpcAdapters<T, schedulerId>::processSecretInputs(
          subtrahendShares, party0Id_);
  auto subtrahendShares2 =
      util::MpcAdapters<T, schedulerId>::processSecretInputs(
          subtrahendShares, party1Id_);
  auto subtrahend = subtrahendShares1 - subtrahendShares2;

  auto minuend = util::MpcAdapters<T, schedulerId>::recoverBatchSharedSecrets(
      minuendShares);

  auto [finalMinuend, finalSubtrahend] =
      util::MpcAdapters<T, schedulerId>::obliviousSwap(
          minuend, subtrahend, indicator);
  auto v = finalMinuend - finalSubtrahend;
  auto rst0 = util::MpcAdapters<T, schedulerId>::openToParty(v, party0Id_);
  auto rst1 = util::MpcAdapters<T, schedulerId>::openToParty(v, party1Id_);
  if (amIParty0_) {
    return rst0;
  } else {
    return rst1;
  }
}

template <typename T, int8_t indicatorSumWidth, int schedulerId>
frontend::Bit<true, schedulerId, true>
DifferenceCalculator<T, indicatorSumWidth, schedulerId>::recoverIndicators(
    const std::vector<uint32_t>& indicatorShares) const {
  // one of the following call is to process this party's input, the other
  // one
  // is to process peer's.
  // it doesn't matter which one corresponding to which party.
  // when processing this party's input, indicatorShares provides meta-data
  // info as well as its content; when processing peer's input,
  // indicatorShares only provides meta-data info, the content doesn't
  // matter.
  auto indicator0 =
      frontend::Int<false, indicatorSumWidth, true, schedulerId, true>(
          indicatorShares, party0Id_);
  auto indicator1 =
      frontend::Int<false, indicatorSumWidth, true, schedulerId, true>(
          indicatorShares, party1Id_);
  return (indicator0 - indicator1)[indicatorSumWidth - 1];
}

} // namespace fbpcf::mpc_std_lib::oram
