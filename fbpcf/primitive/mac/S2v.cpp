/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/primitive/mac/S2v.h"
namespace fbpcf::primitive::mac {

__m128i S2v::getMacM128i(const std::vector<unsigned char>& text) const {
  const std::vector<unsigned char> cmac = getMac128(text);
  return engine::util::buildM128i(cmac);
}

std::vector<unsigned char> S2v::getMac128(
    const std::vector<unsigned char>& text) const {
  __m128i rb = _mm_set_epi64x(0, rb_nonce);
  std::vector<unsigned char> textCopy(text);
  std::vector<unsigned char> initialBlock(16);

  __m128i initalMac = mac_->getMacM128i(initialBlock);

  // dbl
  bool msb = engine::util::getMsb(initalMac);
  engine::util::lShiftByBitsInPlace(initalMac, 1);
  if (msb) {
    initalMac = _mm_xor_si128(initalMac, rb);
  }
  _mm_storeu_si128((__m128i*)initialBlock.data(), initalMac);

  // pad
  if (textCopy.size() < 16) {
    textCopy.push_back(0x80);
    while (textCopy.size() < 16) {
      textCopy.push_back(0x00);
    }
  }

  // xor or xorend
  for (size_t i = 0; i < 16; ++i) {
    textCopy.at(textCopy.size() - 1 - i) ^= initialBlock.at(15 - i);
  }

  return mac_->getMac128(textCopy);
}

} // namespace fbpcf::primitive::mac
