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

  // this are all the oram indexes
  auto oramIndexes = Helper::generateIndex(batchSize, size_);

  // this is the default zero value
  auto zero = util::MpcAdapters<T, schedulerId>::processSecretInputs(
      std::vector<T>(batchSize, T(0)), party0Id_);

  // this is the value to add for each index - if the index is the specified
  // one, add the value; otherwise, add zero.
  std::vector<typename util::SecBatchType<T, schedulerId>::type>
      valueForEachIndex;
  for (auto& item : oramIndexes) {
    auto comparisonResult = Helper::comparison(index, item);
    valueForEachIndex.push_back(value.mux(comparisonResult, zero));
  }

  // party 1's share
  std::vector<typename util::SecBatchType<T, schedulerId>::type> shares1;

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
std::vector<std::vector<frontend::Bit<false, schedulerId, true>>>
LinearOram<T, schedulerId>::Helper::generateIndex(
    size_t batchSize,
    size_t range) {
  auto bitWidth = std::ceil(log2(range));
  std::vector<std::vector<frontend::Bit<false, schedulerId, true>>> rst(
      range, std::vector<frontend::Bit<false, schedulerId, true>>(bitWidth));
  for (size_t i = 0; i < range; i++) {
    size_t index = i;
    for (size_t j = 0; j < bitWidth; j++, index >>= 1) {
      std::vector<bool> plaintext(batchSize, index & 1);
      rst[i][j] = frontend::Bit<false, schedulerId, true>(plaintext);
    }
  }
  return rst;
}

template <typename T, int schedulerId>
frontend::Bit<true, schedulerId, true>
LinearOram<T, schedulerId>::Helper::comparison(
    const std::vector<frontend::Bit<true, schedulerId, true>>& src1,
    const std::vector<frontend::Bit<false, schedulerId, true>>& src2) {
  auto size = src1.size();
  std::vector<frontend::Bit<true, schedulerId, true>> rst(size);

  frontend::equalityCheck<
      std::vector<frontend::Bit<true, schedulerId, true>>,
      std::vector<frontend::Bit<true, schedulerId, true>>,
      std::vector<frontend::Bit<false, schedulerId, true>>>(rst, src1, src2);

  return !rst.at(0);
}

} // namespace fbpcf::mpc_framework::mpc_std_lib::oram
