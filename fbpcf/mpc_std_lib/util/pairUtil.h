/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

namespace fbpcf::mpc_std_lib::util {

/*
 * The adapters here are partial template specializations for function
 * templates listed in mpc_std_lib/util/util.h file to support a pair template
 * type std::pair<T, U>.
 */

template <typename T, typename U, int schedulerId>
struct SecBatchType<std::pair<T, U>, schedulerId> {
  using type = std::pair<
      typename SecBatchType<T, schedulerId>::type,
      typename SecBatchType<U, schedulerId>::type>;
};

template <typename T, typename U, int schedulerId>
class MpcAdapters<std::pair<T, U>, schedulerId> {
 public:
  using SecBatchType =
      typename SecBatchType<std::pair<T, U>, schedulerId>::type;
  static SecBatchType processSecretInputs(
      const std::vector<std::pair<T, U>>& secrets,
      int secretOwnerPartyId);

  static std::pair<SecBatchType, SecBatchType> obliviousSwap(
      const SecBatchType& src1,
      const SecBatchType& src2,
      frontend::Bit<true, schedulerId, true> indicator);

  static std::vector<std::pair<T, U>> openToParty(
      const SecBatchType& src,
      int partyId);

  static SecBatchType batchingWith(
      const SecBatchType& src,
      const std::vector<SecBatchType>& others);

  static std::vector<SecBatchType> unbatching(
      const SecBatchType& src,
      std::shared_ptr<std::vector<uint32_t>> unbatchingStrategy);
};

} // namespace fbpcf::mpc_std_lib::util

#include "fbpcf/mpc_std_lib/util/pair_impl.h"
