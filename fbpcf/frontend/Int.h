/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <algorithm>
#include <stdexcept>
#include <type_traits>

#include "fbpcf/frontend/Bit.h"
#include "fbpcf/frontend/util.h"

namespace fbpcf::frontend {

template <
    bool isSigned,
    int8_t width,
    bool isSecret,
    int schedulerId,
    bool usingBatch = false>
class Int {
  using UnitIntType =
      typename std::conditional<isSigned, int64_t, uint64_t>::type;
  using IntType = typename std::
      conditional<usingBatch, std::vector<UnitIntType>, UnitIntType>::type;
  using BoolType =
      typename std::conditional<usingBatch, std::vector<bool>, bool>::type;

  static_assert(width <= 64);

  template <typename T>
  struct UnitInputTypeChecker
      : std::conditional<
            std::is_integral_v<T> && (std::is_signed_v<T> == isSigned),
            std::true_type,
            std::false_type>::type {};

  template <typename T>
  struct VectorInputTypeChecker : std::false_type {};

  template <typename T>
  struct VectorInputTypeChecker<std::vector<T>>
      : std::conditional<
            usingBatch && std::is_integral_v<T> &&
                (std::is_signed_v<T> == isSigned),
            std::true_type,
            std::false_type>::type {};

  template <typename T>
  struct InputTypeChecker : std::conditional<
                                usingBatch,
                                VectorInputTypeChecker<T>,
                                UnitInputTypeChecker<T>>::type {};

 public:
  class ExtractedInt {
   public:
    ExtractedInt() = default;

    template <typename T>
    explicit ExtractedInt(const T& v) {
      static_assert(
          InputTypeChecker<T>::value,
          "Need to use proper signed/unsigned integer (vector).");
      for (int8_t i = 0; i < width; i++) {
        data_[i] = typename Bit<true, schedulerId, usingBatch>::ExtractedBit(
            extractLsb(v, i));
      }
    }

    typename Bit<isSecret, schedulerId, usingBatch>::ExtractedBit& operator[](
        int index) {
      return data_[index];
    }

    std::vector<BoolType> getBooleanShares() const {
      std::vector<BoolType> output;
      for (int8_t i = 0; i < width; i++) {
        output.push_back(data_[i].getValue());
      }
      return output;
    }

    IntType getValue() const {
      return convertBitsToInt<
          typename Bit<true, schedulerId, usingBatch>::ExtractedBit>(data_);
    }

   private:
    std::array<typename Bit<true, schedulerId, usingBatch>::ExtractedBit, width>
        data_;
  };

  /**
   * Create an uninitialized integer
   */
  Int() {}

  /**
   * Create an integer with a public value v.
   */
  template <typename T>
  explicit Int(const T& src);

  /**
   * Create an integer/a batch of integers with a private value v from party
   * corresponding to partyId; other parties input will be ignored.
   */
  template <typename T>
  Int(const T& v, int partyId);

  /**
   * Construct a private int from extracted shares.
   */
  explicit Int(ExtractedInt&& extractedInt);

  void publicInput(const IntType& v);

  void privateInput(const IntType& v, int partyId);

  template <bool isSecretOther>
  Int<isSigned, width, isSecret || isSecretOther, schedulerId, usingBatch>
  operator+(const Int<isSigned, width, isSecretOther, schedulerId, usingBatch>&
                other) const;

  template <bool isSecretOther>
  Int<isSigned, width, isSecret || isSecretOther, schedulerId, usingBatch>
  operator-(const Int<isSigned, width, isSecretOther, schedulerId, usingBatch>&
                other) const;

  template <bool isSecretOther>
  Bit<isSecret || isSecretOther, schedulerId, usingBatch> operator<(
      const Int<isSigned, width, isSecretOther, schedulerId, usingBatch>& other)
      const;

  template <bool isSecretOther>
  Bit<isSecret || isSecretOther, schedulerId, usingBatch> operator<=(
      const Int<isSigned, width, isSecretOther, schedulerId, usingBatch>& other)
      const;

  template <bool isSecretOther>
  Bit<isSecret || isSecretOther, schedulerId, usingBatch> operator>(
      const Int<isSigned, width, isSecretOther, schedulerId, usingBatch>& other)
      const;

  template <bool isSecretOther>
  Bit<isSecret || isSecretOther, schedulerId, usingBatch> operator>=(
      const Int<isSigned, width, isSecretOther, schedulerId, usingBatch>& other)
      const;

  template <bool isSecretOther>
  Bit<isSecret || isSecretOther, schedulerId, usingBatch> operator==(
      const Int<isSigned, width, isSecretOther, schedulerId, usingBatch>& other)
      const;

  /**
   * run multiplexer between this integer and another one. Return this integer
   * if choice is 0 and the other one if choice is 1.
   */
  template <bool isSecretChoice, bool isSecretOther>
  Int<isSigned,
      width,
      isSecret || isSecretChoice || isSecretOther,
      schedulerId,
      usingBatch>
  mux(const Bit<isSecretChoice, schedulerId, usingBatch>& choice,
      const Int<isSigned, width, isSecretOther, schedulerId, usingBatch>& other)
      const;

  const Bit<isSecret, schedulerId, usingBatch>& operator[](size_t index) const {
    if (index >= width) {
      throw std::invalid_argument("Index exceeds width.");
    }
    return data_[index];
  }

  Bit<isSecret, schedulerId, usingBatch>& operator[](size_t index) {
    if (index >= width) {
      throw std::invalid_argument("Index exceeds width.");
    }
    return data_[index];
  }

  /**
   * Create a new integer that will carry the plaintext signal of this bit.
   * However only party with partyId will receive the actual value, other
   * parties will receive a dummy value.
   */
  Int<isSigned, width, false, schedulerId, usingBatch> openToParty(
      int partyId) const;

  /**
   * get the plaintext value associated with this integer
   */
  IntType getValue() const;

  /**
   * extract this party's share of this int
   */
  typename Int<isSigned, width, true, schedulerId, usingBatch>::ExtractedInt
  extractIntShare() const;

  Int<isSigned, width, isSecret, schedulerId, usingBatch> batchingWith(
      const std::vector<
          Int<isSigned, width, isSecret, schedulerId, usingBatch>>& others)
      const;

  std::vector<Int<isSigned, width, isSecret, schedulerId, usingBatch>>
  unbatching(std::shared_ptr<std::vector<uint32_t>> unbatchingStrategy) const;

 private:
  template <typename T>
  std::vector<UnitIntType> convertTo64BitIntVector(
      const std::vector<T>& src) const;

  // check if the input is valid and make necessary change to convert a
  // signed type to a unsigned value
  void processInput(IntType& v) const;

  // check if a single input is valid and make necessary change to convert a
  // signed type to a unsigned value
  void processSingleInput(UnitIntType& v) const;

  void convertPublicIntToBits(const IntType& v);
  void convertPrivateIntToBits(const IntType& v, int partyId);

  template <typename T>
  static IntType convertBitsToInt(const std::array<T, width>& data);

  // not defining an operator because we only need a private helper
  static void shiftLeft(std::vector<uint64_t>& data);

  static void addLsb(
      std::vector<uint64_t>& data,
      const std::vector<bool>& bits);

  // extract the t-th lsb(s) of v (either return a bool or a std::vector<bool>)
  static BoolType extractLsb(const IntType& v, size_t t);

  std::array<Bit<isSecret, schedulerId, usingBatch>, width> data_;

  // a uint64_t integer such that the last width bits are 1. Written in this
  // format to prevent overflow when width = 64
  static const uint64_t kMask = ((((uint64_t)1 << (width - 1)) - 1) << 1) + 1;

  friend class Int<isSigned, width, !isSecret, schedulerId, usingBatch>;
};

// this struct works as a helper to build a more readable frontend
template <typename T, bool isSecret, int schedulerId>
struct IntTypeHelper;

template <int8_t width, bool isSecret, int schedulerId>
struct IntTypeHelper<Signed<width>, isSecret, schedulerId> {
  using type = Int<true, width, isSecret, schedulerId, false>;
};

template <int8_t width, bool isSecret, int schedulerId>
struct IntTypeHelper<Unsigned<width>, isSecret, schedulerId> {
  using type = Int<false, width, isSecret, schedulerId, false>;
};

template <int8_t width, bool isSecret, int schedulerId>
struct IntTypeHelper<Batch<Signed<width>>, isSecret, schedulerId> {
  using type = Int<true, width, isSecret, schedulerId, true>;
};

template <int8_t width, bool isSecret, int schedulerId>
struct IntTypeHelper<Batch<Unsigned<width>>, isSecret, schedulerId> {
  using type = Int<false, width, isSecret, schedulerId, true>;
};

template <typename T, int schedulerId>
using Integer =
    typename IntTypeHelper<typename T::type, IsSecret<T>::value, schedulerId>::
        type;

} // namespace fbpcf::frontend

#include "fbpcf/frontend/Int_impl.h"
