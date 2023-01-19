/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include "fbpcf/primitive/mac/AesCmac.h"
#include "fbpcf/primitive/mac/IMacFactory.h"

namespace fbpcf::primitive::mac {

/**
 * An AesCmac factory for creating 128-bit AesCmac.
 */
class AesCmacFactory final : public IMacFactory {
 public:
  std::unique_ptr<IMac> create(
      const std::vector<unsigned char>& macKey) const override {
    return std::make_unique<AesCmac>(macKey);
  }
};

} // namespace fbpcf::primitive::mac
