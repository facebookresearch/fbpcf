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
  __m128i temp1;
  __m128i temp2;

  roundKey_[0] = key;
  temp1 = key;
  temp2 = _mm_aeskeygenassist_si128(temp1, 0x1);
  temp1 = aes128KeyExpandAssist(temp1, temp2);
  roundKey_[1] = temp1;
  temp2 = _mm_aeskeygenassist_si128(temp1, 0x2);
  temp1 = aes128KeyExpandAssist(temp1, temp2);
  roundKey_[2] = temp1;
  temp2 = _mm_aeskeygenassist_si128(temp1, 0x4);
  temp1 = aes128KeyExpandAssist(temp1, temp2);
  roundKey_[3] = temp1;
  temp2 = _mm_aeskeygenassist_si128(temp1, 0x8);
  temp1 = aes128KeyExpandAssist(temp1, temp2);
  roundKey_[4] = temp1;
  temp2 = _mm_aeskeygenassist_si128(temp1, 0x10);
  temp1 = aes128KeyExpandAssist(temp1, temp2);
  roundKey_[5] = temp1;
  temp2 = _mm_aeskeygenassist_si128(temp1, 0x20);
  temp1 = aes128KeyExpandAssist(temp1, temp2);
  roundKey_[6] = temp1;
  temp2 = _mm_aeskeygenassist_si128(temp1, 0x40);
  temp1 = aes128KeyExpandAssist(temp1, temp2);
  roundKey_[7] = temp1;
  temp2 = _mm_aeskeygenassist_si128(temp1, 0x80);
  temp1 = aes128KeyExpandAssist(temp1, temp2);
  roundKey_[8] = temp1;
  temp2 = _mm_aeskeygenassist_si128(temp1, 0x1b);
  temp1 = aes128KeyExpandAssist(temp1, temp2);
  roundKey_[9] = temp1;
  temp2 = _mm_aeskeygenassist_si128(temp1, 0x36);
  temp1 = aes128KeyExpandAssist(temp1, temp2);
  roundKey_[kRound] = temp1;
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
