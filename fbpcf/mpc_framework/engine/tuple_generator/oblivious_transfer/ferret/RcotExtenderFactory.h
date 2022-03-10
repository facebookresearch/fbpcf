/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/IMatrixMultiplierFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/IMultiPointCotFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/IRcotExtenderFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/RcotExtender.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::ferret {

class RcotExtenderFactory final : public IRcotExtenderFactory {
 public:
  RcotExtenderFactory(
      std::unique_ptr<IMatrixMultiplierFactory> MatrixMultiplierFactory,
      std::unique_ptr<IMultiPointCotFactory> multiPointCotFactory)
      : MatrixMultiplierFactory_(std::move(MatrixMultiplierFactory)),
        multiPointCotFactory_(std::move(multiPointCotFactory)) {}

  std::unique_ptr<IRcotExtender> create() override {
    return std::make_unique<RcotExtender>(
        MatrixMultiplierFactory_->create(), *multiPointCotFactory_);
  }

 private:
  std::unique_ptr<IMatrixMultiplierFactory> MatrixMultiplierFactory_;
  std::unique_ptr<IMultiPointCotFactory> multiPointCotFactory_;
};

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::ferret
