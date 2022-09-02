/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emmintrin.h>
#include <cstdint>
#include "fbpcf/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/engine/util/IPrgFactory.h"
#include "fbpcf/engine/util/util.h"
#include "fbpcf/mpc_std_lib/walr_multiplication/IWalrMatrixMultiplication.h"
#include "fbpcf/mpc_std_lib/walr_multiplication/IWalrMatrixMultiplicationFactory.h"
#include "fbpcf/mpc_std_lib/walr_multiplication/OTBasedMatrixMultiplication.h"
#include "fbpcf/mpc_std_lib/walr_multiplication/util/COTWithRandomMessage.h"
#include "fbpcf/mpc_std_lib/walr_multiplication/util/COTWithRandomMessageFactory.h"

namespace fbpcf::mpc_std_lib::walr {

template <int schedulerId, typename FixedPointType>
class OTBasedMatrixMultiplicationFactory final
    : public IWalrMatrixMultiplicationFactory<schedulerId> {
 public:
  explicit OTBasedMatrixMultiplicationFactory(
      int myId,
      int partnerId,
      bool isFeatureOwner,
      uint64_t divisor,
      engine::communication::IPartyCommunicationAgentFactory& agentFactory,
      std::unique_ptr<engine::util::IPrgFactory> prgFactory,
      std::unique_ptr<util::COTWithRandomMessageFactory> cotWRMFactory)
      : myId_(myId),
        partnerId_(partnerId),
        isFeatureOwner_(isFeatureOwner),
        divisor_(divisor),
        agentFactory_(agentFactory),
        prgFactory_(std::move(prgFactory)),
        cotWRMFactory_(std::move(cotWRMFactory)) {}

  std::unique_ptr<IWalrMatrixMultiplication<schedulerId>> create() override {
    __m128i delta = engine::util::getRandomM128iFromSystemNoise();
    std::unique_ptr<util::COTWithRandomMessage> cotWRM;
    auto cotWRMAgent = agentFactory_.create(
        partnerId_,
        "walr_matrix_multiplication_cotWRM_traffic_to_party " +
            std::to_string(partnerId_));
    auto rcotAgent = agentFactory_.create(
        partnerId_,
        "walr_matrix_multiplication_rcot_of_cotWRM_traffic_to_party " +
            std::to_string(partnerId_));
    if (isFeatureOwner_) {
      cotWRM = cotWRMFactory_->create(
          delta, std::move(cotWRMAgent), std::move(rcotAgent));
    } else {
      cotWRM =
          cotWRMFactory_->create(std::move(cotWRMAgent), std::move(rcotAgent));
    }

    return std::make_unique<
        OTBasedMatrixMultiplication<schedulerId, FixedPointType>>(
        myId_,
        partnerId_,
        isFeatureOwner_,
        divisor_,
        agentFactory_.create(
            partnerId_,
            "walr_matrix_multiplication_traffic_to_party " +
                std::to_string(partnerId_)),
        std::move(prgFactory_),
        std::move(cotWRM));
  }

 private:
  int myId_;
  int partnerId_;
  bool isFeatureOwner_;
  uint64_t divisor_;
  engine::communication::IPartyCommunicationAgentFactory& agentFactory_;
  std::unique_ptr<engine::util::IPrgFactory> prgFactory_;
  std::unique_ptr<util::COTWithRandomMessageFactory> cotWRMFactory_;
};

} // namespace fbpcf::mpc_std_lib::walr
