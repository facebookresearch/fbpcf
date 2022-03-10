/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include "fbpcf/engine/util/aes.h"

namespace fbpcf::engine::util {

class AesTestHelper final : public Aes {
 public:
  explicit AesTestHelper(__m128i key) : Aes(key) {}

  /**
   * Switch the cipher to decryption mode
   */
  void switchToDecrypt();

  void decryptInPlace(std::vector<__m128i>& ciphertext) const;
};

} // namespace fbpcf::engine::util
