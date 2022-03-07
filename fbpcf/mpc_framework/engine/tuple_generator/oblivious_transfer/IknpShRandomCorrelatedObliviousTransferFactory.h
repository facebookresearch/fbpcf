/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <assert.h>

#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/IBaseObliviousTransferFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/IFlexibleRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/IRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/IknpShRandomCorrelatedObliviousTransfer.h"
#include "fbpcf/mpc_framework/engine/util/AesPrgFactory.h"

namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer {
class IknpShRandomCorrelatedObliviousTransferFactory final
    : public IFlexibleRandomCorrelatedObliviousTransferFactory {
 public:
  /**
   * @param baseOtFactory factory to create the base ot
   */
  explicit IknpShRandomCorrelatedObliviousTransferFactory(
      std::unique_ptr<IBaseObliviousTransferFactory> baseOtFactory)
      : baseOtFactory_(std::move(baseOtFactory)) {}

  std::unique_ptr<IFlexibleRandomCorrelatedObliviousTransfer> createFlexible(
      __m128i delta,
      std::unique_ptr<communication::IPartyCommunicationAgent> agent) override {
    auto prgFactory = std::make_unique<util::AesPrgFactory>();
    auto baseOt = baseOtFactory_->create(std::move(agent));
    return std::make_unique<IknpShRandomCorrelatedObliviousTransfer>(
        delta, std::move(baseOt), std::move(prgFactory));
  }

  std::unique_ptr<IFlexibleRandomCorrelatedObliviousTransfer> createFlexible(
      std::unique_ptr<communication::IPartyCommunicationAgent> agent) override {
    auto prgFactory = std::make_unique<util::AesPrgFactory>();
    auto baseOt = baseOtFactory_->create(std::move(agent));
    return std::make_unique<IknpShRandomCorrelatedObliviousTransfer>(
        std::move(baseOt), std::move(prgFactory));
  }

 private:
  std::unique_ptr<IBaseObliviousTransferFactory> baseOtFactory_;
};

} // namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer
