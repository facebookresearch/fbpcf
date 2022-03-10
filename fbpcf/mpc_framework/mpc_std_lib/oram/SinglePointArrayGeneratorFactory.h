/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/mpc_framework/mpc_std_lib/oram/IObliviousDeltaCalculatorFactory.h"
#include "fbpcf/mpc_framework/mpc_std_lib/oram/ISinglePointArrayGeneratorFactory.h"
#include "fbpcf/mpc_framework/mpc_std_lib/oram/SinglePointArrayGenerator.h"

namespace fbpcf::mpc_std_lib::oram {

class SinglePointArrayGeneratorFactory final
    : public ISinglePointArrayGeneratorFactory {
 public:
  SinglePointArrayGeneratorFactory(
      bool firstShare,
      std::unique_ptr<IObliviousDeltaCalculatorFactory>
          obliviousCalculatrFactory)
      : firstShare_(firstShare),
        obliviousCalculatrFactory_(std::move(obliviousCalculatrFactory)) {}

  std::unique_ptr<ISinglePointArrayGenerator> create() override {
    return std::make_unique<SinglePointArrayGenerator>(
        firstShare_, obliviousCalculatrFactory_->create());
  }

 private:
  bool firstShare_;
  std::unique_ptr<IObliviousDeltaCalculatorFactory> obliviousCalculatrFactory_;
};

} // namespace fbpcf::mpc_std_lib::oram
