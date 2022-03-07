/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <emmintrin.h>
#include <wmmintrin.h>
#include <array>
#include <cassert>
#include <cstdint>
#include <vector>

namespace fbpcf::mpc_framework::engine::util {

/*
AES algorithm can be divided into two parts:
1. key schedule
2. encryption/decryption

A scheduled key can be repeatedly used as the original key doesn't change.
Scheduling a key takes a significant portion of the total workload in
AES cipher. Therefore it would significantly improve the performance if
key scheduling can be amortized.
*/

/*
Existing SIMD instructions treat every __m128i register as an AES block.
As shown on Intel website
(https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=mm_aes&expand=2428,5304,5944,238,230)
AES encryption/decryption rounds can be executed in a signal instruction:
  * __m128i _mm_aesenc_si128 (Block a, __m128i RoundKey)
    Perform one round of an AES encryption flow on data (state) in a using the
round key in RoundKey, and store the result in dst.

  * __m128i _mm_aesenclast_si128 (Block a, __m128i RoundKey)
    Perform the last round of an AES encryption flow on data (state) in a using
the round key in RoundKey, and store the result in dst.

  * __m128i _mm_aesdec_si128 (Block a, __m128i RoundKey)
    Perform one round of an AES decryption flow on data (state) in a using the
round key in RoundKey, and store the result in dst.

  * __m128i _mm_aesdeclast_si128 (Block a, __m128i RoundKey)
    Perform the last round of an AES decryption flow on data (state) in a using
the round key in RoundKey, and store the result in dst.
*/

class Aes {
 public:
  /**
   * By default, once created the cipher is running in encryption mode
   */
  explicit Aes(__m128i key);

  void encryptInPlace(std::vector<__m128i>& plaintext) const;

  void inPlaceHash(std::vector<__m128i>& src) const;

  static __m128i getFixedKey();

 protected:
  static const uint8_t kRound = 10;
  std::array<__m128i, 11> roundKey_;

  // copy-pasted from intel's whitepaper
  inline static __m128i aes128KeyExpandAssist(__m128i& temp1, __m128i& temp2) {
    __m128i temp3;
    temp2 = _mm_shuffle_epi32(temp2, 0xff);
    temp3 = _mm_slli_si128(temp1, 0x4);
    temp1 = _mm_xor_si128(temp1, temp3);
    temp3 = _mm_slli_si128(temp3, 0x4);
    temp1 = _mm_xor_si128(temp1, temp3);
    temp3 = _mm_slli_si128(temp3, 0x4);
    temp1 = _mm_xor_si128(temp1, temp3);
    temp1 = _mm_xor_si128(temp1, temp2);
    return temp1;
  }
};

} // namespace fbpcf::mpc_framework::engine::util
