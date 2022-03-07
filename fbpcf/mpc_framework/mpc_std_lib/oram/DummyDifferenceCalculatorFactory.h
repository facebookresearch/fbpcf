/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/mpc_framework/mpc_std_lib/oram/DummyDifferenceCalculator.h"
#include "fbpcf/mpc_framework/mpc_std_lib/oram/IDifferenceCalculatorFactory.h"

namespace fbpcf::mpc_framework::mpc_std_lib::oram::insecure {

template <typename T, int8_t indicatorSumWidth>
class DummyDifferenceCalculatorFactory final
    : public IDifferenceCalculatorFactory<T> {
 public:
  DummyDifferenceCalculatorFactory(
      bool thisPartyToSetDifference,
      int32_t peerId,
      engine::communication::IPartyCommunicationAgentFactory& factory)
      : thisPartyToSetDifference_(thisPartyToSetDifference),
        peerId_(peerId),
        factory_(factory) {}

  std::unique_ptr<IDifferenceCalculator<T>> create() override {
    return std::make_unique<DummyDifferenceCalculator<T, indicatorSumWidth>>(
        thisPartyToSetDifference_, factory_.create(peerId_));
  }

 private:
  bool thisPartyToSetDifference_;
  int32_t peerId_;
  engine::communication::IPartyCommunicationAgentFactory& factory_;
};

} // namespace fbpcf::mpc_framework::mpc_std_lib::oram::insecure
