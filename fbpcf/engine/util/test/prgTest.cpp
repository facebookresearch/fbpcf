/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <random>
#include <stdexcept>
#include "fbpcf/engine/util/AesPrg.h"
#include "fbpcf/engine/util/util.h"

namespace fbpcf::engine::util {

TEST(AesPrgTest, testRandomBytesWithSIMDAes) {
  AesPrg prg(_mm_set_epi32(1, 2, 3, 4), 1024);
  auto s1 = prg.getRandomBytes(256);
  auto s2 = prg.getRandomBytes(256);
  EXPECT_NE(s1, s2);
  AesPrg prgFromSystemNoise(getRandomM128iFromSystemNoise(), 1024);
  s1 = prgFromSystemNoise.getRandomBytes(256);
  s2 = prgFromSystemNoise.getRandomBytes(256);
  EXPECT_NE(s1, s2);
}

TEST(AesPrgTest, testGetRandomBits) {
  AesPrg prg1(_mm_set_epi32(1, 2, 3, 4), 1024);
  auto randomBits = prg1.getRandomBits(32);

  AesPrg prg2(_mm_set_epi32(1, 2, 3, 4), 1024);
  auto randomBytes = prg2.getRandomBytes(4);

  for (auto byte = 0; byte < randomBytes.size(); ++byte) {
    unsigned char bitVal = 0;
    for (auto i = 0; i < 8; ++i) {
      bitVal ^= (randomBits.at(byte * 8 + i) << i);
    }

    EXPECT_EQ(bitVal, randomBytes.at(byte));
  }
}

TEST(AesPrgTest, testGetRandomUInt64) {
  AesPrg prg1(_mm_set_epi32(1, 2, 3, 4), 1024);
  auto randomInt = prg1.getRandomUInt64(8);

  AesPrg prg2(_mm_set_epi32(1, 2, 3, 4), 1024);
  auto randomBytes = prg2.getRandomBytes(64);

  for (auto i = 0; i < randomInt.size(); i++) {
    for (auto j = 0; j < 8; j++) {
      unsigned char intVal = randomInt.at(i) >> (8 * (8 - j - 1)) & 255;
      EXPECT_EQ(intVal, randomBytes.at(i * 8 + j));
    }
  }
}

TEST(AesPrgTest, testInPlaceGeneration) {
  __m128i aes_key = _mm_set_epi32(1, 2, 3, 4);
  AesPrg prg1(aes_key);
  std::vector<bool> randomBits(192);
  std::vector<__m128i> randomData(2);
  prg1.getRandomBitsInPlace(randomBits);
  prg1.getRandomDataInPlace(randomData);

  Aes cipher(aes_key);
  std::vector<__m128i> vals{
      _mm_set_epi64x(0, 0),
      _mm_set_epi64x(0, 1),
      _mm_set_epi64x(0, 2),
      _mm_set_epi64x(0, 3)};
  cipher.encryptInPlace(vals);

  std::vector<bool> first(128);
  std::vector<bool> second(64);
  util::extractLnbToVector(vals[0], first);
  util::extractLnbToVector(vals[1], second);
  for (int i = 0; i < 128; i++) {
    EXPECT_EQ(randomBits[i], first[i]);
  }

  for (int i = 0; i < 64; i++) {
    EXPECT_EQ(randomBits[i + 128], second[i]);
  }

  EXPECT_EQ(_mm_extract_epi64(randomData[0], 0), _mm_extract_epi64(vals[2], 0));
  EXPECT_EQ(_mm_extract_epi64(randomData[0], 1), _mm_extract_epi64(vals[2], 1));
  EXPECT_EQ(_mm_extract_epi64(randomData[1], 0), _mm_extract_epi64(vals[3], 0));
  EXPECT_EQ(_mm_extract_epi64(randomData[1], 1), _mm_extract_epi64(vals[3], 1));
}

TEST(AesPrgTest, testThrow) {
  AesPrg prg(_mm_set_epi32(1, 2, 3, 4));
  EXPECT_THROW(prg.getRandomBytes(10), std::runtime_error);
  EXPECT_THROW(prg.getRandomBits(10), std::runtime_error);
}

} // namespace fbpcf::engine::util
