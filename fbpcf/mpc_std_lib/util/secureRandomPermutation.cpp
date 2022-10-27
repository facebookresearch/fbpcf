/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/mpc_std_lib/util/secureRandomPermutation.h"

namespace fbpcf::mpc_std_lib::util {
std::vector<uint32_t> secureRandomPermutation(
    uint32_t size,
    engine::util::IPrg& prg) {
  std::vector<uint32_t> rst(size);
  for (size_t i = 0; i < size; i++) {
    rst[i] = i;
  }

  BN_CTX* ctx = BN_CTX_new();
  if (ctx == nullptr) {
    throw std::runtime_error(
        "BN_CTX initialization failed: " + std::to_string(ERR_get_error()));
  }

  for (size_t i = size; i > 1; i--) {
    // Max permutation size + 128 bit security parameter
    auto randomBytes = prg.getRandomBytes(
        sizeof(uint32_t) + RANDOM_PERMUTATION_SECURITY_BITS / 8);
    auto position = engine::util::mod(randomBytes, i, ctx);
    std::swap(rst[position], rst[i - 1]);
  }
  BN_CTX_free(ctx);
  return rst;
}
} // namespace fbpcf::mpc_std_lib::util
