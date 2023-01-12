/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/mpc_std_lib/permuter/IPermuter.h"

#include "fbpcf/mpc_std_lib/util/util.h"

namespace fbpcf::mpc_std_lib::permuter {

/**
 * This permuter uses the AS-Waksman network to run oblivious permutation. Read
 * more about this network:  Bruno Beauquier, Eric Darrot. On Arbitrary Waksman
 * Networks and their Vulnerability. RR-3788, INRIA. 1999. inria-00072871f
 **/
template <typename T, int schedulerId>
class AsWaksmanPermuter final
    : public IPermuter<typename util::SecBatchType<T, schedulerId>::type> {
 public:
  using SecBatchType = typename util::SecBatchType<T, schedulerId>::type;
  AsWaksmanPermuter(int myId, int partnerId)
      : myId_(myId), partnerId_(partnerId) {}

  SecBatchType permute_impl(const SecBatchType& src, size_t size)
      const override;

  SecBatchType permute_impl(
      const SecBatchType& src,
      size_t size,
      const std::vector<uint32_t>& order) const override;

 private:
  std::pair<SecBatchType, SecBatchType> preSubPermutationSwap(
      const SecBatchType& src,
      frontend::Bit<true, schedulerId, true>&& firstSwapConditions,
      size_t size) const;

  SecBatchType postSubPermutationSwap(
      SecBatchType&& src0,
      SecBatchType&& src1,
      frontend::Bit<true, schedulerId, true>&& secondSwapConditions,
      size_t size) const;

  SecBatchType processTwoElements(
      const SecBatchType& src,
      frontend::Bit<true, schedulerId, true>&& swapConditions) const;

  int myId_;
  int partnerId_;
};

/**
 * This object is to calculate the parameters to use AsWaksman network to
 * permute a set of values. The construction of the network can be found in
 * paper: Bruno Beauquier, Eric Darrot. On Arbitrary Waksman Networks and their
 * Vulnerability. RR-3788, INRIA. 1999. inria-00072871f
 */

class AsWaksmanParameterCalculator {
 public:
  explicit AsWaksmanParameterCalculator(const std::vector<uint32_t>& order)
      : size_(order.size()),
        firstHalfSize_(size_ / 2),
        secondHalfSize_(size_ - firstHalfSize_),
        expectedOrder_(order),
        inversedOrder_(size_),
        firstSwapConditions_(firstHalfSize_),
        secondSwapConditions_(secondHalfSize_ - 1),
        firstSubPermuteOrder_(firstHalfSize_, static_cast<uint32_t>(-1)),
        secondSubPermuteOrder_(secondHalfSize_, static_cast<uint32_t>(-1)) {
    for (size_t i = 0; i < size_; i++) {
      inversedOrder_[expectedOrder_.at(i)] = i;
    }
    compute();
  }

  std::vector<bool> getFirstSwapConditions() {
    return std::move(firstSwapConditions_);
  }

  std::vector<bool> getSecondSwapConditions() {
    return std::move(secondSwapConditions_);
  }

  std::vector<uint32_t> getFirstSubPermuteOrder() {
    return std::move(firstSubPermuteOrder_);
  }

  std::vector<uint32_t> getSecondSubPermuteOrder() {
    return std::move(secondSubPermuteOrder_);
  }

 private:
  void compute();
  uint32_t fillIntoSecondHalf(uint32_t indexAfterPermute);

  // find the index of the dual element
  inline uint32_t findDualIndex(uint32_t index) const {
    // the last element has no dual element when size is odd
    if ((index == size_ - 1) && ((size_ % 2) == 1)) {
      return static_cast<uint32_t>(-1);
    }
    if (index >= firstHalfSize_) {
      return index - firstHalfSize_;
    } else {
      return index + firstHalfSize_;
    }
  }

  // find the corresponding index in the sub permute
  inline uint32_t findIndexInSubPermute(uint32_t index) const {
    if (index >= firstHalfSize_) {
      return index - firstHalfSize_;
    } else {
      return index;
    }
  }

  inline uint32_t findAFreeIndex() const {
    size_t index = secondHalfSize_ - 1;
    while ((index < size_) && (secondSubPermuteOrder_.at(index) < size_)) {
      index--;
    }
    if (index < size_) {
      return index + firstHalfSize_;
    } else {
      return index;
    }
  }

  const size_t size_;
  const size_t firstHalfSize_;
  const size_t secondHalfSize_;
  const std::vector<uint32_t>& expectedOrder_;

  std::vector<uint32_t> inversedOrder_;
  std::vector<bool> firstSwapConditions_;
  std::vector<bool> secondSwapConditions_;

  std::vector<uint32_t> firstSubPermuteOrder_;
  std::vector<uint32_t> secondSubPermuteOrder_;
};

} // namespace fbpcf::mpc_std_lib::permuter

#include "fbpcf/mpc_std_lib/permuter/AsWaksmanPermuter_impl.h"

#include "fbpcf/mpc_std_lib/permuter/AsWaksmanPermuterForBitString_impl.h"
