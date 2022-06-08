/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/engine/util/aes.h"
#include <emmintrin.h>

namespace fbpcf::engine::util {

__m128i Aes::getFixedKey() {
  return _mm_set_epi64x(0, 0);
}

Aes::Aes(__m128i key) {
  roundKey_ = expandEncryptionKey(key);
}

std::array<__m128i, 11> Aes::expandEncryptionKey(__m128i key) {
  __m128i temp1;
  __m128i temp2;
  std::array<__m128i, 11> roundKey;
  roundKey[0] = key;
  temp1 = key;
  temp2 = _mm_aeskeygenassist_si128(temp1, 0x1);
  temp1 = aes128KeyExpandAssist(temp1, temp2);
  roundKey[1] = temp1;
  temp2 = _mm_aeskeygenassist_si128(temp1, 0x2);
  temp1 = aes128KeyExpandAssist(temp1, temp2);
  roundKey[2] = temp1;
  temp2 = _mm_aeskeygenassist_si128(temp1, 0x4);
  temp1 = aes128KeyExpandAssist(temp1, temp2);
  roundKey[3] = temp1;
  temp2 = _mm_aeskeygenassist_si128(temp1, 0x8);
  temp1 = aes128KeyExpandAssist(temp1, temp2);
  roundKey[4] = temp1;
  temp2 = _mm_aeskeygenassist_si128(temp1, 0x10);
  temp1 = aes128KeyExpandAssist(temp1, temp2);
  roundKey[5] = temp1;
  temp2 = _mm_aeskeygenassist_si128(temp1, 0x20);
  temp1 = aes128KeyExpandAssist(temp1, temp2);
  roundKey[6] = temp1;
  temp2 = _mm_aeskeygenassist_si128(temp1, 0x40);
  temp1 = aes128KeyExpandAssist(temp1, temp2);
  roundKey[7] = temp1;
  temp2 = _mm_aeskeygenassist_si128(temp1, 0x80);
  temp1 = aes128KeyExpandAssist(temp1, temp2);
  roundKey[8] = temp1;
  temp2 = _mm_aeskeygenassist_si128(temp1, 0x1b);
  temp1 = aes128KeyExpandAssist(temp1, temp2);
  roundKey[9] = temp1;
  temp2 = _mm_aeskeygenassist_si128(temp1, 0x36);
  temp1 = aes128KeyExpandAssist(temp1, temp2);
  roundKey[kRound] = temp1;
  return roundKey;
}

std::array<__m128i, 11> Aes::expandDecryptionKey(__m128i key) {
  auto roundKey = expandEncryptionKey(key);
  int8_t j = 0;
  int8_t i = kRound;
  std::array<__m128i, 11> decryptionKey;
  decryptionKey[i--] = roundKey[j++];
  while (i > 0) {
    decryptionKey[i--] = _mm_aesimc_si128(roundKey[j++]);
  }
  decryptionKey[i] = roundKey[j];
  return decryptionKey;
}

void Aes::encryptInPlace(std::vector<__m128i>& plaintext) const {
  // Using naked pointer for best performance. This is very hot code.
  uint32_t size = plaintext.size();
  auto dataPointer = plaintext.data();
  for (unsigned int i = 0; i < size; ++i) {
    dataPointer[i] = _mm_xor_si128(dataPointer[i], roundKey_.at(0));
  }
  for (unsigned int j = 1; j < kRound; ++j) {
    for (unsigned int i = 0; i < size; ++i) {
      dataPointer[i] = _mm_aesenc_si128(dataPointer[i], roundKey_.at(j));
    }
  }
  for (unsigned int i = 0; i < size; ++i) {
    dataPointer[i] = _mm_aesenclast_si128(dataPointer[i], roundKey_.at(kRound));
  }
}

void Aes::inPlaceHash(std::vector<__m128i>& src) const {
  assert(!std::empty(src));
  auto tmp = src;
  encryptInPlace(src);
  for (size_t i = 0; i < src.size(); i++) {
    src[i] = _mm_xor_si128(src[i], tmp[i]);
  }
}

} // namespace fbpcf::engine::util
