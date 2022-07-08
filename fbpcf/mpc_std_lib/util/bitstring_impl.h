/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/frontend/BitString.h"

namespace fbpcf::mpc_std_lib::util {

template <int schedulerId>
struct SecBatchType<std::vector<bool>, schedulerId> {
  using type = frontend::BitString<true, schedulerId, true>;
};

template <int schedulerId>
class MpcAdapters<std::vector<bool>, schedulerId> {
 public:
  using SecBatchType =
      typename SecBatchType<std::vector<bool>, schedulerId>::type;
  static SecBatchType processSecretInputs(
      const std::vector<std::vector<bool>>& secrets,
      int secretOwnerPartyId) {
    return SecBatchType(secrets, secretOwnerPartyId);
  }

  static SecBatchType recoverBatchSharedSecrets(
      const std::vector<std::vector<bool>>& src);

  static std::pair<SecBatchType, SecBatchType> obliviousSwap(
      const SecBatchType& src1,
      const SecBatchType& src2,
      frontend::Bit<true, schedulerId, true> indicator) {
    auto rst1 = src1.mux(indicator, src2);
    SecBatchType rst2(rst1.size());
    for (size_t i = 0; i < rst2.size(); i++) {
      rst2[i] = rst1.at(i) ^ src1.at(i) ^ src2.at(i);
    }
    return {rst1, rst2};
  }

  static std::vector<std::vector<bool>> openToParty(
      const SecBatchType& src,
      int partyId) {
    return src.openToParty(partyId).getValue();
  }
  static SecBatchType batchingWith(
      const SecBatchType& src,
      const std::vector<SecBatchType>& others) {
    return src.batchingWith(others);
  }

  static std::vector<SecBatchType> unbatching(
      const SecBatchType& src,
      std::shared_ptr<std::vector<uint32_t>> unbatchingStrategy) {
    return src.unbatching(unbatchingStrategy);
  }
};

} // namespace fbpcf::mpc_std_lib::util
