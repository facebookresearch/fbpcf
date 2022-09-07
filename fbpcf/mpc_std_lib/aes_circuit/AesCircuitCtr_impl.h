/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <vector>
#include "fbpcf/mpc_std_lib/aes_circuit/AesCircuitCtr.h"

namespace fbpcf::mpc_std_lib::aes_circuit {

template <typename BitType>
std::vector<BitType> AesCircuitCtr<BitType>::encrypt_impl(
    const std::vector<BitType>& plaintext,
    const std::vector<BitType>& expandedEncKey,
    const std::vector<BitType>& mask) const {
  size_t blockNo = plaintext.size() / 128;
  std::vector<BitType> encryptedMask =
      AesCircuit_->encrypt(mask, expandedEncKey);
  std::vector<BitType> rst;
  rst.reserve(blockNo * 128);
  for (int i = 0; i < blockNo; ++i) {
    for (int j = 0; j < 128; ++j) {
      rst.push_back(plaintext[i * 128 + j] ^ encryptedMask[i * 128 + j]);
    }
  }
  return rst;
}
} // namespace fbpcf::mpc_std_lib::aes_circuit
