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
#include "fbpcf/mpc_framework/engine/util/AesPrg.h"

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

TEST(AesPrgTest, testThrow) {
  AesPrg prg(_mm_set_epi32(1, 2, 3, 4));
  EXPECT_THROW(prg.getRandomBytes(10), std::runtime_error);
  EXPECT_THROW(prg.getRandomBits(10), std::runtime_error);
}

} // namespace fbpcf::engine::util
