/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/mpc_framework/mpc_std_lib/oram/DummyObliviousDeltaCalculator.h"
#include "fbpcf/mpc_framework/mpc_std_lib/oram/IObliviousDeltaCalculatorFactory.h"

namespace fbpcf::mpc_framework::mpc_std_lib::oram::insecure {

class DummyObliviousDeltaCalculatorFactory final
    : public IObliviousDeltaCalculatorFactory {
 public:
  DummyObliviousDeltaCalculatorFactory(
      int32_t peerId,
      engine::communication::IPartyCommunicationAgentFactory& factory)
      : peerId_(peerId), factory_(factory) {}

  std::unique_ptr<IObliviousDeltaCalculator> create() override {
    return std::make_unique<DummyObliviousDeltaCalculator>(
        factory_.create(peerId_));
  }

 private:
  int32_t peerId_;
  engine::communication::IPartyCommunicationAgentFactory& factory_;
};

} // namespace fbpcf::mpc_framework::mpc_std_lib::oram::insecure
