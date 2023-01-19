/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/primitive/mac/AesCmac.h"
#include <fbpcf/engine/util/util.h>
namespace fbpcf::primitive::mac {

__m128i AesCmac::getMacM128i(const std::vector<unsigned char>& text) const {
  const std::vector<unsigned char> cmac = getMac128(text);
  return engine::util::buildM128i(cmac);
}

std::vector<unsigned char> AesCmac::getMac128(
    const std::vector<unsigned char>& text) const {
  std::vector<unsigned char> cmac(16);
  size_t mactlen;
  CMAC_Init(ctx_, macKey_.data(), 16, EVP_aes_128_cbc(), nullptr);
  CMAC_Update(ctx_, text.data(), sizeof(unsigned char) * text.size());
  CMAC_Final(ctx_, cmac.data(), &mactlen);
  return cmac;
}
} // namespace fbpcf::primitive::mac
