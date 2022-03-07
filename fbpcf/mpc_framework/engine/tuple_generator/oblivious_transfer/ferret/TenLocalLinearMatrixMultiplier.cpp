/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/TenLocalLinearMatrixMultiplier.h"
#include "fbpcf/mpc_framework/engine/util/AesPrg.h"

#include <emmintrin.h>

namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer::
    ferret {

std::vector<__m128i> TenLocalLinearMatrixMultiplier::multiplyWithRandomMatrix(
    __m128i seed,
    int64_t rstLength,
    const std::vector<__m128i>& src) const {
  uint32_t srcSize = src.size();
  uint32_t mask = 1;
  while (mask < srcSize) {
    mask = (mask << 1) ^ 1;
  }
  util::AesPrg prg(seed);
  std::vector<__m128i> rst(rstLength);

  int index = 0;
  std::vector<__m128i> randomData(10);
  uint32_t* randomNumberIndex;
  for (; index < rstLength; index += 4) {
    prg.getRandomDataInPlace(randomData);
    randomNumberIndex = reinterpret_cast<uint32_t*>(randomData.data());
    for (int i = 0; i < 4 && index + i < rstLength; i++) {
      rst[index + i] = _mm_set_epi64x(0, 0);
      // each iteration consumes 10 uint32_t random numbers
      for (int j = 0; j < 10; j++) {
        randomNumberIndex[j] &= mask;
        randomNumberIndex[j] = randomNumberIndex[j] >= srcSize
            ? (randomNumberIndex[j] - srcSize)
            : randomNumberIndex[j];
        rst[index + i] =
            _mm_xor_si128(rst[index + i], src[randomNumberIndex[j]]);
      }
      randomNumberIndex += 10;
    }
  }
  return rst;
}

} // namespace
  // fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer::ferret
