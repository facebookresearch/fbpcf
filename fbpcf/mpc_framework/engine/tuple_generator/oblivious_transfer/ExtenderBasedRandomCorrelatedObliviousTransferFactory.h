/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <assert.h>

#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ExtenderBasedRandomCorrelatedObliviousTransfer.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/IFlexibleRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/IRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/IRcotExtenderFactory.h"

namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer {

class ExtenderBasedRandomCorrelatedObliviousTransferFactory final
    : public IRandomCorrelatedObliviousTransferFactory {
 public:
  /**
   * @param bootstrappingRcotFactory factory to generate the bootstrapper
   * @param factory the RCOT extender factory, this factory will create RCOT
   * extenders and pass them into the extender based RCOT.
   * @param extendedSize a parameter for the extender
   * @param baseSize a parameter for the extender
   * @param weight a parameter for the extender
   */
  ExtenderBasedRandomCorrelatedObliviousTransferFactory(
      std::unique_ptr<IFlexibleRandomCorrelatedObliviousTransferFactory>
          bootstrappingRcotFactory,
      std::unique_ptr<ferret::IRcotExtenderFactory> factory,
      int64_t extendedSize,
      int64_t baseSize,
      int64_t weight)
      : bootstrappingRcotFactory_(std::move(bootstrappingRcotFactory)),
        factory_(std::move(factory)),
        extendedSize_(extendedSize),
        baseSize_(baseSize),
        weight_(weight) {}

  std::unique_ptr<IRandomCorrelatedObliviousTransfer> create(
      __m128i delta,
      std::unique_ptr<communication::IPartyCommunicationAgent> agent) override {
    auto extender = factory_->create();
    auto baseRcotSize =
        extender->senderInit(delta, extendedSize_, baseSize_, weight_);
    auto bootstrapperSender =
        bootstrappingRcotFactory_->createFlexible(delta, std::move(agent));
    auto baseRcotResults = bootstrapperSender->rcot(baseRcotSize);
    agent = bootstrapperSender->extractCommunicationAgent();
    extender->setCommunicationAgent(std::move(agent));

    auto ot = std::make_unique<ExtenderBasedRandomCorrelatedObliviousTransfer>(
        util::Role::sender, std::move(extender));
    ot->setBaseRcotResults(std::move(baseRcotResults));
    return ot;
  }

  std::unique_ptr<IRandomCorrelatedObliviousTransfer> create(
      std::unique_ptr<communication::IPartyCommunicationAgent> agent) override {
    auto extender = factory_->create();
    auto baseRcotSize =
        extender->receiverInit(extendedSize_, baseSize_, weight_);
    auto bootstrapperReceiver =
        bootstrappingRcotFactory_->createFlexible(std::move(agent));
    auto baseRcotResults = bootstrapperReceiver->rcot(baseRcotSize);
    agent = bootstrapperReceiver->extractCommunicationAgent();
    extender->setCommunicationAgent(std::move(agent));

    auto ot = std::make_unique<ExtenderBasedRandomCorrelatedObliviousTransfer>(
        util::Role::receiver, std::move(extender));
    ot->setBaseRcotResults(std::move(baseRcotResults));
    return ot;
  }

 private:
  std::unique_ptr<IFlexibleRandomCorrelatedObliviousTransferFactory>
      bootstrappingRcotFactory_;
  std::unique_ptr<ferret::IRcotExtenderFactory> factory_;
  int64_t extendedSize_;
  int64_t baseSize_;
  int64_t weight_;
};

} // namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer
