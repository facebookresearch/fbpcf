/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/mpc_std_lib/util/util.h"

namespace fbpcf::mpc_std_lib::oram::insecure {

// this function calculate the difference(s) between two batches of values
template <typename T, int8_t indicatorSumWidth>
std::vector<T>
DummyDifferenceCalculator<T, indicatorSumWidth>::calculateDifferenceBatch(
    const std::vector<uint32_t>&
        indicatorShares /*this share is additively shared */,
    const std::vector<std::vector<bool>>&
        minuendShares /* this share is xor-shared */,
    const std::vector<T>&
        subtrahendShares /* this share is additively shared */) const {
  auto batchSize = indicatorShares.size();
  auto boolSize = minuendShares.size();
  if (subtrahendShares.size() != batchSize) {
    throw std::invalid_argument("Inconsistent batch size");
  }
  for (auto& item : minuendShares) {
    if (item.size() != batchSize) {
      throw std::invalid_argument("Inconsistent batch size");
    }
  }

  std::vector<std::vector<bool>> boolBuffer(
      batchSize, std::vector<bool>(boolSize, false));

  // send/receive minuendShares
  for (size_t i = 0; i < boolSize; i++) {
    if (thisPartyToSetDifference_) {
      auto buffer = agent_->receiveBool(batchSize);
      for (size_t j = 0; j < batchSize; j++) {
        boolBuffer[j][i] = minuendShares[i][j] ^ buffer.at(j);
      }
    } else {
      agent_->sendBool(minuendShares.at(i));
    }
  }

  std::vector<T> rst(batchSize);

  // reconstruct indicators and subtrahends
  for (size_t i = 0; i < batchSize; i++) {
    if (thisPartyToSetDifference_) {
      int32_t indicator = agent_->receiveSingleT<int32_t>();
      indicator =
          (indicatorShares.at(i) - indicator + 1) % (1 << indicatorSumWidth) -
          1; // this trick is to get the result to be from {-1, 1}

      T subtrahend = agent_->receiveSingleT<T>();
      subtrahend = subtrahendShares.at(i) - subtrahend;

      T minuend = util::Adapters<T>::convertFromBits(boolBuffer.at(i));
      rst[i] = indicator * (minuend - subtrahend);
    } else {
      agent_->sendSingleT(indicatorShares.at(i));
      agent_->sendSingleT(subtrahendShares.at(i));
    }
  }

  // send the result to the other party
  if (thisPartyToSetDifference_) {
    agent_->sendT(rst);
  } else {
    rst = agent_->receiveT<T>(batchSize);
  }
  return rst;
}

} // namespace fbpcf::mpc_std_lib::oram::insecure
