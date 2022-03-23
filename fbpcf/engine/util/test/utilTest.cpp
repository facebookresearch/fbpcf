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

namespace fbpcf::engine::util {

TEST(UtilTest, testGetLsb) {
  for (size_t i = 0; i < 1024; i++) {
    __m128i val = getRandomM128iFromSystemNoise();
    EXPECT_EQ(util::getLsb(val), _mm_extract_epi64(val, 0) % 2 != 0);
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
} // namespace fbpcf::engine::util
