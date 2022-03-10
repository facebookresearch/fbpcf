/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <cassert>
#include <string>
#include <vector>
#include "fbpcf/mpc_framework/frontend/util.h"

namespace fbpcf::frontend {

template <
    bool isSigned,
    int8_t width,
    bool isSecret,
    int schedulerId,
    bool usingBatch>
template <typename T>
Int<isSigned, width, isSecret, schedulerId, usingBatch>::Int(const T& v) {
  static_assert(
      InputTypeChecker<T>::value,
      "Need to use proper signed/unsigned integer (vector).");
  if constexpr (usingBatch) {
    publicInput(convertTo64BitIntVector(v));
  } else {
    publicInput(v);
  }
}

template <
    bool isSigned,
    int8_t width,
    bool isSecret,
    int schedulerId,
    bool usingBatch>
template <typename T>
Int<isSigned, width, isSecret, schedulerId, usingBatch>::Int(
    const T& v,
    int partyId) {
  static_assert(
      InputTypeChecker<T>::value,
      "Need to use proper signed/unsigned integer (vector).");
  if constexpr (usingBatch) {
    privateInput(convertTo64BitIntVector(v), partyId);
  } else {
    privateInput(v, partyId);
  }
}

template <
    bool isSigned,
    int8_t width,
    bool isSecret,
    int schedulerId,
    bool usingBatch>
Int<isSigned, width, isSecret, schedulerId, usingBatch>::Int(
    ExtractedInt&& extractedInt) {
  for (int8_t i = 0; i < width; i++) {
    data_[i] =
        Bit<isSecret, schedulerId, usingBatch>(std::move(extractedInt[i]));
  }
}

template <
    bool isSigned,
    int8_t width,
    bool isSecret,
    int schedulerId,
    bool usingBatch>
void Int<isSigned, width, isSecret, schedulerId, usingBatch>::publicInput(
    const IntType& v) {
  IntType buf = v;
  processInput(buf);
  convertPublicIntToBits(buf);
}

template <
    bool isSigned,
    int8_t width,
    bool isSecret,
    int schedulerId,
    bool usingBatch>
void Int<isSigned, width, isSecret, schedulerId, usingBatch>::privateInput(
    const IntType& v,
    int partyId) {
  IntType buf = v;
  processInput(buf);
  convertPrivateIntToBits(buf, partyId);
}

template <
    bool isSigned,
    int8_t width,
    bool isSecret,
    int schedulerId,
    bool usingBatch>
template <bool isSecretOther>
Int<isSigned, width, isSecret || isSecretOther, schedulerId, usingBatch>
Int<isSigned, width, isSecret, schedulerId, usingBatch>::operator+(
    const Int<isSigned, width, isSecretOther, schedulerId, usingBatch>& other)
    const {
  // signed int add and unsigned int add are the same
  Int<isSigned, width, isSecret || isSecretOther, schedulerId, usingBatch> rst;

  rst.data_[0] = data_.at(0) ^ other.data_.at(0);
  auto carry = data_.at(0) & other.data_.at(0);

  for (int8_t i = 1; i < width - 1; i++) {
    auto left = carry ^ data_.at(i);
    auto right = carry ^ other.data_.at(i);
    rst.data_[i] = left ^ other.data_.at(i);
    carry = (left & right) ^ carry;
  }
  auto left = carry ^ data_[width - 1];
  rst.data_[width - 1] = left ^ other.data_[width - 1];
  return rst;
}

template <
    bool isSigned,
    int8_t width,
    bool isSecret,
    int schedulerId,
    bool usingBatch>
template <bool isSecretOther>
Int<isSigned, width, isSecret || isSecretOther, schedulerId, usingBatch>
Int<isSigned, width, isSecret, schedulerId, usingBatch>::operator-(
    const Int<isSigned, width, isSecretOther, schedulerId, usingBatch>& other)
    const {
  // signed int add and unsigned int subtract are the same
  Int<isSigned, width, isSecret || isSecretOther, schedulerId, usingBatch> rst;

  rst.data_[0] = data_.at(0) ^ other.data_.at(0);
  auto carry = !data_.at(0) & other.data_.at(0);

  for (int8_t i = 1; i < width - 1; i++) {
    // the logic here is:
    // 1. rst.data_[i] is the xor of minuend, subtrahend, and carry over;
    // 2. the new carry over is the old carry over if minuend = subtrahend;
    // 3. otherwise, the new carry over is the subtrahend, no matter what's the
    // carry over: if subtrahend is 0, then minuend is 1, thus carry over must
    // be 0; similarly, if subtrahend is 1, then minuend is 0, thus carry over
    // must be 1;
    auto tmp = carry ^ other.data_.at(i);
    rst.data_[i] = data_.at(i) ^ tmp;
    carry = carry ^ ((data_.at(i) ^ other.data_.at(i)) & tmp);
  }
  rst.data_[width - 1] =
      carry ^ data_.at(width - 1) ^ other.data_.at(width - 1);
  return rst;
}

/**
 * The algorithm of comparising two unsigned integers comes as follows:
 * 1. compare the msb: equal, or one is larger than the other.
 * 2. if the msbs are equal, recursively compare the remaining bits.
 * Since we are running an oblivious computation, the comparison result is
 * unknown. Therefore we have to compare all the bits no matter the results.
 * That results in our comparison results starts from the lsbs. In the i-th
 * iteration, we get the comparison results of the least i significant bits in
 * the two integers, stored in carry variable. Then we use that result to
 * compute the comparison result of the i+1-th iteration: if the two (i+1)-th
 * least significant bits are same,then the comparison result in the (i+1)-th
 * iteration is still carry, otherwise it would be the comparison result of the
 * two (i+1)-th lsbs. In summary we have: carry_{i+1} = left[i] == right[i] ?
 * carry_{i} : right[i].
 * The actual formula we use: carry = (carry ^ left[i]) & (carry ^ right[i]) ^
 * right[i] realize the desired behavior: if left[i] and right[i] are the same,
 * then the result would be the same as (carry ^ right[i]) & (carry ^ right[i])
 * ^ right[i] = carry; otherwise carry ^ left[i] and carry ^ right[i] will be
 * opposite (thus their product will always be 0), and the whole formula can be
 * simplified to right[i]
 */
template <
    bool isSigned,
    int8_t width,
    bool isSecret,
    int schedulerId,
    bool usingBatch>
template <bool isSecretOther>
Bit<isSecret || isSecretOther, schedulerId, usingBatch>
Int<isSigned, width, isSecret, schedulerId, usingBatch>::operator<(
    const Int<isSigned, width, isSecretOther, schedulerId, usingBatch>& other)
    const {
  auto carry = (!data_[0]) & other.data_[0];
  for (int8_t i = 1; i < width - 1; i++) {
    carry = ((carry ^ data_.at(i)) & (carry ^ other.data_.at(i))) ^
        other.data_.at(i);
  }
  // the msb's meaning is different in signed and unsigend int
  if constexpr (isSigned) {
    carry = ((carry ^ data_[width - 1]) & (carry ^ other.data_[width - 1])) ^
        data_[width - 1];
  } else {
    carry = ((carry ^ data_[width - 1]) & (carry ^ other.data_[width - 1])) ^
        other.data_[width - 1];
  }
  return carry;
}

template <
    bool isSigned,
    int8_t width,
    bool isSecret,
    int schedulerId,
    bool usingBatch>
template <bool isSecretOther>
Bit<isSecret || isSecretOther, schedulerId, usingBatch>
Int<isSigned, width, isSecret, schedulerId, usingBatch>::operator<=(
    const Int<isSigned, width, isSecretOther, schedulerId, usingBatch>& other)
    const {
  return !(other < *this);
}

template <
    bool isSigned,
    int8_t width,
    bool isSecret,
    int schedulerId,
    bool usingBatch>
template <bool isSecretOther>
Bit<isSecret || isSecretOther, schedulerId, usingBatch>
Int<isSigned, width, isSecret, schedulerId, usingBatch>::operator>(
    const Int<isSigned, width, isSecretOther, schedulerId, usingBatch>& other)
    const {
  return (other < *this);
}

template <
    bool isSigned,
    int8_t width,
    bool isSecret,
    int schedulerId,
    bool usingBatch>
template <bool isSecretOther>
Bit<isSecret || isSecretOther, schedulerId, usingBatch>
Int<isSigned, width, isSecret, schedulerId, usingBatch>::operator>=(
    const Int<isSigned, width, isSecretOther, schedulerId, usingBatch>& other)
    const {
  return !(*this < other);
}

template <
    bool isSigned,
    int8_t width,
    bool isSecret,
    int schedulerId,
    bool usingBatch>
template <bool isSecretOther>
Bit<isSecret || isSecretOther, schedulerId, usingBatch>
Int<isSigned, width, isSecret, schedulerId, usingBatch>::operator==(
    const Int<isSigned, width, isSecretOther, schedulerId, usingBatch>& other)
    const {
  std::array<Bit<isSecret || isSecretOther, schedulerId, usingBatch>, width>
      rst;
  equalityCheck<
      std::
          array<Bit<isSecret || isSecretOther, schedulerId, usingBatch>, width>,
      std::array<Bit<isSecret, schedulerId, usingBatch>, width>,
      std::array<Bit<isSecretOther, schedulerId, usingBatch>, width>>(
      rst, data_, other.data_);
  return rst.at(0);
}

template <
    bool isSigned,
    int8_t width,
    bool isSecret,
    int schedulerId,
    bool usingBatch>
template <bool isSecretChoice, bool isSecretOther>
Int<isSigned,
    width,
    isSecret || isSecretChoice || isSecretOther,
    schedulerId,
    usingBatch>
Int<isSigned, width, isSecret, schedulerId, usingBatch>::mux(
    const Bit<isSecretChoice, schedulerId, usingBatch>& choice,
    const Int<isSigned, width, isSecretOther, schedulerId, usingBatch>& other)
    const {
  Int<isSigned,
      width,
      isSecret || isSecretChoice || isSecretOther,
      schedulerId,
      usingBatch>
      rst;

  for (int8_t i = 0; i < width; i++) {
    rst.data_[i] = data_.at(i) ^ (choice & (other.data_.at(i) ^ data_.at(i)));
  }
  return rst;
}

template <
    bool isSigned,
    int8_t width,
    bool isSecret,
    int schedulerId,
    bool usingBatch>
typename Int<isSigned, width, isSecret, schedulerId, usingBatch>::IntType
Int<isSigned, width, isSecret, schedulerId, usingBatch>::getValue() const {
  static_assert(!isSecret, "Shouldn't try to get a secret value!");
  return convertBitsToInt<Bit<isSecret, schedulerId, usingBatch>>(data_);
}

template <
    bool isSigned,
    int8_t width,
    bool isSecret,
    int schedulerId,
    bool usingBatch>
Int<isSigned, width, false, schedulerId, usingBatch>
Int<isSigned, width, isSecret, schedulerId, usingBatch>::openToParty(
    int partyId) const {
  static_assert(isSecret, "No need to open a public value.");
  Int<isSigned, width, false, schedulerId, usingBatch> rst;

  for (int8_t i = 0; i < width; i++) {
    rst.data_[i] = data_.at(i).openToParty(partyId);
  }
  return rst;
}

template <
    bool isSigned,
    int8_t width,
    bool isSecret,
    int schedulerId,
    bool usingBatch>
typename Int<isSigned, width, true, schedulerId, usingBatch>::ExtractedInt
Int<isSigned, width, isSecret, schedulerId, usingBatch>::extractIntShare()
    const {
  static_assert(isSecret, "No need to extract a public value.");
  ExtractedInt rst;
  for (int8_t i = 0; i < width; i++) {
    rst[i] = data_.at(i).extractBit();
  }
  return rst;
}

template <
    bool isSigned,
    int8_t width,
    bool isSecret,
    int schedulerId,
    bool usingBatch>
typename Int<isSigned, width, isSecret, schedulerId, usingBatch>::BoolType
Int<isSigned, width, isSecret, schedulerId, usingBatch>::extractLsb(
    const IntType& v,
    size_t t) {
  if constexpr (usingBatch) {
    std::vector<bool> rst(v.size());
    if (rst.size() == 0) {
      return rst;
    } else {
      for (size_t i = 0; i < rst.size(); i++) {
        rst[i] = (v.at(i) >> t) & 1;
      }
      return rst;
    }
  } else {
    return (v >> t) & 1;
  }
}

template <
    bool isSigned,
    int8_t width,
    bool isSecret,
    int schedulerId,
    bool usingBatch>
void Int<isSigned, width, isSecret, schedulerId, usingBatch>::shiftLeft(
    std::vector<uint64_t>& data) {
  for (auto& item : data) {
    item = item << 1;
  }
}

template <
    bool isSigned,
    int8_t width,
    bool isSecret,
    int schedulerId,
    bool usingBatch>
void Int<isSigned, width, isSecret, schedulerId, usingBatch>::addLsb(
    std::vector<uint64_t>& data,
    const std::vector<bool>& bits) {
  if (data.size() != bits.size()) {
    throw std::runtime_error("Inconsistent data and bits size! ");
  }
  if (data.size() == 0) {
    return;
  }
  for (size_t i = 0; i < data.size(); i++) {
    data[i] += bits.at(i);
  }
}

template <
    bool isSigned,
    int8_t width,
    bool isSecret,
    int schedulerId,
    bool usingBatch>
void Int<isSigned, width, isSecret, schedulerId, usingBatch>::processInput(
    IntType& v) const {
  if constexpr (usingBatch) {
    for (auto& item : v) {
      processSingleInput(item);
    }
  } else {
    processSingleInput(v);
  }
}

template <
    bool isSigned,
    int8_t width,
    bool isSecret,
    int schedulerId,
    bool usingBatch>
void Int<isSigned, width, isSecret, schedulerId, usingBatch>::
    processSingleInput(UnitIntType& v) const {
  if constexpr (isSigned) {
    if (v >= 0) {
      if ((v >> (width - 1)) != 0) {
        throw std::runtime_error(
            "Input value is too large! This is a signed integer of " +
            std::to_string(width) + " bits. The acceptable initial value is [" +
            std::to_string(-((kMask >> 1) + 1)) + ", " +
            std::to_string(kMask >> 1) + "]. But this attempt is using " +
            std::to_string(v) + ", Which is not acceptable.");
      }
    } else {
      // v can't be too small;
      // v is a signed value now. -((kMask >> 1) + 1) is the smallest width-bit
      // signed value. If v + ((kMask >> 1) + 1) <= 0, then v is smaller than
      // the smallest value a width-bit signed int can represent.
      if (v + UnitIntType(kMask >> 1) + 1 < 0) {
        throw std::runtime_error(
            "Input value is too small! This is a signed integer of " +
            std::to_string(width) + " bits. The acceptable initial value is [" +
            std::to_string(-((kMask >> 1) + 1)) + ", " +
            std::to_string(kMask >> 1) + "]. But this attempt is using " +
            std::to_string(v) + ", Which is not acceptable.");
      }
      v = v + kMask + 1;
    }
  } else {
    // can't use ((v >> width) != 0 because it won't compile when width = 64;
    // v is merely 64-bit wide;
    if ((v >> (width - 1)) > 1) {
      throw std::runtime_error(
          "Input value is too large! This is a unsigned integer of " +
          std::to_string(width) +
          " bits. The acceptable initial value is [0, " +
          std::to_string(kMask) + "]. But this attempt is using " +
          std::to_string(v) + ", Which is not acceptable.");
    }
  }
}

template <
    bool isSigned,
    int8_t width,
    bool isSecret,
    int schedulerId,
    bool usingBatch>
void Int<isSigned, width, isSecret, schedulerId, usingBatch>::
    convertPublicIntToBits(const IntType& v) {
  for (int8_t i = 0; i < width; i++) {
    data_[i] = Bit<false, schedulerId, usingBatch>(extractLsb(v, i));
  }
}

template <
    bool isSigned,
    int8_t width,
    bool isSecret,
    int schedulerId,
    bool usingBatch>
void Int<isSigned, width, isSecret, schedulerId, usingBatch>::
    convertPrivateIntToBits(const IntType& v, int partyId) {
  for (int8_t i = 0; i < width; i++) {
    data_[i] = Bit<true, schedulerId, usingBatch>(extractLsb(v, i), partyId);
  }
}

template <
    bool isSigned,
    int8_t width,
    bool isSecret,
    int schedulerId,
    bool usingBatch>
template <typename T>
std::vector<typename Int<isSigned, width, isSecret, schedulerId, usingBatch>::
                UnitIntType>
Int<isSigned, width, isSecret, schedulerId, usingBatch>::
    convertTo64BitIntVector(const std::vector<T>& src) const {
  static_assert(sizeof(T) * 8 >= width);
  std::vector<UnitIntType> rst(src.size());
  std::transform(src.begin(), src.end(), rst.begin(), [](T v) { return v; });
  return rst;
}

template <
    bool isSigned,
    int8_t width,
    bool isSecret,
    int schedulerId,
    bool usingBatch>
template <typename T>
typename Int<isSigned, width, isSecret, schedulerId, usingBatch>::IntType
Int<isSigned, width, isSecret, schedulerId, usingBatch>::convertBitsToInt(
    const std::array<T, width>& data) {
  if constexpr (usingBatch) {
    // processing the msb(s) and use the result as the starting point
    auto tmp = data.at(width - 1).getValue();
    std::vector<uint64_t> buffer(tmp.size(), 0);
    addLsb(buffer, tmp);

    // starting from processing data.at(width - 2) since data.at(width - 1)
    // has already been processed
    for (int8_t i = width - 1; i > 0; i--) {
      shiftLeft(buffer);
      addLsb(buffer, data.at(i - 1).getValue());
    }
    if constexpr (isSigned) {
      IntType rst(buffer.size());
      for (size_t i = 0; i < rst.size(); i++) {
        rst[i] = buffer.at(i);
        if (rst.at(i) > (kMask >> 1)) {
          rst[i] = rst.at(i) - kMask - 1;
        }
      }
      return rst;
    } else {
      return buffer;
    }
  } else {
    uint64_t rst = 0;
    for (int8_t i = width; i > 0; i--) {
      rst = rst << 1;
      rst += data.at(i - 1).getValue();
    }
    if constexpr (isSigned) {
      // rst is an unsigned value. We need to convert it to a signed value here.
      // kMask is the largest width-bit unsigned value, kMask >> 1 is the
      // largest width-bit signed value if rst is larger than that, we need to
      // convert it to the corresponding negative value.
      if (rst > (kMask >> 1)) {
        rst = rst - kMask - 1;
      }
    }
    return rst;
  }
}

} // namespace fbpcf::frontend
