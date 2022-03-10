/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include "fbpcf/mpc_framework/engine/util/AesPrg.h"
#include "fbpcf/mpc_framework/engine/util/IPrgFactory.h"

namespace fbpcf::engine::util {

/**
 * an aes prg factory, always creates aes-based prg.
 */
class AesPrgFactory final : public IPrgFactory {
 public:
  explicit AesPrgFactory(int bufferSize = 1024) : bufferSize_(bufferSize) {}

  std::unique_ptr<IPrg> create(__m128i seed) const override {
    return std::make_unique<AesPrg>(seed, bufferSize_);
  }

 private:
  int bufferSize_;
};

} // namespace fbpcf::engine::util
