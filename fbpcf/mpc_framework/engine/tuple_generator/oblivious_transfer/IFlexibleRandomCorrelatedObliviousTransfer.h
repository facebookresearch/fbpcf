/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/IRandomCorrelatedObliviousTransfer.h"

namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer {

/**
 * A flexible RCOT allows to extract the communication it uses before destroy
 */

class IFlexibleRandomCorrelatedObliviousTransfer
    : public IRandomCorrelatedObliviousTransfer {
 public:
  virtual ~IFlexibleRandomCorrelatedObliviousTransfer() = default;

  virtual std::unique_ptr<communication::IPartyCommunicationAgent>
  extractCommunicationAgent() = 0;
};

} // namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer
