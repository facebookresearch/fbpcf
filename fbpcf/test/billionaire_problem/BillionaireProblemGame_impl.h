/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/mpc_framework/frontend/mpcGame.h"
#include "fbpcf/test/billionaire_problem/BillionaireProblemGame.h"

namespace fbpcf::billionaire_problem {

template <int schedulerId, bool usingBatch>
typename BillionaireProblemGame<schedulerId, usingBatch>::CompareResult
BillionaireProblemGame<schedulerId, usingBatch>::billionaireProblem(
    const AssetsLists& aliceAssets,
    const AssetsLists& bobAssets) {
  int alicePartyId = 0;
  int bobPartyId = 1;

  SecAssetsLists secAliceAssets(aliceAssets, alicePartyId);
  SecAssetsLists secBobAssets(bobAssets, bobPartyId);

  auto secCompareResult =
      secAliceAssets.getTotalValue() < secBobAssets.getTotalValue();
  auto pubCompareResult = secCompareResult.openToParty(alicePartyId);
  return pubCompareResult.getValue();
}

template <int schedulerId, bool usingBatch>
BillionaireProblemGame<schedulerId, usingBatch>::SecAssetsLists::SecAssetsLists(
    const AssetsLists& assets,
    int partyId)
    : cash_(assets.cash, partyId),
      stock_(assets.stock, partyId),
      property_(assets.property, partyId) {}

template <int schedulerId, bool usingBatch>
typename BillionaireProblemGame<schedulerId, usingBatch>::SecUnsignedInt
BillionaireProblemGame<schedulerId, usingBatch>::SecAssetsLists::
    getTotalValue() {
  return cash_ + stock_ + property_;
}

} // namespace fbpcf::billionaire_problem
