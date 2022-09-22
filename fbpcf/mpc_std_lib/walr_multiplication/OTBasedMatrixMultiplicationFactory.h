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
  using IWalrMatrixMultiplicationFactory<schedulerId>::metricCollector_;

 public:
  const std::string metricRecorderNamePrefix = "ot_based_matrix_multiplication";

  // The following constructor will be deprecated once we updated all APP codes
  explicit OTBasedMatrixMultiplicationFactory(
      int myId,
      int partnerId,
      bool isFeatureOwner,
      uint64_t divisor,
      engine::communication::IPartyCommunicationAgentFactory& agentFactory,
      std::unique_ptr<engine::util::IPrgFactory> prgFactory,
      std::unique_ptr<util::COTWithRandomMessageFactory> cotWRMFactory)
      : IWalrMatrixMultiplicationFactory<schedulerId>(nullptr),
        myId_(myId),
        partnerId_(partnerId),
        isFeatureOwner_(isFeatureOwner),
        divisor_(divisor),
        agentFactory_(agentFactory),
        prgFactory_(std::move(prgFactory)),
        cotWRMFactory_(std::move(cotWRMFactory)) {}

  explicit OTBasedMatrixMultiplicationFactory(
      int myId,
      int partnerId,
      bool isFeatureOwner,
      uint64_t divisor,
      engine::communication::IPartyCommunicationAgentFactory& agentFactory,
      std::unique_ptr<engine::util::IPrgFactory> prgFactory,
      std::unique_ptr<util::COTWithRandomMessageFactory> cotWRMFactory,
      std::shared_ptr<fbpcf::util::MetricCollector> metricCollector)
      : IWalrMatrixMultiplicationFactory<schedulerId>(metricCollector),
        myId_(myId),
        partnerId_(partnerId),
        isFeatureOwner_(isFeatureOwner),
        divisor_(divisor),
        agentFactory_(agentFactory),
        prgFactory_(std::move(prgFactory)),
        cotWRMFactory_(std::move(cotWRMFactory)) {}

  std::unique_ptr<IWalrMatrixMultiplication<schedulerId>> create() override {
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
      __m128i delta = engine::util::getRandomM128iFromSystemNoise();
      engine::util::setLsbTo1(delta);
      cotWRM = cotWRMFactory_->create(
          delta, std::move(cotWRMAgent), std::move(rcotAgent));
    } else {
      cotWRM =
          cotWRMFactory_->create(std::move(cotWRMAgent), std::move(rcotAgent));
    }

    auto recorder =
        std::make_shared<OTBasedMatrixMultiplicationMetricRecorder>();

    // For backward compatibility we currently allow a null metric collector.
    // The condition will be removed later when we change all apps to use a
    // metric collector.
    if (metricCollector_ != nullptr) {
      std::string role = isFeatureOwner_ ? "feature_owner" : "label_owner";
      metricCollector_->addNewRecorder(
          metricRecorderNamePrefix + "_" + role, recorder);
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
        std::move(cotWRM),
        recorder);
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
