/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/engine/util/util.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <smmintrin.h>
#include <random>

namespace fbpcf::engine::util {

TEST(UtilTest, testGetLsb) {
  for (size_t i = 0; i < 1024; i++) {
    __m128i val = getRandomM128iFromSystemNoise();
    EXPECT_EQ(util::getLsb(val), _mm_extract_epi64(val, 0) % 2 != 0);
  }
}

TEST(UtilTest, testGetMsb) {
  for (size_t i = 0; i < 1024; i++) {
    __m128i val = getRandomM128iFromSystemNoise();
    EXPECT_EQ(
        util::getMsb(val), (_mm_extract_epi64(val, 1) & ((long)1 << 63)) != 0);
  }
}

TEST(UtilTest, testlShiftByBitsInPlace) {
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<unsigned char> dist(0, 0xFF);
  for (size_t i = 0; i < 10; i++) {
    std::vector<unsigned char> randomVal(16);
    for (size_t j = 0; j < randomVal.size(); ++j) {
      randomVal[j] = dist(e);
    }
    std::vector<unsigned char> randomValOriginal(randomVal);
    for (int offset = 1; offset < 128; offset++) {
      __m128i val = buildM128i(randomValOriginal);
      lShiftByBitsInPlace(val, offset);
      // left shift char array by 1 bit accordingly
      for (int j = 15; j >= 0; j--) {
        int carry = randomVal[j] >> 7 & 1;
        randomVal[j] = randomVal[j] << 1;
        if (j != 15)
          randomVal[j + 1] |= carry;
      }
      __m128i valShifted = buildM128i(randomVal);
      EXPECT_EQ(_mm_extract_epi64(val, 0), _mm_extract_epi64(valShifted, 0));
      EXPECT_EQ(_mm_extract_epi64(val, 1), _mm_extract_epi64(valShifted, 1));
    }
  }
}

TEST(UtilTest, testExtractLnbToVector) {
  for (size_t i = 0; i < 20; i++) {
    __m128i val = getRandomM128iFromSystemNoise();
    for (size_t j = 0; j < 128; j++) {
      std::vector<bool> bits(j);
      util::extractLnbToVector(val, bits);

      uint64_t lower64 = _mm_extract_epi64(val, 0);
      uint64_t upper64 = _mm_extract_epi64(val, 1);

      for (size_t k = 0; k < j; k++) {
        if (k < 64) {
          EXPECT_EQ(bits[k], (lower64 >> k) & 1);
        } else {
          EXPECT_EQ(bits[k], (upper64 >> (k - 64) & 1));
        }
      }
    }
  }
}

TEST(UtilTest, setLsb) {
  for (size_t i = 0; i < 1024; i++) {
    __m128i val = getRandomM128iFromSystemNoise();
    util::setLsbTo0(val);
    EXPECT_EQ(util::getLsb(val), false);
    util::setLsbTo1(val);
    EXPECT_EQ(util::getLsb(val), true);
  }
}

TEST(UtilTest, testBigNumMod) {
  BN_CTX* ctx = BN_CTX_new();
  std::vector<uint8_t> num{25};
  EXPECT_EQ(mod(num, 4, ctx), 1);
  EXPECT_EQ(mod(num, 7, ctx), 4);

  for (uint64_t i = 0; i <= 251; i++) {
    num = {
        (uint8_t)i,
        (uint8_t)(i + 1),
        (uint8_t)(i + 2),
        (uint8_t)(i + 3),
        (uint8_t)(i + 4),
    };

    auto remainder = mod(num, 10000, ctx);

    uint32_t expected = (i + ((i + 1) << 8) + ((i + 2) << 16) +
                         ((i + 3) << 24) + ((i + 4) << 32)) %
        10000;

    EXPECT_EQ(remainder, expected);
  }

  BN_CTX_free(ctx);
}
} // namespace fbpcf::engine::util
