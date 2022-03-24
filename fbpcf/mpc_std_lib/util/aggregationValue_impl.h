/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emmintrin.h>
#include <smmintrin.h>
#include <functional>
#include <random>
#include <stdexcept>

namespace fbpcf::mpc_std_lib::util {

/**
 * functions added here are helpers to support a write-only ORAM for
 * AggregationValue type. They are partial specializations for function
 * templates listed in mpc_std_lib/util/util.h file.
 */

struct AggregationValue {
  uint32_t conversionCount;
  uint32_t conversionValue;

  AggregationValue() {}

  AggregationValue(uint32_t count, uint32_t value)
      : conversionCount{count}, conversionValue{value} {}

  explicit AggregationValue(uint32_t value)
      : conversionCount{value > 0}, conversionValue{value} {}
};

inline AggregationValue operator+(
    const AggregationValue& v1,
    const AggregationValue& v2) {
  return AggregationValue{
      v1.conversionCount + v2.conversionCount,
      v1.conversionValue + v2.conversionValue};
}

inline AggregationValue operator-(
    const AggregationValue& v1,
    const AggregationValue& v2) {
  return AggregationValue{
      v1.conversionCount - v2.conversionCount,
      v1.conversionValue - v2.conversionValue};
}

inline AggregationValue operator-(const AggregationValue& v) {
  return AggregationValue{-v.conversionCount, -v.conversionValue};
}

inline void operator+=(AggregationValue& v1, const AggregationValue& v2) {
  v1.conversionCount += v2.conversionCount;
  v1.conversionValue += v2.conversionValue;
}

inline bool operator==(const AggregationValue& v1, const AggregationValue& v2) {
  return v1.conversionCount == v2.conversionCount &&
      v1.conversionValue == v2.conversionValue;
}

template <>
class Adapters<AggregationValue> {
 public:
  static const int8_t countWidth = Adapters<uint32_t>::widthForUint32;
  static const int8_t valueWidth = Adapters<uint32_t>::widthForUint32;

  static AggregationValue convertFromBits(const std::vector<bool>& bits) {
    if (bits.size() != countWidth + valueWidth) {
      throw std::invalid_argument("unexpected input size");
    }
    AggregationValue rst;
    rst.conversionCount = Adapters<uint32_t>::convertFromBits(
        std::vector<bool>(bits.begin(), bits.begin() + countWidth));
    rst.conversionValue = Adapters<uint32_t>::convertFromBits(
        std::vector<bool>(bits.begin() + countWidth, bits.end()));
    return rst;
  }

  static std::vector<bool> convertToBits(const AggregationValue& src) {
    auto count = Adapters<uint32_t>::convertToBits(src.conversionCount);
    auto value = Adapters<uint32_t>::convertToBits(src.conversionValue);
    count.insert(count.end(), value.begin(), value.end());
    return count;
  }

  static AggregationValue generateFromKey(__m128i key) {
    return AggregationValue(
        _mm_extract_epi32(key, 1), _mm_extract_epi32(key, 0));
  }
};

template <int schedulerId>
struct SecretAggregationValue {
  SecretAggregationValue() = default;

  SecretAggregationValue(
      const std::vector<AggregationValue>& src,
      int partyId) {
    std::vector<uint32_t> count(src.size());
    std::vector<uint32_t> value(src.size());
    std::transform(
        src.begin(), src.end(), count.begin(), [](const AggregationValue& src) {
          return src.conversionCount;
        });
    std::transform(
        src.begin(), src.end(), value.begin(), [](const AggregationValue& src) {
          return src.conversionValue;
        });
    conversionCount = frontend::Int<
        false,
        Adapters<AggregationValue>::countWidth,
        true,
        schedulerId,
        true>(count, partyId);
    conversionValue = frontend::Int<
        false,
        Adapters<AggregationValue>::valueWidth,
        true,
        schedulerId,
        true>(value, partyId);
  }

  SecretAggregationValue<schedulerId> operator-(
      const SecretAggregationValue<schedulerId>& subtrahend) {
    SecretAggregationValue<schedulerId> rst;
    rst.conversionCount = conversionCount - subtrahend.conversionCount;
    rst.conversionValue = conversionValue - subtrahend.conversionValue;
    return rst;
  }

  SecretAggregationValue<schedulerId> mux(
      const frontend::Bit<true, schedulerId, true>& choice,
      const SecretAggregationValue<schedulerId>& src) {
    SecretAggregationValue<schedulerId> rst;
    rst.conversionCount = conversionCount.mux(choice, src.conversionCount);
    rst.conversionValue = conversionValue.mux(choice, src.conversionValue);
    return rst;
  }

  frontend::Int<
      false,
      Adapters<AggregationValue>::countWidth,
      true,
      schedulerId,
      true>
      conversionCount;
  frontend::Int<
      false,
      Adapters<AggregationValue>::valueWidth,
      true,
      schedulerId,
      true>
      conversionValue;
};

template <int schedulerId>
struct SecBatchType<AggregationValue, schedulerId> {
  using type = SecretAggregationValue<schedulerId>;
};

template <int schedulerId>
class MpcAdapters<AggregationValue, schedulerId> {
 public:
  using SecBatchType =
      typename SecBatchType<AggregationValue, schedulerId>::type;
  static SecBatchType processSecretInputs(
      const std::vector<AggregationValue>& secrets,
      int secretOwnerPartyId) {
    return SecretAggregationValue<schedulerId>(secrets, secretOwnerPartyId);
  }

  static SecBatchType recoverBatchSharedSecrets(
      const std::vector<std::vector<bool>>& src) {
    typename frontend::Int<
        false,
        Adapters<AggregationValue>::countWidth,
        true,
        schedulerId,
        true>::ExtractedInt extractedCount;
    typename frontend::Int<
        false,
        Adapters<AggregationValue>::valueWidth,
        true,
        schedulerId,
        true>::ExtractedInt extractedValue;

    for (size_t i = 0; i < Adapters<AggregationValue>::countWidth; i++) {
      extractedCount[i] =
          typename frontend::Bit<true, schedulerId, true>::ExtractedBit(
              src.at(i));
    }
    for (size_t i = 0; i < Adapters<AggregationValue>::valueWidth; i++) {
      extractedValue[i] =
          typename frontend::Bit<true, schedulerId, true>::ExtractedBit(
              src.at(i + Adapters<AggregationValue>::countWidth));
    }
    SecretAggregationValue<schedulerId> rst;
    rst.conversionCount = frontend::Int<
        false,
        Adapters<AggregationValue>::countWidth,
        true,
        schedulerId,
        true>(std::move(extractedCount));
    rst.conversionValue = frontend::Int<
        false,
        Adapters<AggregationValue>::valueWidth,
        true,
        schedulerId,
        true>(std::move(extractedValue));
    return rst;
  }

  static std::pair<SecBatchType, SecBatchType> obliviousSwap(
      const SecBatchType& src1,
      const SecBatchType& src2,
      frontend::Bit<true, schedulerId, true> indicator) {
    SecretAggregationValue<schedulerId> rst1;
    SecretAggregationValue<schedulerId> rst2;
    rst1.conversionCount =
        src1.conversionCount.mux(indicator, src2.conversionCount);
    rst1.conversionValue =
        src1.conversionValue.mux(indicator, src2.conversionValue);

    for (size_t i = 0; i < Adapters<AggregationValue>::countWidth; i++) {
      rst2.conversionCount[i] = rst1.conversionCount[i] ^
          src1.conversionCount[i] ^ src2.conversionCount[i];
    }
    for (size_t i = 0; i < Adapters<AggregationValue>::valueWidth; i++) {
      rst2.conversionValue[i] = rst1.conversionValue[i] ^
          src1.conversionValue[i] ^ src2.conversionValue[i];
    }
    return {rst1, rst2};
  }

  static std::vector<AggregationValue> openToParty(
      const SecBatchType& src,
      int partyId) {
    // concentrate "openToParty" calls as much as possible to avoid roundtrips.
    auto openedCount = src.conversionCount.openToParty(partyId);
    auto openedValue = src.conversionValue.openToParty(partyId);

    auto counts = openedCount.getValue();
    auto values = openedValue.getValue();

    if (counts.size() != values.size()) {
      throw std::runtime_error("Unexpected size.");
    }

    std::vector<AggregationValue> rst(counts.size());
    for (size_t i = 0; i < rst.size(); i++) {
      rst[i].conversionCount = counts.at(i);
      rst[i].conversionValue = values.at(i);
    }
    return rst;
  }
};

} // namespace fbpcf::mpc_std_lib::util
