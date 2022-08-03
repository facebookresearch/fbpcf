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
#include "fbpcf/engine/util/util.h"

namespace fbpcf::engine::util {

class IPrg {
 public:
  virtual ~IPrg() = default;

  __m128i getRandomM128i() {
    auto randomBytes = getRandomBytes(16);
    assert(randomBytes.size() == 16);
    return buildM128i(randomBytes);
  }

  virtual std::vector<bool> getRandomBits(uint32_t size) = 0;

  /**
   * Generate random bytes from async buffer
   * @param size the number of random bytes being generated
   * @return each element in the vector is a random byte
   */
  virtual std::vector<unsigned char> getRandomBytes(uint32_t size) = 0;

  /**
   * Generate random unsigned 64 bit integers from random bytes
   * @param size the number of random unsigned 64 bit integers being generated
   * @return each element in the vector is a random unsigned 64 bit integer
   */
  virtual std::vector<uint64_t> getRandomUInt64(uint32_t size) = 0;
};

} // namespace fbpcf::engine::util
