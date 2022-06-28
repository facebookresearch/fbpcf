/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include "fbpcf/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/mpc_std_lib/oram/DummySinglePointArrayGenerator.h"
#include "fbpcf/mpc_std_lib/oram/ISinglePointArrayGeneratorFactory.h"

namespace fbpcf::mpc_std_lib::oram::insecure {

class DummySinglePointArrayGeneratorFactory final
    : public ISinglePointArrayGeneratorFactory {
 public:
  DummySinglePointArrayGeneratorFactory(
      bool thisPartyToSetIndicator,
      int32_t peerId,
      engine::communication::IPartyCommunicationAgentFactory& factory)
      : thisPartyToSetIndicator_(thisPartyToSetIndicator),
        peerId_(peerId),
        factory_(factory) {}

  std::unique_ptr<ISinglePointArrayGenerator> create() override {
    return std::make_unique<DummySinglePointArrayGenerator>(
        thisPartyToSetIndicator_,
        factory_.create(peerId_, "dummy_single_point_array_generator_traffic"));
  }

 private:
  bool thisPartyToSetIndicator_;
  int32_t peerId_;
  engine::communication::IPartyCommunicationAgentFactory& factory_;
};

} // namespace fbpcf::mpc_std_lib::oram::insecure
