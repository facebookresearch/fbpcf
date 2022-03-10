/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <assert.h>
#include <algorithm>

#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/IBidirectionObliviousTransferFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/IRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/RcotBasedBidirectionObliviousTransfer.h"
#include "fbpcf/mpc_framework/engine/util/util.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer {

template <class T>
class RcotBasedBidirectionObliviousTransferFactory final
    : public IBidirectionObliviousTransferFactory<T> {
 public:
  RcotBasedBidirectionObliviousTransferFactory(
      int myid,
      communication::IPartyCommunicationAgentFactory& agentFactory,
      std::unique_ptr<IRandomCorrelatedObliviousTransferFactory> rcotFactory)
      : myid_(myid),
        agentFactory_(agentFactory),
        rcotFactory_(std::move(rcotFactory)) {}

  std::unique_ptr<IBidirectionObliviousTransfer<T>> create(int id) override {
    __m128i delta = util::getRandomM128iFromSystemNoise();
    util::setLsbTo1(delta);

    std::unique_ptr<IRandomCorrelatedObliviousTransfer> senderRcot;
    std::unique_ptr<IRandomCorrelatedObliviousTransfer> receiverRcot;

    if (id < myid_) {
      senderRcot = rcotFactory_->create(delta, agentFactory_.create(id));
      receiverRcot = rcotFactory_->create(agentFactory_.create(id));
    } else {
      receiverRcot = rcotFactory_->create(agentFactory_.create(id));
      senderRcot = rcotFactory_->create(delta, agentFactory_.create(id));
    }

    return std::make_unique<RcotBasedBidirectionObliviousTransfer<T>>(
        agentFactory_.create(id),
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
