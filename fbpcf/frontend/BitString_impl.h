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
#include "fbpcf/frontend/util.h"

// included for clangd resolution. Should not execute during compilation
#include "fbpcf/frontend/BitString.h"

namespace fbpcf::frontend {

template <bool isSecret, int schedulerId, bool usingBatch>
BitString<isSecret, schedulerId, usingBatch>::BitString(
    const std::vector<BoolType>& data) {
  if constexpr (usingBatch) {
    data_ =
        std::vector<Bit<isSecret, schedulerId, usingBatch>>(data.at(0).size());
    auto transposed = transposeVector(data);
    for (size_t i = 0; i < data_.size(); i++) {
      data_[i] = Bit<isSecret, schedulerId, usingBatch>(transposed.at(i));
    }
  } else {
    data_ = std::vector<Bit<isSecret, schedulerId, usingBatch>>(data.size());
    for (size_t i = 0; i < data_.size(); i++) {
      data_[i] = Bit<isSecret, schedulerId, usingBatch>(data.at(i));
    }
  }
}

template <bool isSecret, int schedulerId, bool usingBatch>
BitString<isSecret, schedulerId, usingBatch>::BitString(
    const std::vector<BoolType>& data,
    int partyId) {
  if constexpr (usingBatch) {
    data_ =
        std::vector<Bit<isSecret, schedulerId, usingBatch>>(data.at(0).size());
    auto transposed = transposeVector(data);
    for (size_t i = 0; i < data_.size(); i++) {
      data_[i] =
          Bit<isSecret, schedulerId, usingBatch>(transposed.at(i), partyId);
    }
  } else {
    data_ = std::vector<Bit<isSecret, schedulerId, usingBatch>>(data.size());
    for (size_t i = 0; i < data_.size(); i++) {
      data_[i] = Bit<isSecret, schedulerId, usingBatch>(data.at(i), partyId);
    }
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
  if constexpr (usingBatch) {
    return transposeVector(std::move(plaintext));
  } else {
    return plaintext;
  }
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
#ifndef USE_COMPOSITE_AND_FOR_MUX
  return slowMux(choice, other);
#else
  return fastMux(choice, other);
#endif
}

template <bool isSecret, int schedulerId, bool usingBatch>
template <bool isSecretChoice, bool isSecretOther>
BitString<isSecret || isSecretChoice || isSecretOther, schedulerId, usingBatch>
BitString<isSecret, schedulerId, usingBatch>::slowMux(
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

template <bool isSecret, int schedulerId, bool usingBatch>
template <bool isSecretChoice, bool isSecretOther>
BitString<isSecret || isSecretChoice || isSecretOther, schedulerId, usingBatch>
BitString<isSecret, schedulerId, usingBatch>::fastMux(
    const Bit<isSecretChoice, schedulerId, usingBatch>& choice,
    const BitString<isSecretOther, schedulerId, usingBatch>& other) const {
  if (other.size() != size()) {
    throw std::runtime_error(
        "The two BitStrings need to have the same length for MUX");
  }
  BitString<isSecret || isSecretOther, schedulerId, usingBatch> sum(
      data_.size());

  for (size_t i = 0; i < size(); i++) {
    sum[i] = other[i] ^ data_[i];
  }

  BitString<
      isSecret || isSecretChoice || isSecretOther,
      schedulerId,
      usingBatch>
      rst(data_.size());

  rst.data_ = choice & sum.data_;

  for (size_t i = 0; i < size(); i++) {
    rst[i] = data_[i] ^ rst[i];
  }

  return rst;
}

template <bool isSecret, int schedulerId, bool usingBatch>
BitString<isSecret, schedulerId, usingBatch>
BitString<isSecret, schedulerId, usingBatch>::batchingWith(
    const std::vector<BitString<isSecret, schedulerId, usingBatch>>& others)
    const {
  static_assert(usingBatch, "Only batch values needs to rebatch!");

  for (auto& item : others) {
    if (item.data_.size() != data_.size()) {
      throw std::runtime_error(
          "The BitStrings need to have the same length to batch together.");
    }
  }

  BitString<isSecret, schedulerId, usingBatch> rst(data_.size());
  size_t batchSize = others.size();
  std::vector<Bit<true, schedulerId, usingBatch>> bits(batchSize);
  for (size_t i = 0; i < data_.size(); i++) {
    for (size_t j = 0; j < batchSize; j++) {
      bits[j] = others.at(j).data_.at(i);
    }
    rst.data_[i] = data_.at(i).batchingWith(bits);
  }
  return rst;
}

template <bool isSecret, int schedulerId, bool usingBatch>
std::vector<BitString<isSecret, schedulerId, usingBatch>>
BitString<isSecret, schedulerId, usingBatch>::unbatching(
    std::shared_ptr<std::vector<uint32_t>> unbatchingStrategy) const {
  static_assert(usingBatch, "Only batch values needs to rebatch!");
  std::vector<BitString<isSecret, schedulerId, usingBatch>> rst(
      unbatchingStrategy->size());
  for (auto& item : rst) {
    item.resize(data_.size());
  }

  for (size_t i = 0; i < data_.size(); i++) {
    auto bitVec = data_.at(i).unbatching(unbatchingStrategy);
    for (size_t j = 0; j < unbatchingStrategy->size(); j++) {
      rst.at(j).data_.at(i) = bitVec.at(j);
    }
  }
  return rst;
}

template <bool isSecret, int schedulerId, bool usingBatch>
std::vector<std::vector<bool>>
BitString<isSecret, schedulerId, usingBatch>::transposeVector(
    const std::vector<std::vector<bool>>& src) {
  size_t outerSize = src.size();
  size_t innerSize = src.at(0).size();
  std::vector<std::vector<bool>> rst(innerSize, std::vector<bool>(outerSize));
  for (size_t i = 0; i < outerSize; i++) {
    if (src.at(i).size() != innerSize) {
      throw std::runtime_error("All batches have to have the same size!");
    }
    for (size_t j = 0; j < innerSize; j++) {
      rst[j][i] = src.at(i).at(j);
    }
  }
  return rst;
}

} // namespace fbpcf::frontend
