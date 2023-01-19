/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <emmintrin.h>
#include <immintrin.h>
#include <smmintrin.h>
#include <wmmintrin.h>
#include <xmmintrin.h>
#include <vector>

#include "fbpcf/engine/util/util.h"
#include "fbpcf/primitive/mac/AesCmac.h"
#include "fbpcf/primitive/mac/IMac.h"

namespace fbpcf::primitive::mac {

const uint64_t rb_nonce = 0x0000000000000087;

class S2v final : public IMac {
 public:
  /**
   * Create a S2v that has an initial key and can generate 128-bit Synthetic
   * Initialization Vector based on any arbitrary inputs.
   * It is noted that the S2V in RFC5297 could intake associated data that could
   * be fields (like addresses, ports) will not be encrypted. We have no
   * associated data in our current use cases so the current S2V doesn't support
   * it. So the input text is the plaintext only, which is denoted as Sn in S2V
   * RFC5297.
   */
  explicit S2v(std::unique_ptr<IMac> mac) : mac_(std::move(mac)) {}

  /**
   * @inherit doc
   */
  std::vector<unsigned char> getMac128(
      const std::vector<unsigned char>& text) const override;

  /**
   * @inherit doc
   */
  __m128i getMacM128i(const std::vector<unsigned char>& text) const override;

 private:
  std::unique_ptr<IMac> mac_;
};

} // namespace fbpcf::primitive::mac
