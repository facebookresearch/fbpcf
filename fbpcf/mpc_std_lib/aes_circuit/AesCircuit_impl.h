/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <vector>
namespace fbpcf::mpc_std_lib::aes_circuit {

template <typename BitType>
std::vector<BitType> AesCircuit<BitType>::encrypt_impl(
    const std::vector<BitType>& plaintext,
    const std::vector<BitType>& expandedEncKey) const {
  throw std::runtime_error("Not implemented!");
}

template <typename BitType>
std::vector<BitType> AesCircuit<BitType>::decrypt_impl(
    const std::vector<BitType>& ciphertext,
    const std::vector<BitType>& expandedDecKey) const {
  throw std::runtime_error("Not implemented!");
}

template <typename BitType>
std::vector<std::array<AesCircuit<BitType>::WordType, 4>>
AesCircuit<BitType>::convertToWords(const std::vector<BitType>& src) const {
  throw std::runtime_error("Not implemented!");
}

template <typename BitType>
std::vector<BitType> AesCircuit<BitType>::convertFromWords(
    std::vector<std::array<WordType, 4>>& src) const {
  throw std::runtime_error("Not implemented!");
}

template <typename BitType>
void AesCircuit<BitType>::sBoxInPlace(ByteType& src) const {
  throw std::runtime_error("Not implemented!");
}

template <typename BitType>
void AesCircuit<BitType>::inverseSBoxInPlace(ByteType& src) const {
  throw std::runtime_error("Not implemented!");
}

template <typename BitType>
void AesCircuit<BitType>::mixColumnsInPlace(WordType& src) const {
  throw std::runtime_error("Not implemented!");
}

template <typename BitType>
void AesCircuit<BitType>::inverseMixColumnsInPlace(WordType& src) const {
  throw std::runtime_error("Not implemented!");
}

template <typename BitType>
void AesCircuit<BitType>::shiftRowInPlace(WordType& src, int8_t offset) {
  throw std::runtime_error("Not implemented!");
}

} // namespace fbpcf::mpc_std_lib::aes_circuit
