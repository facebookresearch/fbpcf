/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstddef>
#include <stdexcept>
#include "fbpcf/mpc_framework/mpc_std_lib/util/util.h"

namespace fbpcf::mpc_framework::mpc_std_lib::oram {

template <typename T, int schedulerId>
T LinearOram<T, schedulerId>::publicRead(size_t publicIndex, Role receiver)
    const {
  if (publicIndex >= size_) {
    throw std::runtime_error("ORAM read index is too large.");
  }
  if (myRole_ != receiver) {
    agent_->sendSingleT<T>(memory_.at(publicIndex));
    // return a default dummy value;
    return T(0);
  } else {
    return memory_.at(publicIndex) + agent_->receiveSingleT<T>();
  }
}

template <typename T, int schedulerId>
T LinearOram<T, schedulerId>::secretRead(size_t publicIndex) const {
  if (publicIndex >= size_) {
    throw std::runtime_error("ORAM read index is too large.");
  }
  return memory_.at(publicIndex);
}

template <typename T, int schedulerId>
void LinearOram<T, schedulerId>::obliviousAddBatch(
    const std::vector<std::vector<bool>>& indexShares,
    const std::vector<std::vector<bool>>& valueShares) {
  if (pow(2, indexShares.size()) < size_) {
    throw std::runtime_error("Input index array size is too small.");
  }

  if ((indexShares.size() == 0) || (valueShares.size() == 0)) {
    throw std::runtime_error("Input cannot be empty");
  }
  auto batchSize = indexShares.at(0).size();

  std::vector<frontend::Bit<true, schedulerId, true>> index;
  for (size_t i = 0; i < indexShares.size(); i++) {
    if (indexShares.at(i).size() != batchSize) {
      throw std::runtime_error("Input size is inconsistent!");
    }
    auto shares = indexShares[i];
    index.push_back(frontend::Bit<true, schedulerId, true>(
        typename frontend::Bit<true, schedulerId, true>::ExtractedBit(
            std::move(shares))));
  }

  // this is the value to add
  auto value =
      util::MpcAdapters<T, schedulerId>::recoverBatchSharedSecrets(valueShares);

  // this is the value to add for each index - if the index is the specified
  // one, add the value; otherwise, add zero.
  auto valueForEachIndex =
      generateInputValue(std::move(value), std::move(index), size_, batchSize);

  // party 1's share
  std::vector<SecBatchT> shares1;

  for (size_t i = 0; i < valueForEachIndex.size(); i++) {
    std::vector<T> share0ForThisIndex(batchSize, T(0));
    // generate a random value as party 0's share
    if (myRole_ == Role::Alice) {
      for (auto& item : share0ForThisIndex) {
        item = util::Adapters<T>::generateFromKey(prg_->getRandomM128i());
        memory_[i] = memory_.at(i) + item;
      }
    }
    auto share0 = util::MpcAdapters<T, schedulerId>::processSecretInputs(
        share0ForThisIndex, party0Id_);
    // calculate party 1's share
    shares1.push_back(valueForEachIndex.at(i) - share0);
  }

  for (size_t i = 0; i < size_; i++) {
    // open the difference to party 1
    auto share1Plaintext = util::MpcAdapters<T, schedulerId>::openToParty(
        shares1.at(i), party1Id_);
    if (myRole_ == Role::Bob) {
      for (auto& item : share1Plaintext) {
        memory_[i] = memory_.at(i) + item;
      }
    }
  }
}

template <typename T, int schedulerId>
std::vector<typename LinearOram<T, schedulerId>::SecBatchT>
LinearOram<T, schedulerId>::generateInputValue(
    SecBatchT&& src,
    const std::vector<frontend::Bit<true, schedulerId, true>>&& conditions,
    size_t range,
    size_t batchSize) const {
  if ((range == 0) || (batchSize == 0)) {
    throw std::runtime_error("Invalid inputs!");
  }

  // this is the default zero value
  auto zero = util::MpcAdapters<T, schedulerId>::processSecretInputs(
      std::vector<T>(batchSize, T(0)), party0Id_);

  std::vector<typename LinearOram<T, schedulerId>::SecBatchT> rst;
  rst.push_back(std::move(src));
  uint32_t bitWidth = std::ceil(log2(range));
  for (int i = bitWidth - 1; i >= 0; i--) {
    rst = conditionalExpansionOneLayer(std::move(rst), zero, conditions.at(i));
    // find the smallest number that is no smaller than  ceil(range / 2^i)
    size_t largestNeeded = (range + (1 << i) - 1) >> i;
    if (rst.size() > largestNeeded) {
      rst.erase(rst.begin() + largestNeeded, rst.end());
    }
  }
  return rst;
}

template <typename T, int schedulerId>
std::vector<typename LinearOram<T, schedulerId>::SecBatchT>
LinearOram<T, schedulerId>::conditionalExpansionOneLayer(
    std::vector<SecBatchT>&& src,
    const SecBatchT& zero,
    const frontend::Bit<true, schedulerId, true>& condition) const {
  std::vector<typename LinearOram<T, schedulerId>::SecBatchT> rst;
  for (auto& item : src) {
    auto [t0, t1] =
        util::MpcAdapters<T, schedulerId>::obliviousSwap(item, zero, condition);
    rst.push_back(std::move(t0));
    rst.push_back(std::move(t1));
  }
  return rst;
}

} // namespace fbpcf::mpc_framework::mpc_std_lib::oram
