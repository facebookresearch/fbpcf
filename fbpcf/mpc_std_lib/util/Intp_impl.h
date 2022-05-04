/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <climits>
#include <cmath>
#include <cstdint>
#include <exception>
#include <type_traits>
#include <vector>

namespace fbpcf::mpc_std_lib::util {

/**
 * This is the type for plaintext integers.
 */
template <bool isSigned, int8_t width>
class Intp {
  template <
      typename T8,
      typename T16,
      typename T32,
      typename T64,
      int8_t intWidth>
  using typeSelector = typename std::conditional<
      intWidth <= 16,
      typename std::conditional<intWidth <= 8, T8, T16>::type,
      typename std::conditional<intWidth <= 32, T32, T64>::type>::type;

 public:
  using NativeType = typename std::conditional<
      isSigned,
      typeSelector<int8_t, int16_t, int32_t, int64_t, width>,
      typeSelector<uint8_t, uint16_t, uint32_t, uint64_t, width>>::type;

  Intp() : v_(0) {}

  // intentionally allow implicit conversion here
  /* implicit */ Intp(NativeType v) : v_(validateValue(v)) {}

  operator NativeType() const {
    return v_;
  }

  operator NativeType&() {
    return v_;
  }

  explicit operator NativeType*() const {
    return &v_;
  }

  Intp<isSigned, width> operator+(const Intp<isSigned, width>& other) const {
    return Intp<isSigned, width>(add(v_, other.v_));
  }

  Intp<isSigned, width> operator-() const {
    if constexpr (isSigned) {
      if (v_ == kMin) {
        return v_;
      } else {
        return -v_;
      }
    } else {
      return kOffSet - v_;
    }
  }

  Intp<isSigned, width> operator-(const Intp<isSigned, width>& other) const {
    return Intp<isSigned, width>(subtract(v_, other.v_));
  }

 public:
  static constexpr NativeType kMax = isSigned ? LLONG_MAX >> (64 - width)
                                              : ULLONG_MAX >> (64 - width);
  static constexpr NativeType kMin = isSigned ? LLONG_MIN >> (64 - width) : 0;

  // this is the wrap-around offset for integers. For 64bit ints,
  // wrap-around is automatic. Otherwise it needs to be done manually. This
  // wrap-around is 2^width, which happen to be (ULLONG_MAX >> (64 - width))
  // + 1.
  static constexpr NativeType kOffSet =
      width == 64 ? 0 : (ULLONG_MAX >> (64 - width)) + 1;

  inline static NativeType round(NativeType v) {
    if (v > kMax) {
      return v - kOffSet;
    } else if constexpr (isSigned) {
      if (v < kMin) {
        return v + kOffSet;
      }
    }
    return v;
  }

 private:
  static NativeType validateValue(NativeType v) {
    if (v > kMax) {
      throw std::runtime_error(
          "Value is too large. The maximum of " + std::to_string(width) +
          " bit " + (isSigned ? " signed " : "unsigned ") + "integer is " +
          std::to_string(kMax) + ". But you provided " + std::to_string(v) +
          ". ");
    } else if (v < kMin) {
      throw std::runtime_error(
          "Value is too small. The minimum of " + std::to_string(width) +
          " bit signed integer is " + std::to_string(kMin) +
          ". But you provided " + std::to_string(v) + ". ");
    } else {
      return v;
    }
  }

  static NativeType add(NativeType a, NativeType b) {
    if constexpr (
        isSigned &&
        ((width == 8) || (width == 16) || (width == 32) || (width == 64))) {
      // special handling is needed only for signed integer with some special
      // width (e.g. overflow is possible).
      if (std::signbit(a) == std::signbit(b)) {
        // the two numbers have the same sign, overflow is possible.
        // special treatment to prevent overflow
        return round(uint64_t(a) + uint64_t(b));
      } else {
        return round(a + b);
      }
    } else {
      return round(a + b);
    }
  }

  static NativeType subtract(NativeType a, NativeType b) {
    if constexpr (
        isSigned &&
        ((width == 8) || (width == 16) || (width == 32) || (width == 64))) {
      // special handling is needed only for signed integer with some special
      // width (e.g. overflow is possible).
      if (std::signbit(a) != std::signbit(b)) {
        // the two numbers have different sign, overflow is possible.
        // special treatment to prevent overflow
        return round(uint64_t(a) - uint64_t(b));
      } else {
        return round(a - b);
      }
    } else {
      return round(a - b);
    }
  }

  NativeType v_;
};

template <bool isSigned, int8_t width>
class Adapters<Intp<isSigned, width>> {
 public:
  static Intp<isSigned, width> convertFromBits(const std::vector<bool>& bits) {
    uint64_t rst = 0;
    if (bits.size() > width) {
      throw std::invalid_argument("Too many input bits!");
    }
    for (size_t i = bits.size(); i > 0; i--) {
      rst <<= 1;
      rst += bits.at(i - 1);
    }

    return Intp<isSigned, width>::round(rst);
  }

  static std::vector<bool> convertToBits(const Intp<isSigned, width>& src) {
    std::vector<bool> rst(width, 0);
    uint64_t tmp = src;
    for (size_t t = 0; tmp > 0; t++) {
      rst[t] = tmp & 1;
      tmp >>= 1;
    }
    return rst;
  }

  // when the size of T is <=128, this function can simply take a subset of the
  // key to construct T; otherwise, this function needs to run a PRG with key as
  // seed to generate sufficient amount of random data to construct T;
  static Intp<isSigned, width> generateFromKey(__m128i key) {
    return Intp<isSigned, width>::round(
        _mm_extract_epi64(key, 0) & (ULLONG_MAX >> (64 - width)));
  }
};

// these helpers are for MPC part

template <bool isSigned, int8_t width, int schedulerId>
struct SecBatchType<Intp<isSigned, width>, schedulerId> {
  using type = frontend::Int<isSigned, width, true, schedulerId, true>;
};

template <bool isSigned, int8_t width, int schedulerId>
class MpcAdapters<Intp<isSigned, width>, schedulerId> {
 public:
  using SecBatchType =
      typename SecBatchType<Intp<isSigned, width>, schedulerId>::type;
  static SecBatchType processSecretInputs(
      const std::vector<Intp<isSigned, width>>& secrets,
      int secretOwnerPartyId) {
    std::vector<typename Intp<isSigned, width>::NativeType> tmp(secrets.size());
    std::transform(
        secrets.begin(), secrets.end(), tmp.begin(), [](auto v) { return v; });
    return SecBatchType(std::move(tmp), secretOwnerPartyId);
  }

  static SecBatchType recoverBatchSharedSecrets(
      const std::vector<std::vector<bool>>& src) {
    typename SecBatchType::ExtractedInt rst;
    for (size_t i = 0; i < width; i++) {
      rst[i] = typename frontend::Bit<true, schedulerId, true>::ExtractedBit(
          src.at(i));
    }
    return SecBatchType(std::move(rst));
  }

  static std::pair<SecBatchType, SecBatchType> obliviousSwap(
      const SecBatchType& src1,
      const SecBatchType& src2,
      frontend::Bit<true, schedulerId, true> indicator) {
    auto rst1 = src1.mux(indicator, src2);
    auto rst2 = rst1;
    for (size_t i = 0; i < width; i++) {
      rst2[i] = rst2[i] ^ src1[i] ^ src2[i];
    }
    return {rst1, rst2};
  }

  static std::vector<Intp<isSigned, width>> openToParty(
      const SecBatchType& src,
      int partyId) {
    auto buf = src.openToParty(partyId).getValue();
    std::vector<Intp<isSigned, width>> rst(buf.size());
    std::transform(
        buf.begin(), buf.end(), rst.begin(), [](auto v) { return v; });
    return rst;
  }
};

} // namespace fbpcf::mpc_std_lib::util
