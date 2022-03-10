/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emmintrin.h>

#include <vector>
#include "fbpcf/mpc_framework/frontend/Bit.h"
#include "fbpcf/mpc_framework/frontend/Int.h"

namespace fbpcf::mpc_std_lib::util {

/*
 * these helpers are for non-MPC part
 * Note that this type T must be isomorphic to an additive group where + and -
 * are always well-defined.
 */
template <typename T>
class Adapters {
 public:
  static T convertFromBits(const std::vector<bool>& bits);

  static std::vector<bool> convertToBits(const T& src);

  // when the size of T is <=128, this function can simply take a subset of the
  // key to construct T; otherwise, this function needs to run a PRG with key as
  // seed to generate sufficient amount of random data to construct T;
  static T generateFromKey(__m128i key);
};

// these helpers are for MPC part

template <typename T, int schedulerId>
struct SecBatchType;

template <typename T, int schedulerId>
class MpcAdapters {
 public:
  using SecBatchType = typename SecBatchType<T, schedulerId>::type;
  static SecBatchType processSecretInputs(
      const std::vector<T>& secrets,
      int secretOwnerPartyId);

  static SecBatchType recoverBatchSharedSecrets(
      const std::vector<std::vector<bool>>& src);

  static std::pair<SecBatchType, SecBatchType> obliviousSwap(
      const SecBatchType& src1,
      const SecBatchType& src2,
      frontend::Bit<true, schedulerId, true> indicator);

  static std::vector<T> openToParty(const SecBatchType& src, int partyId);
};

std::vector<std::vector<bool>> convertToBits(const std::vector<__m128i>& src);

std::vector<__m128i> convertFromBits(const std::vector<std::vector<bool>>& src);

} // namespace fbpcf::mpc_std_lib::util

#include "fbpcf/mpc_framework/mpc_std_lib/util/uint32_impl.h"

#include "fbpcf/mpc_framework/mpc_std_lib/util/aggregationValue_impl.h"
