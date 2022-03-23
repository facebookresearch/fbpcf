/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/mpc_std_lib/permuter/DummyPermuter.h"
#include "fbpcf/mpc_std_lib/permuter/IPermuterFactory.h"

namespace fbpcf::mpc_std_lib::permuter::insecure {

template <typename T>
class DummyPermuterFactory final : public IPermuterFactory<T> {
 public:
  DummyPermuterFactory(int myId, int partnerId)
      : myId_(myId), partnerId_(partnerId) {}

  std::unique_ptr<IPermuter<T>> create() override {
    return std::make_unique<DummyPermuter<T>>(myId_, partnerId_);
  }

 private:
  int myId_;
  int partnerId_;
};

} // namespace fbpcf::mpc_std_lib::permuter::insecure
