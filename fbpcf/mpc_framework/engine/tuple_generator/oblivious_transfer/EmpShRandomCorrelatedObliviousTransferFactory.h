/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <openssl/rand.h>
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/EmpShRandomCorrelatedObliviousTransfer.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/IFlexibleRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/IRcotExtenderFactory.h"
#include "fbpcf/mpc_framework/engine/util/IPrgFactory.h"
#include "fbpcf/mpc_framework/engine/util/util.h"

namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer {

class EmpShRandomCorrelatedObliviousTransferFactory final
    : public IFlexibleRandomCorrelatedObliviousTransferFactory {
 public:
  explicit EmpShRandomCorrelatedObliviousTransferFactory(
      std::unique_ptr<util::IPrgFactory> prgFactory)
      : prgFactory_(std::move(prgFactory)) {}

  std::unique_ptr<IFlexibleRandomCorrelatedObliviousTransfer> createFlexible(
      __m128i delta,
      std::unique_ptr<communication::IPartyCommunicationAgent> agent) override {
    return std::make_unique<EmpShRandomCorrelatedObliviousTransfer>(
        delta, std::move(agent));
  }

  std::unique_ptr<IFlexibleRandomCorrelatedObliviousTransfer> createFlexible(
      std::unique_ptr<communication::IPartyCommunicationAgent> agent) override {
    return std::make_unique<EmpShRandomCorrelatedObliviousTransfer>(
        std::move(agent),
        prgFactory_->create(util::getRandomM128iFromSystemNoise()));
  }

 private:
  std::unique_ptr<util::IPrgFactory> prgFactory_;
};

} // namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer
