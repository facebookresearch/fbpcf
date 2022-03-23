/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/engine/util/IPrgFactory.h"
#include "fbpcf/engine/util/util.h"
#include "fbpcf/mpc_std_lib/permuter/IPermuterFactory.h"
#include "fbpcf/mpc_std_lib/shuffler/IShufflerFactory.h"
#include "fbpcf/mpc_std_lib/shuffler/PermuteBasedShuffler.h"

namespace fbpcf::mpc_std_lib::shuffler {

template <typename T>
class PermuteBasedShufflerFactory final : public IShufflerFactory<T> {
 public:
  PermuteBasedShufflerFactory(
      int myId,
      int partnerId,
      std::unique_ptr<permuter::IPermuterFactory<T>> permuterFactory,
      std::unique_ptr<engine::util::IPrgFactory> prgFactory)
      : myId_(myId),
        partnerId_(partnerId),
        permuterFactory_(std::move(permuterFactory)),
        prgFactory_(std::move(prgFactory)) {}

  std::unique_ptr<IShuffler<T>> create() override {
    return std::make_unique<PermuteBasedShuffler<T>>(
        myId_,
        partnerId_,
        permuterFactory_->create(),
        prgFactory_->create(engine::util::getRandomM128iFromSystemNoise()));
  }

 private:
  int myId_;
  int partnerId_;
  std::unique_ptr<permuter::IPermuterFactory<T>> permuterFactory_;
  std::unique_ptr<engine::util::IPrgFactory> prgFactory_;
};

} // namespace fbpcf::mpc_std_lib::shuffler
