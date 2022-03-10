/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emmintrin.h>
#include <vector>
#include "fbpcf/mpc_framework/engine/util/util.h"

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

} // namespace fbpcf::engine::util
