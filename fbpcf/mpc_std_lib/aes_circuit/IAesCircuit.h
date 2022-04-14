/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstddef>
#include <stdexcept>
#include <vector>

namespace fbpcf::mpc_std_lib::aes_circuit {

/*
 * An AES circuit object implements the AES algorithm at a conceptual-bit level.
 * This "conceptual bit" can be anything that has an isomorphic behavior
 * regarding AND and XOR as normal bits. Currently we only support decryption.
 * We may add support for encryption in future.
 */
/**
 * Bit type can be either bool or any MPC Bit types.
 */
template <typename BitType>
class IAesCircuit {
 public:
  static const size_t kExpandedKeyWidth =
      1408; /** The expanded AES key contains 11 16-byte keys. 1408 = 11 * 16 *
               8 **/

  virtual ~IAesCircuit() = default;

  /**
   * Encrypt the plaintext with the expanded key.
   * @param plaintext the plaintext for AES inside MPC. It must be a
   * multiplication of 128.
   * @param expandedEncKey the expanded enc AES key inside MPC. It must be the
   * expected expanded key size.
   * @return the ciphertext inside MPC;
   */
  std::vector<BitType> encrypt(
      const std::vector<BitType>& plaintext,
      const std::vector<BitType>& expandedEncKey) const {
    if (plaintext.size() % 128 != 0) {
      throw std::runtime_error("Input plaintext must be a multiple of 128");
    }
    if (expandedEncKey.size() != kExpandedKeyWidth) {
      throw std::runtime_error("Expanded Enc AES key must be 1408 bits.");
    }
    return encrypt_impl(plaintext, expandedEncKey);
  }

  /**
   * decrypt the ciphertext with the expanded key.
   * @param ciphertext the ciphertext for AES inside MPC. It must be a
   * multiplication of 128.
   * @param expandedKey the expanded AES key inside MPC. It must be the
   * expected expanded key size.
   * @return the plaintext inside MPC;
   */
  std::vector<BitType> decrypt(
      const std::vector<BitType>& ciphertext,
      const std::vector<BitType>& expandedDecKey) const {
    if (ciphertext.size() % 128 != 0) {
      throw std::runtime_error("Input ciphertext must be a multiple of 128");
    }
    if (expandedDecKey.size() != kExpandedKeyWidth) {
      throw std::runtime_error("Expanded Dec AES key must be 1408 bits.");
    }
    return decrypt_impl(ciphertext, expandedDecKey);
  }

 private:
  virtual std::vector<BitType> encrypt_impl(
      const std::vector<BitType>& plaintext,
      const std::vector<BitType>& expandedEncKey) const = 0;

  virtual std::vector<BitType> decrypt_impl(
      const std::vector<BitType>& ciphertext,
      const std::vector<BitType>& expandedDecKey) const = 0;
};

} // namespace fbpcf::mpc_std_lib::aes_circuit
