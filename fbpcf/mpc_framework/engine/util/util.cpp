/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/mpc_framework/engine/util/util.h"

namespace fbpcf::mpc_framework::engine::util {

Expander::Expander(int64_t index)
    : cipher0_(_mm_set_epi64x(0, (uint64_t)index << 1)),
      cipher1_(_mm_set_epi64x(1 + ((uint64_t)index << 1), 0)) {}

std::vector<__m128i> Expander::expand(std::vector<__m128i>&& src) const {
  // expand n __m128i variable to 2n __m128i variable with two ciphers
  assert(!std::empty(src));
  std::vector<__m128i> tmp = src;
  cipher0_.encryptInPlace(tmp);
  std::vector<__m128i> rst(src.size() * 2);
  for (int i = 0; i < src.size(); i++) {
    rst[2 * i] = _mm_xor_si128(tmp.at(i), src.at(i));
    rst[2 * i + 1] = src.at(i);
  }
  cipher1_.encryptInPlace(src);
  for (int i = 0; i < src.size(); i++) {
    rst[2 * i + 1] = _mm_xor_si128(src.at(i), rst.at(2 * i + 1));
  }
  return rst;
}

} // namespace fbpcf::mpc_framework::engine::util
