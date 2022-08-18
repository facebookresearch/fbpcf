/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include "fbpcf/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/engine/tuple_generator/ITupleGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/TwoPartyTupleGenerator.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/IRandomCorrelatedObliviousTransfer.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/IRandomCorrelatedObliviousTransferFactory.h"

namespace fbpcf::engine::tuple_generator {

class TwoPartyTupleGeneratorFactory final : public ITupleGeneratorFactory {
 public:
  TwoPartyTupleGeneratorFactory(
      std::unique_ptr<
          oblivious_transfer::IRandomCorrelatedObliviousTransferFactory>
          rcotFactory,
      communication::IPartyCommunicationAgentFactory& agentFactory,
      int myId,
      uint64_t bufferSize)
      : rcotFactory_{std::move(rcotFactory)},
        agentFactory_{agentFactory},
        myId_(myId),
        bufferSize_(bufferSize) {}

  /**
   * Create a two party tuple generator.
   */
  std::unique_ptr<ITupleGenerator> create() override {
    auto delta = util::getRandomM128iFromSystemNoise();
    util::setLsbTo1(delta);

    std::unique_ptr<oblivious_transfer::IRandomCorrelatedObliviousTransfer>
        senderRcot;
    std::unique_ptr<oblivious_transfer::IRandomCorrelatedObliviousTransfer>
        receiverRcot;

    auto otherId = 1 - myId_;

    if (myId_ == 0) {
      senderRcot = rcotFactory_->create(
          delta,
          agentFactory_.create(
              otherId, "two_party_tuple_generator_traffic_as_ot_sender"));
      receiverRcot = rcotFactory_->create(agentFactory_.create(
          otherId, "two_party_tuple_generator_traffic_as_ot_receiver"));
    } else {
      receiverRcot = rcotFactory_->create(agentFactory_.create(
          otherId, "two_party_tuple_generator_traffic_as_ot_receiver"));
      senderRcot = rcotFactory_->create(
          delta,
          agentFactory_.create(
              otherId, "two_party_tuple_generator_traffic_as_ot_sender"));
    }
    auto recorder = std::make_shared<TuplesMetricRecorder>();

    return std::make_unique<TwoPartyTupleGenerator>(
        std::move(senderRcot),
        std::move(receiverRcot),
        delta,
        recorder,
        bufferSize_);
  }

 private:
  std::unique_ptr<oblivious_transfer::IRandomCorrelatedObliviousTransferFactory>
      rcotFactory_;
  communication::IPartyCommunicationAgentFactory& agentFactory_;
  int myId_;
  uint64_t bufferSize_;
};

} // namespace fbpcf::engine::tuple_generator
