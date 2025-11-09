/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <emmintrin.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <future>
#include <memory>

#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/DummyMatrixMultiplierFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/IMatrixMultiplier.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/TenLocalLinearMatrixMultiplierFactory.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::ferret {
int getHammingWeight(uint8_t src) {
  int count = 0;
  for (int i = 0; i < 8; i++) {
    count += src & 1;
    src >>= 1;
  }
  return count;
}

int getHammingWeight(__m128i src) {
  uint8_t* pointer = reinterpret_cast<uint8_t*>(&src);
  int count = 0;
  for (int i = 0; i < sizeof(__m128i); i++) {
    count += getHammingWeight(pointer[i]);
  }
  return count;
}

void testMatrixMultiplier(
    std::unique_ptr<IMatrixMultiplier> matrixMultiplier,
    int weightCap) {
  std::vector<__m128i> src(128);
  uint64_t index = 1;
  for (int i = 0; i < 64; i++) {
    src[i] = _mm_set_epi64x(0, index);
    src[i + 64] = _mm_set_epi64x(index, 0);
    index <<= 1;
  }
  __m128i seed = _mm_set_epi64x(123, 456);
  int length = 16384;
  auto rst = matrixMultiplier->multiplyWithRandomMatrix(seed, length, src);
  EXPECT_EQ(rst.size(), length);
  for (int i = 0; i < length; i++) {
    EXPECT_LE(getHammingWeight(rst[i]), weightCap);
  }
}

TEST(MatrixMultiplierTest, testDummyMatrixMultiplier) {
  insecure::DummyMatrixMultiplierFactory factory;

  testMatrixMultiplier(factory.create(), 1);
}

TEST(MatrixMultiplierTest, test10LocalLinearMatrixMultiplier) {
  TenLocalLinearMatrixMultiplierFactory factory;

  testMatrixMultiplier(factory.create(), 10);
}

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::ferret
