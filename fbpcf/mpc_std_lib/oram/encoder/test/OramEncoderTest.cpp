/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>
#include <memory>
#include <random>

#include "fbpcf/mpc_std_lib/oram/encoder/IOramEncoder.h"
#include "fbpcf/mpc_std_lib/oram/encoder/OramEncoder.h"

namespace fbpcf::mpc_std_lib::oram {

TEST(OramEncoderTest, testEncoderNoFilters) {
  std::unique_ptr<IOramEncoder> encoder = std::make_unique<OramEncoder>();

  /*
   * Total breakdown groups: 4
   * B1: [0 - 1]
   * B2: [0 - 2]
   * B3: [0 - 1]
   * B4: [0 - 3]
   * Total possible Breakdowns: 1 + 2 * 3 * 2 * 4 = 49
   * i = b1 * 24 + b2 * 8 + b3 * 4 + b4 + 1
   */

  std::vector<std::vector<uint32_t>> breakdownTuples(0);
  for (uint32_t i = 0; i < 48; i++) {
    std::vector<uint32_t> breakdownValues{
        i / 24, (i / 8) % 3, (i / 4) % 2, i % 4};
    breakdownTuples.push_back(breakdownValues);
  }

  auto mapping = encoder->generateORAMIndexes(breakdownTuples);

  for (int i = 0; i < 48; i++) {
    EXPECT_EQ(mapping[i], i + 1);
  }

  breakdownTuples.clear();

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint32_t> randomB1AndB3(0, 1);
  std::uniform_int_distribution<uint32_t> randomB2(0, 2);
  std::uniform_int_distribution<uint32_t> randomB4(0, 3);

  for (int i = 0; i < 100; i++) {
    std::vector<uint32_t> breakdownValues{
        randomB1AndB3(e), randomB2(e), randomB1AndB3(e), randomB4(e)};
    breakdownTuples.push_back(breakdownValues);
  }

  mapping = encoder->generateORAMIndexes(breakdownTuples);

  for (int i = 0; i < 100; i++) {
    EXPECT_EQ(
        mapping[i],
        1 + breakdownTuples[i][0] * 24 + breakdownTuples[i][1] * 8 +
            breakdownTuples[i][2] * 4 + breakdownTuples[i][3]);
  }
}
} // namespace fbpcf::mpc_std_lib::oram
