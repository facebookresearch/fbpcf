/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/mpc_std_lib/permuter/AsWaksmanPermuter.h"

namespace fbpcf::mpc_std_lib::permuter {

void AsWaksmanParameterCalculator::compute() {
  uint32_t nextIndexInSecondHalf = size_ - 1;
  do {
    nextIndexInSecondHalf = fillIntoSecondHalf(nextIndexInSecondHalf);

    if ((nextIndexInSecondHalf >= size_ /* check if it's an invalid value*/) ||
        (findIndexInSubPermute(nextIndexInSecondHalf) >= secondHalfSize_) ||
        (secondSubPermuteOrder_.at(
             findIndexInSubPermute(nextIndexInSecondHalf)) < secondHalfSize_)) {
      nextIndexInSecondHalf = findAFreeIndex();

      if (nextIndexInSecondHalf >= size_ /* check if it's an invalid value*/) {
        return;
      }
      auto subIndex = findIndexInSubPermute(nextIndexInSecondHalf);
      secondSwapConditions_[subIndex] = false;
    }

  } while (1);
}

uint32_t AsWaksmanParameterCalculator::fillIntoSecondHalf(
    uint32_t indexAfterPermute) {
  auto indexBeforePermute = expectedOrder_.at(indexAfterPermute);

  auto subIndexBeforePermute = findIndexInSubPermute(indexBeforePermute);
  auto subIndexAfterPermute = findIndexInSubPermute(indexAfterPermute);
  secondSubPermuteOrder_[subIndexAfterPermute] = subIndexBeforePermute;

  auto dualIndexBeforePermute = findDualIndex(indexBeforePermute);
  if (dualIndexBeforePermute >= size_ /* check if it's an invalid value*/) {
    return static_cast<uint32_t>(-1);
  }
  firstSwapConditions_[subIndexBeforePermute] =
      indexBeforePermute < firstHalfSize_;
  auto dualIndexAfterPermute = inversedOrder_.at(dualIndexBeforePermute);

  auto subIndexForDualIndexAfterPermute =
      findIndexInSubPermute(dualIndexAfterPermute);
  firstSubPermuteOrder_[subIndexForDualIndexAfterPermute] =
      subIndexBeforePermute;

  if (subIndexForDualIndexAfterPermute < secondHalfSize_ - 1) {
    secondSwapConditions_[subIndexForDualIndexAfterPermute] =
        dualIndexAfterPermute >= firstHalfSize_;
  }
  return findDualIndex(dualIndexAfterPermute);
}

} // namespace fbpcf::mpc_std_lib::permuter
