/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <math.h>
#include <cstdint>
#include <stdexcept>
#include <string>
#include "fbpcf/mpc_std_lib/permuter/IPermuter.h"

#include "fbpcf/mpc_std_lib/util/util.h"
#include "folly/logging/xlog.h"

namespace fbpcf::mpc_std_lib::permuter {
template <typename T, int schedulerId>
class AsWaksmanPermuter;

/**
 * The partial specification for the AsWaksmanPermuter of BitString.
 **/
template <int schedulerId>
class AsWaksmanPermuter<std::vector<bool>, schedulerId> final
    : public IPermuter<frontend::BitString<true, schedulerId, true>> {
 public:
  AsWaksmanPermuter(int myId, int partnerId)
      : myId_(myId), partnerId_(partnerId) {}

  frontend::BitString<true, schedulerId, true> permute(
      const frontend::BitString<true, schedulerId, true>& src,
      size_t size) const override;

  frontend::BitString<true, schedulerId, true> permute(
      const frontend::BitString<true, schedulerId, true>& src,
      size_t size,
      const std::vector<uint32_t>& order) const override;

 private:
  std::vector<std::vector<bool>> computingBatchAndWithMpc(
      const std::vector<bool>& left,
      const std::vector<std::vector<bool>>& right) const;
  void permute(
      std::vector<std::vector<bool>>& src,
      std::vector<size_t> batches,
      size_t totalSize,
      std::vector<std::vector<bool>>& choices) const;

  size_t swapSchedule(
      const std::vector<std::vector<bool>>& src,
      size_t startIndex /*inclusive*/,
      size_t endIndex /*exclusive*/,
      std::vector<std::vector<bool>>& dst,
      size_t dstIndex,
      bool skipLastOne) const;

  size_t swapCompute(
      std::vector<std::vector<bool>>& src,
      size_t startIndex /*inclusive*/,
      size_t endIndex /*exclusive*/,
      const std::vector<std::vector<bool>>& andResult,
      size_t andResultIndex,
      bool skipLastOne) const;

  void computeChoiceVectors(
      std::vector<std::vector<uint32_t>>&& orders,
      std::vector<std::vector<bool>>& rst,
      int totalNumber) const;

  void swapInALayer(
      std::vector<std::vector<bool>>& src,
      const std::vector<std::uint64_t>& batches,
      std::vector<std::vector<bool>>& givenChoices,
      bool skipLast) const;

  int myId_;
  int partnerId_;
};

template <int schedulerId>
frontend::BitString<true, schedulerId, true>
AsWaksmanPermuter<std::vector<bool>, schedulerId>::permute(
    const frontend::BitString<true, schedulerId, true>& src,
    size_t size) const {
  std::vector<std::vector<bool>> vectorOfVectors =
      src.extractStringShare().getValue();
  if (vectorOfVectors.size() == 0) {
    throw std::runtime_error("empty input!");
  }
  auto length = vectorOfVectors.at(0).size();
  std::vector<std::vector<bool>> dummyChoice;

  permute(vectorOfVectors, std::vector<size_t>(1, length), length, dummyChoice);
  return frontend::BitString<true, schedulerId, true>(
      typename frontend::BitString<true, schedulerId, true>::ExtractedString(
          vectorOfVectors));
}

template <int schedulerId>
frontend::BitString<true, schedulerId, true>
AsWaksmanPermuter<std::vector<bool>, schedulerId>::permute(
    const frontend::BitString<true, schedulerId, true>& src,
    size_t size,
    const std::vector<uint32_t>& order) const {
  auto vectorOfVectors = src.extractStringShare().getValue();
  if (vectorOfVectors.size() == 0) {
    throw std::runtime_error("empty input!");
  }
  auto length = vectorOfVectors.at(0).size();
  std::vector<std::vector<bool>> rst;
  rst.reserve(2 * (std::ceil(log2(order.size()))) - 1);
  computeChoiceVectors(
      std::vector<std::vector<uint32_t>>(1, order), rst, order.size());

  permute(vectorOfVectors, std::vector<size_t>(1, length), length, rst);
  return frontend::BitString<true, schedulerId, true>(
      typename frontend::BitString<true, schedulerId, true>::ExtractedString(
          vectorOfVectors));
}

template <int schedulerId>
std::vector<std::vector<bool>>
AsWaksmanPermuter<std::vector<bool>, schedulerId>::computingBatchAndWithMpc(
    const std::vector<bool>& left,
    const std::vector<std::vector<bool>>& right) const {
  frontend::Bit<true, schedulerId, true> leftBit(
      (typename frontend::Bit<true, schedulerId, true>::ExtractedBit(left)));
  std::vector<frontend::Bit<true, schedulerId, true>> rightBit;
  rightBit.reserve(right.size());
  for (size_t i = 0; i < right.size(); i++) {
    rightBit.push_back(frontend::Bit<true, schedulerId, true>(
        typename frontend::Bit<true, schedulerId, true>::ExtractedBit(
            right.at(i))));
  }
  auto result = leftBit & rightBit;
  std::vector<std::vector<bool>> rst;
  rst.reserve(right.size());
  for (size_t i = 0; i < right.size(); i++) {
    rst.push_back(result.at(i).extractBit().getValue());
  }
  return rst;
}

template <int schedulerId>
void AsWaksmanPermuter<std::vector<bool>, schedulerId>::permute(
    std::vector<std::vector<bool>>& src,
    std::vector<size_t> batches,
    size_t totalSize,
    std::vector<std::vector<bool>>& givenChoices) const {
  if (batches.size() * 2 >= totalSize) {
    // doing some 2-element swap.
    swapInALayer(src, batches, givenChoices, false);
    return;
  } else {
    std::vector<size_t> newBatches;
    newBatches.reserve(batches.size() * 2);
    for (size_t i = 0; i < batches.size(); i++) {
      newBatches.push_back(batches.at(i) >> 1);
      newBatches.push_back(batches.at(i) - newBatches.at(2 * i));
    }

    // do some pre-swap
    swapInALayer(src, batches, givenChoices, false);
    permute(src, newBatches, totalSize, givenChoices);
    // do some post-swap
    swapInALayer(src, batches, givenChoices, true);
  }
}

template <int schedulerId>
void AsWaksmanPermuter<std::vector<bool>, schedulerId>::swapInALayer(
    std::vector<std::vector<bool>>& src,
    const std::vector<std::uint64_t>& batches,
    std::vector<std::vector<bool>>& givenChoices,
    bool skipLast) const {
  // run the swap
  size_t index = 0;
  size_t dstIndex = 0;
  size_t totalNumberOfSwap = 0;
  for (size_t i = 0; i < batches.size(); i++) {
    totalNumberOfSwap += (batches.at(i) - skipLast) >> 1;
  }
  std::vector<std::vector<bool>> compositeAndInputs(
      src.size(), std::vector<bool>(totalNumberOfSwap));
  for (size_t i = 0; i < batches.size(); i++) {
    dstIndex = swapSchedule(
        src,
        index,
        index + batches.at(i),
        compositeAndInputs,
        dstIndex,
        skipLast);
    index += batches.at(i);
  }
  // do ANDs
  std::vector<bool> choice;
  if (givenChoices.size() != 0) {
    choice = givenChoices.back();
    givenChoices.pop_back();
  } else {
    choice = std::vector<bool>(totalNumberOfSwap, 0);
  }
  std::vector<std::vector<bool>> compositeAndResults =
      computingBatchAndWithMpc(choice, compositeAndInputs);

  index = 0;
  size_t andResultIndex = 0;
  for (size_t i = 0; i < batches.size(); i++) {
    andResultIndex = swapCompute(
        src,
        index,
        index + batches.at(i),
        compositeAndResults,
        andResultIndex,
        skipLast);
    index += batches.at(i);
  }
}

template <int schedulerId>
size_t AsWaksmanPermuter<std::vector<bool>, schedulerId>::swapSchedule(
    const std::vector<std::vector<bool>>& src,
    size_t startIndex /*inclusive*/,
    size_t endIndex /*exclusive*/,
    std::vector<std::vector<bool>>& dst,
    size_t dstIndex,
    bool skipLastOne) const {
  size_t count = endIndex - startIndex;
  size_t offSet = count >> 1;
  size_t numberOfSwap = (count - skipLastOne) >> 1;
  for (size_t t = 0; t < src.size(); t++) {
    for (size_t i = 0; i < numberOfSwap; i++) {
      auto x = src.at(t).at(startIndex + i);
      auto y = src.at(t).at(startIndex + i + offSet);
      dst.at(t).at(dstIndex + i) = x ^ y;
    }
  }
  dstIndex += numberOfSwap;
  return dstIndex;
}

template <int schedulerId>
size_t AsWaksmanPermuter<std::vector<bool>, schedulerId>::swapCompute(
    std::vector<std::vector<bool>>& src,
    size_t startIndex /*inclusive*/,
    size_t endIndex /*exclusive*/,
    const std::vector<std::vector<bool>>& andResult,
    size_t andResultIndex,
    bool skipLastOne) const {
  size_t count = endIndex - startIndex;
  size_t offSet = count >> 1;
  size_t numberOfSwap = (count - skipLastOne) >> 1;

  for (size_t t = 0; t < src.size(); t++) {
    for (size_t i = 0; i < numberOfSwap; i++) {
      src.at(t).at(startIndex + i) =
          src.at(t).at(startIndex + i) ^ andResult.at(t).at(andResultIndex + i);
      src.at(t).at(startIndex + i + offSet) =
          src.at(t).at(startIndex + i + offSet) ^
          andResult.at(t).at(andResultIndex + i);
    }
  }
  andResultIndex += numberOfSwap;
  return andResultIndex;
}

template <int schedulerId>
void AsWaksmanPermuter<std::vector<bool>, schedulerId>::computeChoiceVectors(
    std::vector<std::vector<uint32_t>>&& orders,
    std::vector<std::vector<bool>>& rst,
    int totalNumber) const {
  if (orders.size() * 2 >= totalNumber) {
    std::vector<bool> choices;
    choices.reserve(totalNumber - orders.size());
    for (auto& item : orders) {
      if (item.size() == 2) {
        choices.push_back(item.at(0) == 1);
      }
    }
    rst.push_back(choices);
  } else {
    std::vector<bool> firstSwapChoices;
    std::vector<bool> secondSwapChoices;
    std::vector<std::vector<uint32_t>> suborders;
    suborders.reserve(orders.size() * 2);
    for (auto& item : orders) {
      AsWaksmanParameterCalculator calculator(item);
      auto choice = calculator.getFirstSwapConditions();
      firstSwapChoices.insert(
          firstSwapChoices.end(), choice.begin(), choice.end());

      choice = calculator.getSecondSwapConditions();
      secondSwapChoices.insert(
          secondSwapChoices.end(), choice.begin(), choice.end());
      suborders.push_back(calculator.getFirstSubPermuteOrder());
      suborders.push_back(calculator.getSecondSubPermuteOrder());
    }
    // store the result in a reversed order, works like a stack
    rst.push_back(std::move(secondSwapChoices));
    computeChoiceVectors(std::move(suborders), rst, totalNumber);
    rst.push_back(std::move(firstSwapChoices));
  }
}

} // namespace fbpcf::mpc_std_lib::permuter
