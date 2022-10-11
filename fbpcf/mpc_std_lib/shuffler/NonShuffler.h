/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/mpc_std_lib/shuffler/IShuffler.h"

namespace fbpcf::mpc_std_lib::shuffler::insecure {

/**
 * This shuffler doesn't do anything but simply output the input. It is only
 *meant to be used as a placeholder in tests.
 **/
template <typename T>
class NonShuffler final : public IShuffler<T> {
 public:
  T shuffle(const T& src, uint32_t /*size*/) const override {
    return src;
  }
};

} // namespace fbpcf::mpc_std_lib::shuffler::insecure
