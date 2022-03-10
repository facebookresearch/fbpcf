/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/IBaseObliviousTransfer.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer {

/**
 * Create a base oblivious transfer with a particular party.
 * Some implementation may need party id to decide parties' roles in the
 * underlying protocol.
 */
class IBaseObliviousTransferFactory {
 public:
  virtual ~IBaseObliviousTransferFactory() = default;

  virtual std::unique_ptr<IBaseObliviousTransfer> create(
      std::unique_ptr<communication::IPartyCommunicationAgent> agent) = 0;
};

} // namespace fbpcf::engine::tuple_generator::oblivious_transfer
