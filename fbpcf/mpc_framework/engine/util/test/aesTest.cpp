/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <smmintrin.h>
#include <random>
#include "fbpcf/mpc_framework/engine/util/test/aesTestHelper.h"

namespace fbpcf::engine::util {

TEST(aesTest, testEncryptionAndDecryption) {
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);

  __m128i key = _mm_set_epi32(dist(e), dist(e), dist(e), dist(e));
  AesTestHelper cipher(key);

  std::vector<__m128i> plaintext(16);
  for (int i = 0; i < 16; i++) {
    plaintext[i] = _mm_set_epi32(dist(e), dist(e), dist(e), dist(e));
  }
  auto duplicate = plaintext;

  cipher.encryptInPlace(duplicate);
  cipher.switchToDecrypt();
  cipher.decryptInPlace(duplicate);

  for (int i = 0; i < 16; i++) {
    EXPECT_TRUE(_mm_testz_si128(
        _mm_xor_si128(duplicate[i], plaintext[i]),
        _mm_xor_si128(duplicate[i], plaintext[i])));
  }
}

} // namespace fbpcf::engine::util
