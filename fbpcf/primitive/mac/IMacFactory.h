/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <emmintrin.h>
#include "fbpcf/primitive/mac/IMac.h"

namespace fbpcf::primitive::mac {

/**
 * MAC factory API, create a MAC generator with a given key
 */
class IMacFactory {
 public:
  virtual ~IMacFactory() = default;
  virtual std::unique_ptr<IMac> create(
      const std::vector<unsigned char>& macKey) const = 0;
};

} // namespace fbpcf::primitive::mac
