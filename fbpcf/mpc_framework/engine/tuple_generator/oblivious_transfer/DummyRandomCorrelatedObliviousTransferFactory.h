/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <emmintrin.h>
#include <functional>
#include <memory>
#include <vector>
#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/DummyRandomCorrelatedObliviousTransfer.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/IFlexibleRandomCorrelatedObliviousTransferFactory.h"

namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer::
    insecure {

class DummyRandomCorrelatedObliviousTransferFactory final
    : public IFlexibleRandomCorrelatedObliviousTransferFactory {
 public:
  std::unique_ptr<IFlexibleRandomCorrelatedObliviousTransfer> createFlexible(
      __m128i /*delta*/,
      std::unique_ptr<communication::IPartyCommunicationAgent> agent) override {
    return std::make_unique<DummyRandomCorrelatedObliviousTransfer>(
        std::move(agent));
  }

  std::unique_ptr<IFlexibleRandomCorrelatedObliviousTransfer> createFlexible(
      std::unique_ptr<communication::IPartyCommunicationAgent> agent) override {
    return std::make_unique<DummyRandomCorrelatedObliviousTransfer>(
        std::move(agent));
  }
};

} // namespace
  // fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer::insecure
