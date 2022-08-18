/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emmintrin.h>
#include <cstdint>
#include <vector>
#include "fbpcf/engine/util/util.h"

namespace fbpcf::engine::util {

template <>
class Masker<bool> {
 public:
  static bool mask(bool src, __m128i key) {
    return src ^ getLsb(key);
  }
  static bool
  unmask(__m128i key, bool choice, bool correction0, bool correction1) {
    return getLsb(key) ^ (correction0 ^ choice * (correction0 ^ correction1));
  }
};

template <>
class Masker<uint64_t> {
 public:
  static uint64_t mask(uint64_t src, __m128i key) {
    return src - getLast64Bits(key);
  }
  static uint64_t
  unmask(__m128i key, bool choice, uint64_t correction0, uint64_t correction1) {
    return getLast64Bits(key) +
        (correction0 + choice * (correction1 - correction0));
  }
};
} // namespace fbpcf::engine::util
