/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <assert.h>
#include <emmintrin.h>
#include <cstdint>
#include <vector>
#include "fbpcf/engine/util/util.h"

namespace fbpcf::primitive::mac {

class IMac {
 public:
  virtual ~IMac() = default;

  /**
   * Generate 128-bit MAC for the input text
   * @param text the data on which the MAC is calculated.
   * @return a MAC with 128 bits as a __m128i
   */
  virtual __m128i getMacM128i(const std::vector<unsigned char>& text) const = 0;

  /**
   * Generate 128-bit MAC for the input text
   * @param text the data on which the MAC is calculated.
   * @return a MAC with 16 bytes as an unsigned char vector
   */
  virtual std::vector<unsigned char> getMac128(
      const std::vector<unsigned char>& text) const = 0;
};

} // namespace fbpcf::primitive::mac
