/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <emmintrin.h>
#include <memory>

#include "fbpcf/mpc_framework/engine/util/IPrg.h"

namespace fbpcf::engine::util {

/**
 * prg factory API, create a seeded prg with a given seed
 */
class IPrgFactory {
 public:
  virtual ~IPrgFactory() = default;
  virtual std::unique_ptr<IPrg> create(__m128i seed) const = 0;
};

} // namespace fbpcf::engine::util
