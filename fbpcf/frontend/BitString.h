/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <functional>
#include <memory>
#include <type_traits>
#include <vector>
#include "fbpcf/frontend/Bit.h"

namespace fbpcf::frontend {

/**
 * BitString type is merely a vector of bits with some basic APIs like bit-wise
 * XOR/AND/NOT/MUX.
 */
template <bool isSecret, int schedulerId, bool usingBatch = false>
class BitString : public scheduler::SchedulerKeeper<schedulerId> {
  using BoolType =
      typename std::conditional<usingBatch, std::vector<bool>, bool>::type;

 public:
  class ExtractedString {
   public:
    ExtractedString() = default;

    explicit ExtractedString(const std::vector<BoolType>& data)
        : data_(data.size()) {
      for (size_t i = 0; i < data_.size(); i++) {
        if constexpr (usingBatch) {
          if (data.at(i).size() != data.at(0).size()) {
            throw std::runtime_error(
                "All bits has to have the same batch size!");
          }
        }
        data_[i] = typename Bit<true, schedulerId, usingBatch>::ExtractedBit(
            data.at(i));
      }
    }

    typename Bit<isSecret, schedulerId, usingBatch>::ExtractedBit& operator[](
        int index) {
      return data_[index];
    }

    size_t size() const {
      return data_.size();
    }

   private:
    std::vector<typename Bit<true, schedulerId, usingBatch>::ExtractedBit>
        data_;
  };

  BitString() : data_() {}

  explicit BitString(size_t size) : data_(size) {}

  // For batch type constructor, inner vector size should be the batch size.
  // Outer vector size is BitString length.
  explicit BitString(const std::vector<BoolType>& data);

  // For batch type constructor, inner vector size should be the batch size.
  // Outer vector size is BitString length.
  BitString(const std::vector<BoolType>& data, int partyId);

  explicit BitString(ExtractedString&& extractedString);

  /**
   * Create a new BitString that will carry the plaintext signal of this bit.
   * However only party with partyId will receive the actual value, other
   * parties will receive a dummy value.
   */
  BitString<false, schedulerId, usingBatch> openToParty(int partyId) const;

  /**
   * get the plaintext value associated with this BitString.
   * For batch type constructor, inner vector size is the batch size.
   * Outer vector size is BitString length.
   */
  std::vector<BoolType> getValue() const;

  /**
   * extract this party's share of this BitString
   */
  typename BitString<true, schedulerId, usingBatch>::ExtractedString
  extractStringShare() const;

  size_t size() const {
    return data_.size();
  }

  // if this BitString becomes larger after resizing, then the extra bits have
  // undefined value.
  void resize(size_t size) {
    data_.resize(size);
  }

  const Bit<isSecret, schedulerId, usingBatch>& at(size_t i) const {
    return data_.at(i);
  }

  Bit<isSecret, schedulerId, usingBatch>& operator[](size_t i) {
    return data_[i];
  }

  const Bit<isSecret, schedulerId, usingBatch>& operator[](size_t i) const {
    return data_[i];
  }

  BitString<isSecret, schedulerId, usingBatch> operator!() const;

  template <bool isSecretOther>
  BitString<isSecret || isSecretOther, schedulerId, usingBatch> operator&(
      const BitString<isSecretOther, schedulerId, usingBatch>& src) const;

  template <bool isSecretOther>
  BitString<isSecret || isSecretOther, schedulerId, usingBatch> operator^(
      const BitString<isSecretOther, schedulerId, usingBatch>& src) const;

  /**
   * run multiplexer between this bitstring and another one. Return this string
   * if choice is 0 and the other one if choice is 1.
   */
  template <bool isSecretChoice, bool isSecretOther>
  BitString<
      isSecret || isSecretChoice || isSecretOther,
      schedulerId,
      usingBatch>
  mux(const Bit<isSecretChoice, schedulerId, usingBatch>& choice,
      const BitString<isSecretOther, schedulerId, usingBatch>& other) const;

  /**
   * Combines a single BitString with a vector of BitString to create a single
   * BitString with Batch size equal to the sum of the batch sizes of passed in
   * BitStrings
   */
  BitString<isSecret, schedulerId, usingBatch> batchingWith(
      const std::vector<BitString<isSecret, schedulerId, usingBatch>>& others)
      const;

  /**
   * Splits a batched bit string into smaller batches based on the passed in
   * vector of integers. Each element of the unbatching strategy indicates the
   * sub batch size.
   *
   */
  std::vector<BitString<isSecret, schedulerId, usingBatch>> unbatching(
      std::shared_ptr<std::vector<uint32_t>> unbatchingStrategy) const;

  /**
   * get the batch size of this BitString, requires usingBatch is True
   */
  size_t getBatchSize() const;

 private:
  std::vector<Bit<isSecret, schedulerId, usingBatch>> data_;

  friend class BitString<!isSecret, schedulerId, usingBatch>;
};

} // namespace fbpcf::frontend

#include "fbpcf/frontend/BitString_impl.h"
