/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <algorithm>
#include <array>
#include <vector>
#include "fbpcf/mpc_std_lib/aes_circuit/AesCircuit.h"

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
std::vector<std::array<typename AesCircuit<BitType>::WordType, 4>>
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
  const auto& U = src;
  std::array<BitType, 28> T;

  T[1] = U[0] ^ U[3];
  T[2] = U[0] ^ U[5];
  T[3] = U[0] ^ U[6];
  T[4] = U[3] ^ U[5];
  T[5] = U[4] ^ U[6];
  T[6] = T[1] ^ T[5];
  T[7] = U[1] ^ U[2];
  T[8] = U[7] ^ T[6];
  T[9] = U[7] ^ T[7];
  T[10] = T[6] ^ T[7];
  T[11] = U[1] ^ U[5];
  T[12] = U[2] ^ U[5];
  T[13] = T[3] ^ T[4];
  T[14] = T[6] ^ T[11];
  T[15] = T[5] ^ T[11];
  T[16] = T[5] ^ T[12];
  T[17] = T[9] ^ T[16];
  T[18] = U[3] ^ U[7];
  T[19] = T[7] ^ T[18];
  T[20] = T[1] ^ T[19];
  T[21] = U[6] ^ U[7];
  T[22] = T[7] ^ T[21];
  T[23] = T[2] ^ T[22];
  T[24] = T[2] ^ T[10];
  T[25] = T[20] ^ T[17];
  T[26] = T[3] ^ T[16];
  T[27] = T[1] ^ T[12];

  std::array<BitType, 64> M;
  sharedSBoxInPlace(T, src.at(7), M);
  std::array<BitType, 30> L;

  L[0] = M[61] ^ M[62];
  L[1] = M[50] ^ M[56];
  L[2] = M[46] ^ M[48];
  L[3] = M[47] ^ M[55];
  L[4] = M[54] ^ M[58];
  L[5] = M[49] ^ M[61];
  L[6] = M[62] ^ L[5];
  L[7] = M[46] ^ L[3];
  L[8] = M[51] ^ M[59];
  L[9] = M[52] ^ M[53];
  L[10] = M[53] ^ L[4];
  L[11] = M[60] ^ L[2];
  L[12] = M[48] ^ M[51];
  L[13] = M[50] ^ L[0];
  L[14] = M[52] ^ M[61];
  L[15] = M[55] ^ L[1];
  L[16] = M[56] ^ L[0];
  L[17] = M[57] ^ L[1];
  L[18] = M[58] ^ L[8];
  L[19] = M[63] ^ L[4];
  L[20] = L[0] ^ L[1];
  L[21] = L[1] ^ L[7];
  L[22] = L[3] ^ L[12];
  L[23] = L[18] ^ L[2];
  L[24] = L[15] ^ L[9];
  L[25] = L[6] ^ L[10];
  L[26] = L[7] ^ L[9];
  L[27] = L[8] ^ L[10];
  L[28] = L[11] ^ L[14];
  L[29] = L[11] ^ L[17];

  src[0] = L[6] ^ L[24];
  src[1] = !(L[16] ^ L[26]);
  src[2] = !(L[19] ^ L[28]);
  src[3] = L[6] ^ L[21];
  src[4] = L[20] ^ L[22];
  src[5] = L[25] ^ L[29];
  src[6] = !(L[13] ^ L[27]);
  src[7] = !(L[6] ^ L[23]);
}

template <typename BitType>
void AesCircuit<BitType>::inverseSBoxInPlace(ByteType& src) const {
  const auto& U = src;
  std::array<BitType, 28> T;
  std::array<BitType, 20> R;

  T[23] = U[0] ^ U[3];
  T[22] = !(U[1] ^ U[3]);
  T[2] = !(U[0] ^ U[1]);
  T[1] = U[3] ^ U[4];
  T[24] = !(U[4] ^ U[7]);
  R[5] = U[6] ^ U[7];
  T[8] = !(U[1] ^ T[23]);
  T[19] = T[22] ^ R[5];
  T[9] = !(U[7] ^ T[1]);
  T[10] = T[2] ^ T[24];
  T[13] = T[2] ^ R[5];
  T[3] = T[1] ^ R[5];
  T[25] = !(U[2] ^ T[1]);
  R[13] = U[1] ^ U[6];
  T[17] = !(U[2] ^ T[19]);
  T[20] = T[24] ^ R[13];
  T[4] = U[4] ^ T[8];
  R[17] = !(U[2] ^ U[5]);
  R[18] = !(U[5] ^ U[6]);
  R[19] = !(U[2] ^ U[4]);
  T[6] = T[22] ^ R[17];
  T[16] = R[13] ^ R[19];
  T[27] = T[1] ^ R[18];
  T[15] = T[10] ^ T[27];
  T[14] = T[10] ^ R[18];
  T[26] = T[3] ^ T[16];

  auto D = U[0] ^ R[17];
  std::array<BitType, 64> M;
  sharedSBoxInPlace(T, D, M);

  std::array<BitType, 30> P;

  P[0] = M[52] ^ M[61];
  P[1] = M[58] ^ M[59];
  P[2] = M[54] ^ M[62];
  P[3] = M[47] ^ M[50];
  P[4] = M[48] ^ M[56];
  P[5] = M[46] ^ M[51];
  P[6] = M[49] ^ M[60];
  P[7] = P[0] ^ P[1];
  P[8] = M[50] ^ M[53];
  P[9] = M[55] ^ M[63];
  P[10] = M[57] ^ P[4];
  P[11] = P[0] ^ P[3];
  P[12] = M[46] ^ M[48];
  P[13] = M[49] ^ M[51];
  P[14] = M[49] ^ M[62];
  P[15] = M[54] ^ M[59];
  P[16] = M[57] ^ M[61];
  P[17] = M[58] ^ P[2];
  P[18] = M[63] ^ P[5];
  P[19] = P[2] ^ P[3];
  P[20] = P[4] ^ P[6];
  P[22] = P[2] ^ P[7];
  P[23] = P[7] ^ P[8];
  P[24] = P[5] ^ P[7];
  P[25] = P[6] ^ P[10];
  P[26] = P[9] ^ P[11];
  P[27] = P[10] ^ P[18];
  P[28] = P[11] ^ P[25];
  P[29] = P[15] ^ P[20];

  src[0] = P[13] ^ P[22];
  src[1] = P[26] ^ P[29];
  src[2] = P[17] ^ P[28];
  src[3] = P[12] ^ P[22];
  src[4] = P[23] ^ P[27];
  src[5] = P[19] ^ P[24];
  src[6] = P[14] ^ P[23];
  src[7] = P[9] ^ P[16];
}

template <typename BitType>
void AesCircuit<BitType>::sharedSBoxInPlace(
    const std::array<BitType, 28>& T,
    const BitType& D,
    std::array<BitType, 64>& M) const {
  M[1] = T[13] & T[6];
  M[2] = T[23] & T[8];
  M[3] = T[14] ^ M[1];
  M[4] = T[19] & D;
  M[5] = M[4] ^ M[1];
  M[6] = T[3] & T[16];
  M[7] = T[22] & T[9];
  M[8] = T[26] ^ M[6];
  M[9] = T[20] & T[17];
  M[10] = M[9] ^ M[6];
  M[11] = T[1] & T[15];
  M[12] = T[4] & T[27];
  M[13] = M[12] ^ M[11];
  M[14] = T[2] & T[10];
  M[15] = M[14] ^ M[11];
  M[16] = M[3] ^ M[2];
  M[17] = M[5] ^ T[24];
  M[18] = M[8] ^ M[7];
  M[19] = M[10] ^ M[15];
  M[20] = M[16] ^ M[13];
  M[21] = M[17] ^ M[15];
  M[22] = M[18] ^ M[13];
  M[23] = M[19] ^ T[25];
  M[24] = M[22] ^ M[23];
  M[25] = M[22] & M[20];
  M[26] = M[21] ^ M[25];
  M[27] = M[20] ^ M[21];
  M[28] = M[23] ^ M[25];
  M[29] = M[28] & M[27];
  M[30] = M[26] & M[24];
  M[31] = M[20] & M[23];
  M[32] = M[27] & M[31];
  M[33] = M[27] ^ M[25];
  M[34] = M[21] & M[22];
  M[35] = M[24] & M[34];
  M[36] = M[24] ^ M[25];
  M[37] = M[21] ^ M[29];
  M[38] = M[32] ^ M[33];
  M[39] = M[23] ^ M[30];
  M[40] = M[35] ^ M[36];
  M[41] = M[38] ^ M[40];
  M[42] = M[37] ^ M[39];
  M[43] = M[37] ^ M[38];
  M[44] = M[39] ^ M[40];
  M[45] = M[42] ^ M[41];
  M[46] = M[44] & T[6];
  M[47] = M[40] & T[8];
  M[48] = M[39] & D;
  M[49] = M[43] & T[16];
  M[50] = M[38] & T[9];
  M[51] = M[37] & T[17];
  M[52] = M[42] & T[15];
  M[53] = M[45] & T[27];
  M[54] = M[41] & T[10];
  M[55] = M[44] & T[13];
  M[56] = M[40] & T[23];
  M[57] = M[39] & T[19];
  M[58] = M[43] & T[3];
  M[59] = M[38] & T[22];
  M[60] = M[37] & T[20];
  M[61] = M[42] & T[1];
  M[62] = M[45] & T[4];
  M[63] = M[41] & T[2];
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
  if (offset == 1) {
    std::swap(src[0], src[1]);
    std::swap(src[1], src[2]);
    std::swap(src[2], src[3]);
  } else if (offset == 2) {
    std::swap(src[0], src[2]);
    std::swap(src[1], src[3]);
  } else if (offset == 3) {
    std::swap(src[3], src[2]);
    std::swap(src[2], src[1]);
    std::swap(src[1], src[0]);
  }
}

} // namespace fbpcf::mpc_std_lib::aes_circuit
