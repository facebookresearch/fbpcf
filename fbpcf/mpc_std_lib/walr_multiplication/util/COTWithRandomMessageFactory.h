/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emmintrin.h>
#include <memory>
#include "fbpcf/engine/tuple_generator/oblivious_transfer/IRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/mpc_std_lib/walr_multiplication/util/COTWithRandomMessage.h"

namespace fbpcf::mpc_std_lib::walr::util {

class COTWithRandomMessageFactory {
 public:
  explicit COTWithRandomMessageFactory(
      std::unique_ptr<engine::tuple_generator::oblivious_transfer::
                          IRandomCorrelatedObliviousTransferFactory>
          rcotFactory)
      : rcotFactory_(std::move(rcotFactory)) {}

  // create sender
  std::unique_ptr<COTWithRandomMessage> create(
      __m128i delta,
      std::unique_ptr<engine::communication::IPartyCommunicationAgent> agent,
      std::unique_ptr<engine::communication::IPartyCommunicationAgent>
          rcotAgent) {
    auto rcot = rcotFactory_->create(delta, std::move(rcotAgent));
    return std::make_unique<COTWithRandomMessage>(
        delta, std::move(agent), std::move(rcot));
  }

  // create receiver
  std::unique_ptr<COTWithRandomMessage> create(
      std::unique_ptr<engine::communication::IPartyCommunicationAgent> agent,
      std::unique_ptr<engine::communication::IPartyCommunicationAgent>
          rcotAgent) {
    auto rcot = rcotFactory_->create(std::move(rcotAgent));
    return std::make_unique<COTWithRandomMessage>(
        std::move(agent), std::move(rcot));
  }

 private:
  std::unique_ptr<engine::tuple_generator::oblivious_transfer::
                      IRandomCorrelatedObliviousTransferFactory>
      rcotFactory_;
};
} // namespace fbpcf::mpc_std_lib::walr::util
