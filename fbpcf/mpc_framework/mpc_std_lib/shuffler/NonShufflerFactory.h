/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/mpc_framework/mpc_std_lib/shuffler/IShufflerFactory.h"
#include "fbpcf/mpc_framework/mpc_std_lib/shuffler/NonShuffler.h"

namespace fbpcf::mpc_std_lib::shuffler::insecure {

template <typename T>
class NonShufflerFactory final : public IShufflerFactory<T> {
 public:
  std::unique_ptr<IShuffler<T>> create() override {
    return std::make_unique<NonShuffler<T>>();
  }
};

} // namespace fbpcf::mpc_std_lib::shuffler::insecure
