/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <array>
#include "fbpcf/mpc_std_lib/aes_circuit/IAesCircuit.h"

namespace fbpcf::mpc_std_lib::aes_circuit {

/*
 * This is the implementation of AES circuit
 */
template <typename BitType>
class AesCircuit : public IAesCircuit<BitType> {
 private:
  std::vector<BitType> encrypt_impl(
      const std::vector<BitType>& plaintext,
      const std::vector<BitType>& expandedEncKey) const override;
  void sharedSBoxInPlace(
      const std::array<BitType, 28>& T,
      const BitType& D,
      std::array<BitType, 64>& M) const;

  using ByteType = std::array<BitType, 8>;
  using WordType = std::array<ByteType, 4>;
  /**
   * @inherit doc
   */
  std::vector<BitType> decrypt_impl(
      const std::vector<BitType>& ciphertext,
      const std::vector<BitType>& expandedDecKey) const override;

 protected:
  std::vector<std::array<WordType, 4>> convertToWords(
      const std::vector<BitType>& src) const;

  std::vector<BitType> convertFromWords(
      std::vector<std::array<WordType, 4>>& src) const;

  void sBoxInPlace(ByteType& src) const;
  void inverseSBoxInPlace(ByteType& src) const;

  void mixColumnsInPlace(WordType& src) const;
  void inverseMixColumnsInPlace(WordType& src) const;

  void shiftRowInPlace(std::array<WordType, 4>& src) const;

#ifdef AES_CIRCUIT_TEST_FRIENDS
  AES_CIRCUIT_TEST_FRIENDS;
#endif
};

} // namespace fbpcf::mpc_std_lib::aes_circuit

#include "fbpcf/mpc_std_lib/aes_circuit/AesCircuit_impl.h"
