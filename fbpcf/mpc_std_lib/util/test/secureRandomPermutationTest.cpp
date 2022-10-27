/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/mpc_std_lib/util/secureRandomPermutation.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "fbpcf/engine/util/AesPrgFactory.h"
#include "fbpcf/engine/util/util.h"
#include "fbpcf/test/TestHelper.h"

namespace fbpcf::mpc_std_lib::util {
void permutationTest(engine::util::IPrg& prg) {
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint32_t> dist(1, 0xFFF);
  uint32_t size = dist(e);
  const std::vector<uint32_t>& permutation = secureRandomPermutation(size, prg);
  EXPECT_EQ(size, permutation.size());
  for (uint32_t i = 0; i < size; i++) {
    EXPECT_TRUE(
        std::find(permutation.begin(), permutation.end(), i) !=
        permutation.end());
  }
}

TEST(permutationTest, testPermutationWithAesPrg) {
  auto prgFactory = std::make_unique<engine::util::AesPrgFactory>();
  auto prg = prgFactory->create(engine::util::getRandomM128iFromSystemNoise());
  permutationTest(*prg);
}

} // namespace fbpcf::mpc_std_lib::util
