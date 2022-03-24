/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/mpc_std_lib/permuter/AsWaksmanPermuter.h"
#include "fbpcf/mpc_std_lib/permuter/IPermuterFactory.h"

namespace fbpcf::mpc_std_lib::permuter {

template <typename T, int schedulerId>
class AsWaksmanPermuterFactory final
    : public IPermuterFactory<
          typename util::SecBatchType<T, schedulerId>::type> {
 public:
  AsWaksmanPermuterFactory(int myId, int partnerId)
      : myId_(myId), partnerId_(partnerId) {}

  std::unique_ptr<IPermuter<typename util::SecBatchType<T, schedulerId>::type>>
  create() override {
    return std::make_unique<AsWaksmanPermuter<T, schedulerId>>(
        myId_, partnerId_);
  }

 private:
  int myId_;
  int partnerId_;
};

} // namespace fbpcf::mpc_std_lib::permuter
