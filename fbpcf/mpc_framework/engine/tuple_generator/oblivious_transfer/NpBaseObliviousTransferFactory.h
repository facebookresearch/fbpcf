/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/IBaseObliviousTransferFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/NpBaseObliviousTransfer.h"

namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer {

/**
 * Create a base oblivious transfer with a particular party.
 * Some implementation may need party id to decide parties' roles in the
 * underlying protocol.
 */
class NpBaseObliviousTransferFactory final
    : public IBaseObliviousTransferFactory {
 public:
  explicit NpBaseObliviousTransferFactory() {}

  std::unique_ptr<IBaseObliviousTransfer> create(
      std::unique_ptr<communication::IPartyCommunicationAgent> agent) override {
    return std::make_unique<NpBaseObliviousTransfer>(std::move(agent));
  }
};

} // namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer
