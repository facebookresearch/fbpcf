/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fbpcf/mpc_std_lib/util/util.h>
namespace fbpcf::mpc_std_lib::permuter {

template <typename T, int schedulerId>
typename AsWaksmanPermuter<T, schedulerId>::SecBatchType
AsWaksmanPermuter<T, schedulerId>::permute(const SecBatchType& src, size_t size)
    const {
  if (size == 1) {
    return src;
  }
  if (size == 2) {
    std::vector<bool> placeHolder(1);
    return processTwoElements(
        src, frontend::Bit<true, schedulerId, true>(placeHolder, partnerId_));
  }

  std::vector<bool> placeHolder(size / 2);
  frontend::Bit<true, schedulerId, true> firstSwapConditions(
      placeHolder, partnerId_);

  auto [first, second] =
      preSubPermutationSwap(src, std::move(firstSwapConditions), size);
  auto permutedFirst = permute(std::move(first), size / 2);
  auto permutedSecond = permute(std::move(second), size - size / 2);

  placeHolder.resize((size - 1) / 2);
  frontend::Bit<true, schedulerId, true> secondSwapConditions(
      placeHolder, partnerId_);

  return postSubPermutationSwap(
      std::move(permutedFirst),
      std::move(permutedSecond),
      std::move(secondSwapConditions),
      size);
}

template <typename T, int schedulerId>
typename AsWaksmanPermuter<T, schedulerId>::SecBatchType
AsWaksmanPermuter<T, schedulerId>::permute(
    const SecBatchType& src,
    size_t size,
    const std::vector<uint32_t>& order) const {
  if (size == 1) {
    return src;
  }
  if (size == 2) {
    std::vector<bool> swapChoice(1, order.at(0) == 1);
    return processTwoElements(
        src, frontend::Bit<true, schedulerId, true>(swapChoice, myId_));
  }
  AsWaksmanParameterCalculator calculator(order);

  frontend::Bit<true, schedulerId, true> firstSwapConditions(
      calculator.getFirstSwapConditions(), myId_);

  auto [first, second] =
      preSubPermutationSwap(src, std::move(firstSwapConditions), size);

  auto permutedFirst =
      permute(std::move(first), size / 2, calculator.getFirstSubPermuteOrder());

  auto permutedSecond = permute(
      std::move(second),
      size - size / 2,
      calculator.getSecondSubPermuteOrder());

  frontend::Bit<true, schedulerId, true> secondSwapConditions(
      calculator.getSecondSwapConditions(), myId_);

  return postSubPermutationSwap(
      std::move(permutedFirst),
      std::move(permutedSecond),
      std::move(secondSwapConditions),
      size);
}

template <typename T, int schedulerId>
std::pair<
    typename AsWaksmanPermuter<T, schedulerId>::SecBatchType,
    typename AsWaksmanPermuter<T, schedulerId>::SecBatchType>
AsWaksmanPermuter<T, schedulerId>::preSubPermutationSwap(
    const SecBatchType& src,
    frontend::Bit<true, schedulerId, true>&& firstSwapConditions,
    size_t size) const {
  // unbatch to 3 pieces if size is odd, 2 pieces if even
  auto unbatchSize = std::make_shared<std::vector<uint32_t>>(2 + (size & 1));
  (*unbatchSize)[0] = size / 2;
  (*unbatchSize)[1] = size / 2;
  if ((size & 1) == 1) {
    (*unbatchSize)[2] = 1;
  }
  auto batches =
      util::MpcAdapters<T, schedulerId>::unbatching(src, unbatchSize);
  if ((size & 1) == 0) {
    return util::MpcAdapters<T, schedulerId>::obliviousSwap(
        std::move(batches[0]),
        std::move(batches[1]),
        std::move(firstSwapConditions));
  } else {
    auto [first, second] = util::MpcAdapters<T, schedulerId>::obliviousSwap(
        std::move(batches[0]),
        std::move(batches[1]),
        std::move(firstSwapConditions));
    auto fullSecond = util::MpcAdapters<T, schedulerId>::batchingWith(
        std::move(second), {std::move(batches[2])});
    return {first, fullSecond};
  }
}

template <typename T, int schedulerId>
typename AsWaksmanPermuter<T, schedulerId>::SecBatchType
AsWaksmanPermuter<T, schedulerId>::postSubPermutationSwap(
    SecBatchType&& src0,
    SecBatchType&& src1,
    frontend::Bit<true, schedulerId, true>&& secondSwapConditions,
    size_t size) const {
  auto unbatchSize = std::make_shared<std::vector<uint32_t>>(2);
  (*unbatchSize)[0] = (size - 1) / 2;
  (*unbatchSize)[1] = 1;
  auto secondBatches =
      util::MpcAdapters<T, schedulerId>::unbatching(src1, unbatchSize);
  if ((size & 1) == 0) {
    auto firstBatches =
        util::MpcAdapters<T, schedulerId>::unbatching(src0, unbatchSize);
    auto [first, second] = util::MpcAdapters<T, schedulerId>::obliviousSwap(
        std::move(firstBatches[0]),
        std::move(secondBatches[0]),
        std::move(secondSwapConditions));
    return util::MpcAdapters<T, schedulerId>::batchingWith(
        first,
        {std::move(firstBatches[1]),
         std::move(second),
         std::move(secondBatches[1])});
  } else {
    auto [first, second] = util::MpcAdapters<T, schedulerId>::obliviousSwap(
        std::move(src0),
        std::move(secondBatches[0]),
        std::move(secondSwapConditions));
    return util::MpcAdapters<T, schedulerId>::batchingWith(
        first, {std::move(second), std::move(secondBatches[1])});
  }
}

template <typename T, int schedulerId>
typename AsWaksmanPermuter<T, schedulerId>::SecBatchType
AsWaksmanPermuter<T, schedulerId>::processTwoElements(
    const SecBatchType& src,
    frontend::Bit<true, schedulerId, true>&& swapConditions) const {
  auto unbatchSize = std::make_shared<std::vector<uint32_t>>(2);
  (*unbatchSize)[0] = 1;
  (*unbatchSize)[1] = 1;
  auto batches =
      util::MpcAdapters<T, schedulerId>::unbatching(src, unbatchSize);
  auto [first, second] = util::MpcAdapters<T, schedulerId>::obliviousSwap(
      std::move(batches[0]), std::move(batches[1]), swapConditions);

  return util::MpcAdapters<T, schedulerId>::batchingWith(
      first, {std::move(second)});
}

} // namespace fbpcf::mpc_std_lib::permuter
