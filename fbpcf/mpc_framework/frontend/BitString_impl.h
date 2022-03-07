/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <cassert>
#include <stdexcept>
#include <vector>
#include "fbpcf/mpc_framework/frontend/util.h"

namespace fbpcf::mpc_framework::frontend {

template <bool isSecret, int schedulerId, bool usingBatch>
BitString<isSecret, schedulerId, usingBatch>::BitString(
    const std::vector<BoolType>& data)
    : data_(data.size()) {
  for (size_t i = 0; i < data.size(); i++) {
    if constexpr (usingBatch) {
      if (data.at(i).size() != data.at(0).size()) {
        throw std::runtime_error("All bits has to have the same batch size!");
      }
    }
    data_[i] = Bit<false, schedulerId, usingBatch>(data.at(i));
  }
}

template <bool isSecret, int schedulerId, bool usingBatch>
BitString<isSecret, schedulerId, usingBatch>::BitString(
    const std::vector<BoolType>& data,
    int partyId)
    : data_(data.size()) {
  for (size_t i = 0; i < data.size(); i++) {
    if constexpr (usingBatch) {
      if (data.at(i).size() != data.at(0).size()) {
        throw std::runtime_error("All bits has to have the same batch size!");
      }
    }
    data_[i] = Bit<true, schedulerId, usingBatch>(data.at(i), partyId);
  }
}

template <bool isSecret, int schedulerId, bool usingBatch>
BitString<isSecret, schedulerId, usingBatch>::BitString(
    ExtractedString&& extractedString)
    : data_(extractedString.size()) {
  for (size_t i = 0; i < data_.size(); i++) {
    data_[i] =
        Bit<isSecret, schedulerId, usingBatch>(std::move(extractedString[i]));
  }
}

template <bool isSecret, int schedulerId, bool usingBatch>
BitString<false, schedulerId, usingBatch>
BitString<isSecret, schedulerId, usingBatch>::openToParty(int partyId) const {
  static_assert(isSecret, "No need to open a public value.");
  BitString<false, schedulerId, usingBatch> rst(data_.size());
  for (size_t i = 0; i < data_.size(); i++) {
    rst.data_[i] = data_.at(i).openToParty(partyId);
  }
  return rst;
}

template <bool isSecret, int schedulerId, bool usingBatch>
std::vector<typename BitString<isSecret, schedulerId, usingBatch>::BoolType>
BitString<isSecret, schedulerId, usingBatch>::getValue() const {
  std::vector<BoolType> plaintext(size());

  for (size_t i = 0; i < size(); i++) {
    plaintext[i] = data_.at(i).getValue();
  }
  return plaintext;
}

template <bool isSecret, int schedulerId, bool usingBatch>
typename BitString<true, schedulerId, usingBatch>::ExtractedString
BitString<isSecret, schedulerId, usingBatch>::extractStringShare() const {
  static_assert(isSecret, "No need to extract a public value.");
  std::vector<BoolType> shares(size());

  for (size_t i = 0; i < size(); i++) {
    shares[i] = data_.at(i).extractBit().getValue();
  }
  return ExtractedString(std::move(shares));
}

template <bool isSecret, int schedulerId, bool usingBatch>
BitString<isSecret, schedulerId, usingBatch>
BitString<isSecret, schedulerId, usingBatch>::operator!() const {
  BitString<isSecret, schedulerId, usingBatch> rst(data_.size());

  for (size_t i = 0; i < size(); i++) {
    rst[i] = !data_.at(i);
  }

  return rst;
}

template <bool isSecret, int schedulerId, bool usingBatch>
template <bool isSecretOther>
BitString<isSecret || isSecretOther, schedulerId, usingBatch>
BitString<isSecret, schedulerId, usingBatch>::operator&(
    const BitString<isSecretOther, schedulerId, usingBatch>& src) const {
  if (src.size() != size()) {
    throw std::runtime_error(
        "The two BitStrings need to have the same length for bit-wise AND");
  }
  BitString<isSecret || isSecretOther, schedulerId, usingBatch> rst(
      data_.size());

  for (size_t i = 0; i < size(); i++) {
    rst[i] = data_.at(i) & src.data_.at(i);
  }

  return rst;
}

template <bool isSecret, int schedulerId, bool usingBatch>
template <bool isSecretOther>
BitString<isSecret || isSecretOther, schedulerId, usingBatch>
BitString<isSecret, schedulerId, usingBatch>::
    BitString<isSecret, schedulerId, usingBatch>::operator^(
        const BitString<isSecretOther, schedulerId, usingBatch>& src) const {
  if (src.size() != size()) {
    throw std::runtime_error(
        "The two BitStrings need to have the same length for bit-wise XOR");
  }
  BitString<isSecret || isSecretOther, schedulerId, usingBatch> rst(
      data_.size());

  for (size_t i = 0; i < size(); i++) {
    rst[i] = data_.at(i) ^ src.data_.at(i);
  }

  return rst;
}

template <bool isSecret, int schedulerId, bool usingBatch>
template <bool isSecretChoice, bool isSecretOther>
BitString<isSecret || isSecretChoice || isSecretOther, schedulerId, usingBatch>
BitString<isSecret, schedulerId, usingBatch>::mux(
    const Bit<isSecretChoice, schedulerId, usingBatch>& choice,
    const BitString<isSecretOther, schedulerId, usingBatch>& other) const {
  if (other.size() != size()) {
    throw std::runtime_error(
        "The two BitStrings need to have the same length for MUX");
  }
  BitString<
      isSecret || isSecretChoice || isSecretOther,
      schedulerId,
      usingBatch>
      rst(data_.size());

  for (size_t i = 0; i < size(); i++) {
    rst[i] = data_.at(i) ^ (choice & (other.data_.at(i) ^ data_.at(i)));
  }

  return rst;
}

} // namespace fbpcf::mpc_framework::frontend
