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
 * functions added here are helpers to support shuffler and permuter for
 * AttributionData type. They are partial specializations for function
 * templates listed in mpc_std_lib/util/util.h file.
 */

/*
 * AttributionValue is composed of 64 bits unsinged integer type of ad id
 * and 32 bits unsinged integer type of conversion value.
 */
struct AttributionValue {
  uint64_t adId;
  uint32_t conversionValue;
  AttributionValue() {}
  AttributionValue(uint64_t id, uint32_t value)
      : adId{id}, conversionValue(value) {}
};

template <>
class Adapters<AttributionValue> {
 public:
  static const int8_t adIdWidth = 64; // 64 bits unsinged integer
  static const int8_t valueWidth = 32; // 32 bits unsigned integer
};

/*
 * SecretAttributionValue is a secret batch of AttributionValue type. Namely, it
 * is composed of secret batches of ad ids and conversion values, each with an
 * appropriate batched Int type. This type supports
 * unbatching and batchingWith functions, similar to those implemented for Bit,
 * BitString and Int types.
 */
template <int schedulerId>
struct SecretAttributionValue {
  SecretAttributionValue() = default;
  SecretAttributionValue(
      const frontend::Int<
          false,
          Adapters<AttributionValue>::adIdWidth,
          true,
          schedulerId,
          true>& id,
      const frontend::Int<
          false,
          Adapters<AttributionValue>::valueWidth,
          true,
          schedulerId,
          true>& value)
      : adId(id), conversionValue(value) {}
  SecretAttributionValue(
      const std::vector<AttributionValue>& src,
      int partyId) {
    std::vector<uint64_t> id(src.size());
    std::vector<uint32_t> value(src.size());
    std::transform(
        src.begin(), src.end(), id.begin(), [](const AttributionValue& src) {
          return src.adId;
        });
    std::transform(
        src.begin(), src.end(), value.begin(), [](const AttributionValue& src) {
          return src.conversionValue;
        });
    adId = frontend::Int<
        false,
        Adapters<AttributionValue>::adIdWidth,
        true,
        schedulerId,
        true>(id, partyId);
    conversionValue = frontend::Int<
        false,
        Adapters<AttributionValue>::valueWidth,
        true,
        schedulerId,
        true>(value, partyId);
  }
  std::vector<SecretAttributionValue<schedulerId>> unbatching(
      std::shared_ptr<std::vector<uint32_t>> unbatchingStrategy) const {
    auto unbatchedAdId = adId.unbatching(unbatchingStrategy);
    auto unbatchedConversionValue =
        conversionValue.unbatching(unbatchingStrategy);
    std::vector<SecretAttributionValue<schedulerId>> rst(
        unbatchingStrategy->size());
    for (size_t i = 0; i < unbatchingStrategy->size(); i++) {
      rst[i].adId = unbatchedAdId.at(i);
      rst[i].conversionValue = unbatchedConversionValue.at(i);
    }

    return rst;
  }
  SecretAttributionValue<schedulerId> batchingWith(
      const std::vector<SecretAttributionValue<schedulerId>>& others) const {
    std::vector<frontend::Int<
        false,
        Adapters<AttributionValue>::adIdWidth,
        true,
        schedulerId,
        true>>
        otherIds(others.size());
    std::vector<frontend::Int<
        false,
        Adapters<AttributionValue>::valueWidth,
        true,
        schedulerId,
        true>>
        otherValues(others.size());
    std::transform(
        others.begin(),
        others.end(),
        otherIds.begin(),
        [](const SecretAttributionValue<schedulerId>& other) {
          return other.adId;
        });
    std::transform(
        others.begin(),
        others.end(),
        otherValues.begin(),
        [](const SecretAttributionValue<schedulerId>& other) {
          return other.conversionValue;
        });
    SecretAttributionValue<schedulerId> rst(
        adId.batchingWith(otherIds), conversionValue.batchingWith(otherValues));
    return rst;
  }
  frontend::
      Int<false, Adapters<AttributionValue>::adIdWidth, true, schedulerId, true>
          adId;
  frontend::Int<
      false,
      Adapters<AttributionValue>::valueWidth,
      true,
      schedulerId,
      true>
      conversionValue;
};

/*
 * AttributionData is composed of AsstributionValue type value and boolean type
 * label.
 */
struct AttributionData {
  AttributionValue value;
  bool label;
  AttributionData() {}
  AttributionData(AttributionValue value, bool label)
      : value{std::move(value)}, label{label} {}
};

/*
 * SecretAttributionData is a secret batch of AttributionData type. Namely, it
 * is composed of secret batches of AttributionValue type values and boolean
 * type labels This type supports unbatching and batchingWith functions, similar
 * to those implemented for Bit, BitString and Int types.
 */
template <int schedulerId>
struct SecretAttributionData {
  SecretAttributionData() = default;
  SecretAttributionData(
      const SecretAttributionValue<schedulerId>& value,
      const frontend::Bit<true, schedulerId, true>& label)
      : value{std::move(value)}, label{std::move(label)} {}
  SecretAttributionData(const std::vector<AttributionData>& src, int partyId) {
    std::vector<AttributionValue> attValue(src.size());
    std::vector<bool> attLabel(src.size());
    for (size_t i = 0; i < src.size(); i++) {
      attValue[i] = src[i].value;
      attLabel[i] = src[i].label;
    }
    value = SecretAttributionValue<schedulerId>(attValue, partyId);
    label = frontend::Bit<true, schedulerId, true>(attLabel, partyId);
  }

  std::vector<SecretAttributionData> unbatching(
      std::shared_ptr<std::vector<uint32_t>> unbatchingStrategy) const {
    auto unbatchedValue = value.unbatching(unbatchingStrategy);
    auto unbatchedLabel = label.unbatching(unbatchingStrategy);

    std::vector<SecretAttributionData<schedulerId>> rst(
        unbatchingStrategy->size());
    for (size_t i = 0; i < unbatchingStrategy->size(); i++) {
      rst[i] = SecretAttributionData<schedulerId>(
          unbatchedValue.at(i), unbatchedLabel.at(i));
    }

    return rst;
  }
  SecretAttributionData<schedulerId> batchingWith(
      const std::vector<SecretAttributionData<schedulerId>>& others) const {
    std::vector<SecretAttributionValue<schedulerId>> otherValues(others.size());
    std::vector<frontend::Bit<true, schedulerId, true>> otherLabels(
        others.size());
    for (size_t i = 0; i < others.size(); i++) {
      otherValues[i] = others.at(i).value;
      otherLabels[i] = others.at(i).label;
    }
    SecretAttributionData<schedulerId> rst(
        value.batchingWith(otherValues), label.batchingWith(otherLabels));

    return rst;
  }
  SecretAttributionValue<schedulerId> value;
  frontend::Bit<true, schedulerId, true> label;
};

template <int schedulerId>
struct SecBatchType<AttributionData, schedulerId> {
  using type = SecretAttributionData<schedulerId>;
};

/*
 * Helpers for AttributionData type in MPC.
 */
template <int schedulerId>
class MpcAdapters<AttributionData, schedulerId> {
 public:
  using SecBatchType =
      typename SecBatchType<AttributionData, schedulerId>::type;
  static SecBatchType processSecretInputs(
      const std::vector<AttributionData>& secrets,
      int secretOwnerPartyId) {
    return SecretAttributionData<schedulerId>(secrets, secretOwnerPartyId);
  }
  static std::pair<SecBatchType, SecBatchType> obliviousSwap(
      const SecBatchType& src1,
      const SecBatchType& src2,
      frontend::Bit<true, schedulerId, true> indicator) {
    auto [rstLabel1, rstLabel2] = MpcAdapters<bool, schedulerId>::obliviousSwap(
        src1.label, src2.label, indicator);

    auto [rstId1, rstId2] =
        MpcAdapters<Intp<false, 64>, schedulerId>::obliviousSwap(
            src1.value.adId, src2.value.adId, indicator);
    auto [rstConversionValue1, rstConversionValue2] =
        MpcAdapters<uint32_t, schedulerId>::obliviousSwap(
            src1.value.conversionValue, src2.value.conversionValue, indicator);

    auto rst1 = SecretAttributionData<schedulerId>(
        SecretAttributionValue<schedulerId>(rstId1, rstConversionValue1),
        rstLabel1);
    auto rst2 = SecretAttributionData<schedulerId>(
        SecretAttributionValue<schedulerId>(rstId2, rstConversionValue2),
        rstLabel2);

    return {rst1, rst2};
  }
  static std::vector<AttributionData> openToParty(
      const SecBatchType& src,
      int partyId) {
    auto id = src.value.adId.openToParty(partyId).getValue();
    auto value = src.value.conversionValue.openToParty(partyId).getValue();
    auto label = src.label.openToParty(partyId).getValue();

    if (label.size() != id.size() || label.size() != value.size()) {
      throw std::runtime_error("Unexpected size.");
    }

    std::vector<AttributionData> rst(label.size());
    for (size_t i = 0; i < rst.size(); i++) {
      rst[i] =
          AttributionData(AttributionValue(id.at(i), value.at(i)), label.at(i));
    }
    return rst;
  }
};

} // namespace fbpcf::mpc_std_lib::util
