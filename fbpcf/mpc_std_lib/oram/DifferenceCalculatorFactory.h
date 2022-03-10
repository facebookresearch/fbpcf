/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include "fbpcf/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/mpc_std_lib/oram/DifferenceCalculator.h"
#include "fbpcf/mpc_std_lib/oram/IDifferenceCalculatorFactory.h"

namespace fbpcf::mpc_std_lib::oram {

template <typename T, int8_t indicatorSumWidth, int schedulerId>
class DifferenceCalculatorFactory final
    : public IDifferenceCalculatorFactory<T> {
 public:
  DifferenceCalculatorFactory(
      bool amIParty0,
      int32_t party0Id,
      int32_t party1Id)
      : amIParty0_(amIParty0), party0Id_(party0Id), party1Id_(party1Id) {}

  std::unique_ptr<IDifferenceCalculator<T>> create() override {
    return std::make_unique<
        DifferenceCalculator<T, indicatorSumWidth, schedulerId>>(
        amIParty0_, party0Id_, party1Id_);
  }

 private:
  bool amIParty0_;
  int32_t party0Id_;
  int32_t party1Id_;
};

} // namespace fbpcf::mpc_std_lib::oram
