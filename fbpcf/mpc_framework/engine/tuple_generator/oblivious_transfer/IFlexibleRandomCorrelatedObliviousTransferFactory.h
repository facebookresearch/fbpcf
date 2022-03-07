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
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/IFlexibleRandomCorrelatedObliviousTransfer.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/IRandomCorrelatedObliviousTransferFactory.h"

namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer {

class IFlexibleRandomCorrelatedObliviousTransferFactory
    : public IRandomCorrelatedObliviousTransferFactory {
 public:
  std::unique_ptr<IRandomCorrelatedObliviousTransfer> create(
      __m128i delta,
      std::unique_ptr<communication::IPartyCommunicationAgent> agent) override {
    return createFlexible(delta, std::move(agent));
  }

  std::unique_ptr<IRandomCorrelatedObliviousTransfer> create(
      std::unique_ptr<communication::IPartyCommunicationAgent> agent) override {
    return createFlexible(std::move(agent));
  }

  /**
   * Construct sender
   */
  virtual std::unique_ptr<IFlexibleRandomCorrelatedObliviousTransfer>
  createFlexible(
      __m128i delta,
      std::unique_ptr<communication::IPartyCommunicationAgent> agent) = 0;

  /**
   * Construct receiver
   */
  virtual std::unique_ptr<IFlexibleRandomCorrelatedObliviousTransfer>
  createFlexible(
      std::unique_ptr<communication::IPartyCommunicationAgent> agent) = 0;
};

} // namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer
