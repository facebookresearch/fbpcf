/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/mpc_std_lib/walr_multiplication/DummyMatrixMultiplication.h"
#include "fbpcf/mpc_std_lib/walr_multiplication/IWalrMatrixMultiplication.h"
#include "fbpcf/mpc_std_lib/walr_multiplication/IWalrMatrixMultiplicationFactory.h"

namespace fbpcf::mpc_std_lib::walr::insecure {

template <int schedulerId>
class DummyMatrixMultiplicationFactory final
    : public IWalrMatrixMultiplicationFactory<schedulerId> {
 public:
  explicit DummyMatrixMultiplicationFactory(
      int myId,
      int partnerId,
      engine::communication::IPartyCommunicationAgentFactory& agentFactory)
      : myId_(myId), partnerId_(partnerId), agentFactory_(agentFactory) {}

  std::unique_ptr<IWalrMatrixMultiplication<schedulerId>> create() override {
    auto recorder = std::make_shared<WalrMatrixMultiplicationMetricRecorder>();
    return std::make_unique<DummyMatrixMultiplication<schedulerId>>(
        myId_,
        partnerId_,
        agentFactory_.create(
            partnerId_,
            "walr_matrix_multiplication_traffic_to_party " +
                std::to_string(partnerId_)),
        recorder);
  }

 private:
  int myId_;
  int partnerId_;
  engine::communication::IPartyCommunicationAgentFactory& agentFactory_;
};

} // namespace fbpcf::mpc_std_lib::walr::insecure
