/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <assert.h>
#include <algorithm>

#include "fbpcf/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/IBidirectionObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/IRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/RcotBasedBidirectionObliviousTransfer.h"
#include "fbpcf/engine/util/util.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer {

class RcotBasedBidirectionObliviousTransferFactory final
    : public IBidirectionObliviousTransferFactory {
 public:
  RcotBasedBidirectionObliviousTransferFactory(
      int myid,
      communication::IPartyCommunicationAgentFactory& agentFactory,
      std::unique_ptr<IRandomCorrelatedObliviousTransferFactory> rcotFactory)
      : myid_(myid),
        agentFactory_(agentFactory),
        rcotFactory_(std::move(rcotFactory)) {}

  std::unique_ptr<IBidirectionObliviousTransfer> create(int id) override {
    __m128i delta = util::getRandomM128iFromSystemNoise();
    util::setLsbTo1(delta);

    std::unique_ptr<IRandomCorrelatedObliviousTransfer> senderRcot;
    std::unique_ptr<IRandomCorrelatedObliviousTransfer> receiverRcot;

    if (id < myid_) {
      senderRcot = rcotFactory_->create(
          delta, agentFactory_.create(id, "rcot_sender_traffic"));
      receiverRcot = rcotFactory_->create(
          agentFactory_.create(id, "rcot_receiver_traffic"));
    } else {
      receiverRcot = rcotFactory_->create(
          agentFactory_.create(id, "rcot_receiver_traffic"));
      senderRcot = rcotFactory_->create(
          delta, agentFactory_.create(id, "rcot_sender_traffic"));
    }

    return std::make_unique<RcotBasedBidirectionObliviousTransfer>(
        agentFactory_.create(id, "bidirection_ot_traffic"),
        delta,
        std::move(senderRcot),
        std::move(receiverRcot));
  }

 private:
  int myid_;
  communication::IPartyCommunicationAgentFactory& agentFactory_;
  std::unique_ptr<IRandomCorrelatedObliviousTransferFactory> rcotFactory_;
};

} // namespace fbpcf::engine::tuple_generator::oblivious_transfer
