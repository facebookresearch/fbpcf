/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <memory>

#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgent.h"

namespace fbpcf::mpc_framework::engine::communication {

/**
 * An communication factory API
 */
class IPartyCommunicationAgentFactory {
 public:
  virtual ~IPartyCommunicationAgentFactory() = default;

  /**
   * create an agent that talks to a certain party.
   */
  virtual std::unique_ptr<IPartyCommunicationAgent> create(int id) = 0;
};

} // namespace fbpcf::mpc_framework::engine::communication
