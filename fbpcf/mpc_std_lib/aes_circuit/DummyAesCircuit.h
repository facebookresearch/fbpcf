/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/mpc_std_lib/aes_circuit/IAesCircuit.h"

namespace fbpcf::mpc_std_lib::aes_circuit::insecure {

/*
 * This is merely a placeholder that basically does nothing other than input
 * validation. This object is only meant to be used for test purpose. When
 * testing, the corresponding encryption/decryption process should also do
 * nothing to make the final result correct.
 */
template <typename BitType>
class DummyAesCircuit final : public IAesCircuit<BitType> {
 private:
  std::vector<BitType> encrypt_impl(
      const std::vector<BitType>& plaintext,
      const std::vector<BitType>& /* expandedEncKey */) const override {
    return plaintext;
  }

  /**
   * @inherit doc
   */
  std::vector<BitType> decrypt_impl(
      const std::vector<BitType>& ciphertext,
      const std::vector<BitType>& /* expandedDecKey */) const override {
    return ciphertext;
  }
};

} // namespace fbpcf::mpc_std_lib::aes_circuit::insecure
