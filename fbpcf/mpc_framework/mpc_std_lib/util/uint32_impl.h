/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <smmintrin.h>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace fbpcf::mpc_std_lib::util {

template <>
class Adapters<uint32_t> {
 public:
  /* how many bits in uint32_t*/
  static const int8_t widthForUint32 = 32;

  static uint32_t convertFromBits(const std::vector<bool>& bits) {
    uint32_t rst = 0;
    if (bits.size() > widthForUint32) {
      throw std::invalid_argument("Too many input bits!");
    }
    for (size_t i = bits.size(); i > 0; i--) {
      rst <<= 1;
      rst += bits.at(i - 1);
    }

    return rst;
  }

  static std::vector<bool> convertToBits(const uint32_t& src) {
    std::vector<bool> rst(widthForUint32, 0);
    uint32_t tmp = src;
    for (size_t t = 0; tmp > 0; t++) {
      rst[t] = tmp & 1;
      tmp >>= 1;
    }
    return rst;
  }

  static uint32_t generateFromKey(__m128i key) {
    return _mm_extract_epi32(key, 0);
  }
};

template <int schedulerId>
struct SecBatchType<uint32_t, schedulerId> {
  using type = frontend::
      Int<false, Adapters<uint32_t>::widthForUint32, true, schedulerId, true>;
};

template <int schedulerId>
class MpcAdapters<uint32_t, schedulerId> {
  static const int8_t widthForUint32 = Adapters<uint32_t>::widthForUint32;

 public:
  using SecBatchType = typename SecBatchType<uint32_t, schedulerId>::type;
  static SecBatchType processSecretInputs(
      const std::vector<uint32_t>& secrets,
      int secretOwnerPartyId);

  static SecBatchType recoverBatchSharedSecrets(
      const std::vector<std::vector<bool>>& src);

  static std::pair<SecBatchType, SecBatchType> obliviousSwap(
      const SecBatchType& src1,
      const SecBatchType& src2,
      frontend::Bit<true, schedulerId, true> indicator);

  static std::vector<uint32_t> openToParty(
      const SecBatchType& src,
      int partyId) {
    auto buf = src.openToParty(partyId).getValue();
    std::vector<uint32_t> rst(buf.size());
    std::transform(
        buf.begin(), buf.end(), rst.begin(), [](auto v) { return v; });
    return rst;
  }
};

template <int schedulerId>
typename MpcAdapters<uint32_t, schedulerId>::SecBatchType
MpcAdapters<uint32_t, schedulerId>::processSecretInputs(
    const std::vector<uint32_t>& secrets,
    int secretOwnerPartyId) {
  return frontend::Int<false, widthForUint32, true, schedulerId, true>(
      secrets, secretOwnerPartyId);
}

template <int schedulerId>
typename MpcAdapters<uint32_t, schedulerId>::SecBatchType
MpcAdapters<uint32_t, schedulerId>::recoverBatchSharedSecrets(
    const std::vector<std::vector<bool>>& src) {
  typename frontend::Int<false, widthForUint32, true, schedulerId, true>::
      ExtractedInt rst;
  for (size_t i = 0; i < widthForUint32; i++) {
    rst[i] = typename frontend::Bit<true, schedulerId, true>::ExtractedBit(
        src.at(i));
  }
  return frontend::Int<false, widthForUint32, true, schedulerId, true>(
      std::move(rst));
}

template <int schedulerId>
std::pair<
    typename MpcAdapters<uint32_t, schedulerId>::SecBatchType,
    typename MpcAdapters<uint32_t, schedulerId>::SecBatchType>
MpcAdapters<uint32_t, schedulerId>::obliviousSwap(
    const SecBatchType& src1,
    const SecBatchType& src2,
    frontend::Bit<true, schedulerId, true> indicator) {
  auto rst1 = src1.mux(indicator, src2);
  auto rst2 = rst1;
  for (size_t i = 0; i < widthForUint32; i++) {
    rst2[i] = rst2[i] ^ src1[i] ^ src2[i];
  }
  return {rst1, rst2};
}

} // namespace fbpcf::mpc_std_lib::util
