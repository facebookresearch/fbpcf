/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/mpc_std_lib/util/util.h"
#include <smmintrin.h>
#include <stdexcept>

namespace fbpcf::mpc_std_lib::util {

std::vector<bool> convertToBits(__m128i src) {
  std::vector<bool> rst(128);
  auto tmp = _mm_extract_epi64(src, 0);
  for (size_t i = 0; i < 64; i++) {
    rst[i] = tmp & 1;
    tmp >>= 1;
  }

  tmp = _mm_extract_epi64(src, 1);
  for (size_t i = 0; i < 64; i++) {
    rst[i + 64] = tmp & 1;
    tmp >>= 1;
  }

  return rst;
}

std::vector<std::vector<bool>> convertToBits(const std::vector<__m128i>& src) {
  size_t batchSize = src.size();
  std::vector<std::vector<bool>> rst(128, std::vector<bool>(batchSize));
  for (size_t i = 0; i < batchSize; i++) {
    auto tmp = convertToBits(src.at(i));
    for (size_t j = 0; j < 128; j++) {
      rst[j][i] = tmp.at(j);
    }
  }
  return rst;
}

std::vector<__m128i> convertFromBits(
    const std::vector<std::vector<bool>>& src) {
  if (src.size() != 128) {
    throw std::runtime_error("Unexpected input size.");
  }
  auto batchSize = src.at(0).size();
  std::vector<int64_t> low(batchSize, 0);
  std::vector<int64_t> high(batchSize, 0);
  for (size_t i = 0; i < 64; i++) {
    for (size_t j = 0; j < batchSize; j++) {
      low[j] <<= 1;
      high[j] <<= 1;
      low[j] ^= src.at(63 - i).at(j);
      high[j] ^= src.at(127 - i).at(j);
    }
  }
  std::vector<__m128i> rst(batchSize);
  for (size_t j = 0; j < batchSize; j++) {
    rst[j] = _mm_set_epi64x(high.at(j), low.at(j));
  }
  return rst;
}

} // namespace fbpcf::mpc_std_lib::util
