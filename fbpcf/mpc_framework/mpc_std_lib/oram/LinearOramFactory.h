/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <stdexcept>
#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/mpc_framework/engine/util/AesPrgFactory.h"
#include "fbpcf/mpc_framework/engine/util/IPrgFactory.h"
#include "fbpcf/mpc_framework/engine/util/util.h"
#include "fbpcf/mpc_framework/mpc_std_lib/oram/IWriteOnlyOramFactory.h"
#include "fbpcf/mpc_framework/mpc_std_lib/oram/LinearOram.h"

namespace fbpcf::mpc_framework::mpc_std_lib::oram {

template <typename T, int schedulerId>
class LinearOramFactory final : public IWriteOnlyOramFactory<T> {
 public:
  LinearOramFactory(
      typename IWriteOnlyOram<T>::Role myRole,
      int32_t myId,
      int32_t peerId,
      engine::communication::IPartyCommunicationAgentFactory& agentFactory,
      std::unique_ptr<engine::util::IPrgFactory> prgFactory)
      : myRole_(myRole),
        peerId_(peerId),
        agentFactory_(agentFactory),
        prgFactory_(std::move(prgFactory)) {
    party0Id_ = myRole_ == IWriteOnlyOram<T>::Role::Alice ? myId : peerId_;
    party1Id_ = myRole_ == IWriteOnlyOram<T>::Role::Bob ? myId : peerId_;
  }

  std::unique_ptr<IWriteOnlyOram<T>> create(size_t size) override {
    return std::make_unique<LinearOram<T, schedulerId>>(
        size,
        myRole_,
        party0Id_,
        party1Id_,
        agentFactory_.create(peerId_),
        prgFactory_->create(engine::util::getRandomM128iFromSystemNoise()));
  }

  /**
   * @inherit doc
   */
  uint32_t getMaxBatchSize(size_t /* size */, uint8_t /* concurrency */)
      override {
    return 65536; // this is a very arbitrary number.
  }

 private:
  typename IWriteOnlyOram<T>::Role myRole_;
  int32_t peerId_;
  int32_t party0Id_;
  int32_t party1Id_;
  engine::communication::IPartyCommunicationAgentFactory& agentFactory_;
  std::unique_ptr<engine::util::IPrgFactory> prgFactory_;
};

template <typename T, int schedulerId>
std::unique_ptr<IWriteOnlyOramFactory<T>> getSecureLinearOramFactory(
    bool amIParty0,
    int32_t party0Id,
    int32_t party1Id,
    engine::communication::IPartyCommunicationAgentFactory& factory) {
  return std::make_unique<
      fbpcf::mpc_framework::mpc_std_lib::oram::LinearOramFactory<
          fbpcf::mpc_framework::mpc_std_lib::util::AggregationValue,
          schedulerId>>(
      amIParty0 ? IWriteOnlyOram<T>::Role::Alice : IWriteOnlyOram<T>::Role::Bob,
      amIParty0 ? party0Id : party0Id,
      amIParty0 ? party1Id : party0Id,
      factory,
      std::make_unique<fbpcf::mpc_framework::engine::util::AesPrgFactory>());
}

} // namespace fbpcf::mpc_framework::mpc_std_lib::oram
