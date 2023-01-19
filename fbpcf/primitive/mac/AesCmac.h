/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <emmintrin.h>
#include <immintrin.h>
#include <openssl/cmac.h>
#include <smmintrin.h>
#include <wmmintrin.h>
#include <xmmintrin.h>

#include "fbpcf/engine/util/aes.h"
#include "fbpcf/primitive/mac/IMac.h"

namespace fbpcf::primitive::mac {

class AesCmac final : public IMac {
 public:
  /**
   * Create a AesCmac that has an initial key and can generate 128-bit Mac for
   * arbitrary inputs.
   */
  explicit AesCmac(const std::vector<unsigned char>& macKey) : macKey_(macKey) {
    ctx_ = CMAC_CTX_new();
  }

  ~AesCmac() {
    CMAC_CTX_free(ctx_);
  }

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
  std::vector<unsigned char> macKey_;
  CMAC_CTX* ctx_;
};

} // namespace fbpcf::primitive::mac
