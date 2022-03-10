/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstddef>
#include <stdexcept>
#include "fbpcf/mpc_std_lib/util/util.h"

namespace fbpcf::mpc_std_lib::oram {

template <typename T>
T WriteOnlyOram<T>::publicRead(size_t publicIndex, Role receiver) const {
  if (publicIndex >= size_) {
    throw std::runtime_error("ORAM read index is too large.");
  }
  if (myRole_ != receiver) {
    agent_->sendSingleT<T>(memory_.at(publicIndex));
    // return a default dummy value;
    return T(0);
  } else {
    T v = memory_.at(publicIndex) - agent_->receiveSingleT<T>();
    return myRole_ == Role::Alice ? v : -v;
  }
}

template <typename T>
T WriteOnlyOram<T>::secretRead(size_t publicIndex) const {
  if (publicIndex >= size_) {
    throw std::runtime_error("ORAM read index is too large.");
  }
  T v = memory_.at(publicIndex);
  return myRole_ == Role::Alice ? v : -v;
}

template <typename T>
void WriteOnlyOram<T>::obliviousAddBatch(
    const std::vector<std::vector<bool>>& indexShares,
    const std::vector<std::vector<bool>>& values) {
  if (pow(2, indexShares.size()) < size_) {
    throw std::runtime_error("Input index array size is too small.");
  }

  if ((indexShares.size() == 0) || (values.size() == 0)) {
    throw std::runtime_error("Input cannot be empty");
  }
  auto batchSize = indexShares.at(0).size();
  for (auto& item : indexShares) {
    if (item.size() != batchSize) {
      throw std::runtime_error("Input size is inconsistent!");
    }
  }
  for (auto& item : values) {
    if (item.size() != batchSize) {
      throw std::runtime_error("Input size is inconsistent!");
    }
  }
  auto masks = generateMasks(indexShares, values);
  for (auto& item : masks) {
    if (item.size() != size_) {
      throw std::runtime_error("unexpected mask size");
    }
    for (size_t i = 0; i < size_; i++) {
      // a vector of T may not support memory_[i]+= item.at(i), e.g. T = bool
      memory_[i] = memory_.at(i) + item.at(i);
    }
  }
}

template <typename T>
std::vector<std::vector<T>> WriteOnlyOram<T>::generateMasks(
    const std::vector<std::vector<bool>>& indexShares,
    const std::vector<std::vector<bool>>& values) const {
  size_t batchSize = values.at(0).size();

  auto indicatorKeyPairs =
      generator_->generateSinglePointArrays(indexShares, size_);

  std::vector<std::vector<T>> rst(batchSize, std::vector<T>(size_));
  std::vector<uint32_t> indicatorShares(batchSize);
  std::vector<T> subtrahendShares(batchSize);

  for (size_t i = 0; i < batchSize; i++) {
    indicatorShares[i] = 0;
    subtrahendShares[i] = T(0);

    // We want to compute the difference (between two parties) of
    // subtrahendShares and indicatorShares  at the shared-index-position.
    // However we don't know the secret index. So we calculate the difference of
    // the sums and obliviously pick the ONLY one that differs between two
    // parties.
    for (size_t j = 0; j < size_; j++) {
      rst[i][j] = util::Adapters<T>::generateFromKey(
          indicatorKeyPairs.at(i).second.at(j));
      subtrahendShares[i] = subtrahendShares[i] + rst.at(i).at(j);
      indicatorShares[i] += indicatorKeyPairs.at(i).first.at(j);
    }
  }

  auto difference = calculator_->calculateDifferenceBatch(
      indicatorShares, values, subtrahendShares);

  for (size_t i = 0; i < batchSize; i++) {
    for (size_t j = 0; j < size_; j++) {
      rst[i][j] =
          rst[i][j] + indicatorKeyPairs.at(i).first.at(j) * difference.at(i);
    }
  }
  return rst;
}

} // namespace fbpcf::mpc_std_lib::oram
