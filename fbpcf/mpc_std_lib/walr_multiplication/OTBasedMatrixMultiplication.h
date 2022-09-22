/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emmintrin.h>
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include "fbpcf/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/IRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/engine/util/IPrgFactory.h"
#include "fbpcf/engine/util/util.h"
#include "fbpcf/mpc_std_lib/walr_multiplication/IWalrMatrixMultiplication.h"
#include "fbpcf/mpc_std_lib/walr_multiplication/util/COTWithRandomMessage.h"
#include "fbpcf/mpc_std_lib/walr_multiplication/util/NumberMapper.h"

namespace fbpcf::mpc_std_lib::walr {

/**
 * This object is a metric recorder
 */
class OTBasedMatrixMultiplicationMetricRecorder final
    : public WalrMatrixMultiplicationMetricRecorder {
 public:
  OTBasedMatrixMultiplicationMetricRecorder()
      : otMessagesUsed_(0), columnsSent_(0), columnsReceived_(0) {}

  void addOtMessagesUsed(uint64_t size) {
    otMessagesUsed_ += size;
  }

  void addColumnsSent(uint64_t size) {
    columnsSent_ += size;
  }

  void addColumnsReceived(uint64_t size) {
    columnsReceived_ += size;
  }

  folly::dynamic getMetrics() const override {
    return folly::dynamic::object("ot_messages_used", otMessagesUsed_.load())(
        "columns_sent", columnsSent_.load())(
        "columns_received", columnsReceived_.load())(
        "features_sent", featuresSent_.load())(
        "features_received", featuresReceived_.load());
  }

 private:
  std::atomic_uint64_t otMessagesUsed_;
  std::atomic_uint64_t columnsSent_;
  std::atomic_uint64_t columnsReceived_;
};

template <int schedulerId, typename FixedPointType>
class OTBasedMatrixMultiplication final
    : public IWalrMatrixMultiplication<schedulerId> {
 public:
  explicit OTBasedMatrixMultiplication(
      int myId,
      int partnerId,
      bool isFeatureOwner,
      uint64_t divisor, // The precision loss will be roughly 1 / divisor
      std::unique_ptr<engine::communication::IPartyCommunicationAgent> agent,
      std::unique_ptr<engine::util::IPrgFactory> prgFactory,
      std::unique_ptr<util::COTWithRandomMessage> cotWRM,
      std::shared_ptr<OTBasedMatrixMultiplicationMetricRecorder> recorder)
      : myId_(myId),
        partnerId_(partnerId),
        isFeatureOwner_(isFeatureOwner),
        numberMapper_(util::NumberMapper<FixedPointType>(divisor)),
        agent_(std::move(agent)),
        prgFactory_(std::move(prgFactory)),
        cotWRM_(std::move(cotWRM)),
        recorder_(recorder) {}

  void setDivisor(uint64_t divisor) {
    numberMapper_.setDivisor(divisor);
  }

  std::pair<uint64_t, uint64_t> getNonEngineTrafficStatistics() const override {
    auto cotWRMTraffic = cotWRM_->getTrafficStatistics();
    auto otherTraffic = agent_->getTrafficStatistics();
    return {
        cotWRMTraffic.first + otherTraffic.first,
        cotWRMTraffic.second + otherTraffic.second};
  }

 protected:
  std::vector<double> matrixVectorMultiplicationImpl(
      const std::vector<std::vector<double>>& features,
      const frontend::Bit<true, schedulerId, true>& labels) const override;

  void matrixVectorMultiplicationImpl(
      const frontend::Bit<true, schedulerId, true>& labels,
      const std::vector<double>& dpNoise) const override;

 private:
  int myId_;
  int partnerId_;
  bool isFeatureOwner_;
  util::NumberMapper<FixedPointType> numberMapper_;
  std::unique_ptr<engine::communication::IPartyCommunicationAgent> agent_;
  std::unique_ptr<engine::util::IPrgFactory> prgFactory_;
  std::unique_ptr<util::COTWithRandomMessage> cotWRM_;
  std::shared_ptr<OTBasedMatrixMultiplicationMetricRecorder> recorder_;
};
} // namespace fbpcf::mpc_std_lib::walr

#include "fbpcf/mpc_std_lib/walr_multiplication/OTBasedMatrixMultiplication_impl.h"
