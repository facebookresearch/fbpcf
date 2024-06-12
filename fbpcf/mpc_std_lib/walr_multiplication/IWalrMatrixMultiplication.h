/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <vector>

#include "fbpcf/frontend/Bit.h"
#include "fbpcf/scheduler/IScheduler.h"
#include "fbpcf/util/IMetricRecorder.h"

namespace fbpcf::mpc_std_lib::walr {

/**
 * The metric recorder for the IWalrMatrixMultiplication class
 */
class WalrMatrixMultiplicationMetricRecorder
    : public fbpcf::util::IMetricRecorder {
 public:
  WalrMatrixMultiplicationMetricRecorder()
      : featuresSent_(0), featuresReceived_(0) {}

  void addFeaturesSent(uint64_t size) {
    featuresSent_ += size;
  }
  void addFeaturesReceived(uint64_t size) {
    featuresReceived_ += size;
  }

  folly::dynamic getMetrics() const override {
    return folly::dynamic::object("features_sent", featuresSent_.load())(
        "features_received", featuresReceived_.load());
  }

 protected:
  std::atomic_uint64_t featuresSent_;
  std::atomic_uint64_t featuresReceived_;
};

template <int schedulerId>
class IWalrMatrixMultiplication {
  using FixedPointType = uint64_t;

 public:
  virtual ~IWalrMatrixMultiplication() = default;
  /**
   * The API for the caller with features and label shares.
   * @param features: the feature matrix. Each element is a column vector of the
   * feature matrix.
   * @param labels: the label vector represented as a batch of Bits,
   * consisting of only (secret) boolean labels.
   * To make the shape compatible, one must have features.size() ==
   * labels.getBatchSize().
   * @return the product of feature matrix and the label vector.
   */
  std::vector<double> matrixVectorMultiplication(
      const std::vector<std::vector<double>>& features,
      const frontend::Bit<true, schedulerId, true>& labels) {
    auto rst = matrixVectorMultiplicationImpl(features, labels);
    return rst;
  }

  /**
   * The API for the caller with only label shares.
   * @param labels: the label vector consisting of only (secret) boolean labels.
   * @param dpNoise: the dp noise that would be imposed on the output.
   */
  void matrixVectorMultiplication(
      const frontend::Bit<true, schedulerId, true>& labels,
      const std::vector<double>& dpNoise) {
    matrixVectorMultiplicationImpl(labels, dpNoise);
  }

  /**
   * Get the total amount of traffic transmitted.
   * @return a pair of (sent, received) data in bytes.
   */
  std::pair<uint64_t, uint64_t> getTrafficStatistics() const {
    auto nonEngineTraffic = getNonEngineTrafficStatistics();
    return {
        engineTraffic_.first + nonEngineTraffic.first,
        engineTraffic_.second + nonEngineTraffic.second};
  }

  /**
   * Get the total amount of non-engine traffic transmitted.
   * @return a pair of (sent, received) data in bytes.
   */
  virtual std::pair<uint64_t, uint64_t> getNonEngineTrafficStatistics()
      const = 0;

 protected:
  // The implementation API for the caller with feature and label shares.
  virtual std::vector<double> matrixVectorMultiplicationImpl(
      const std::vector<std::vector<double>>& features,
      const frontend::Bit<true, schedulerId, true>& labels) const = 0;

  // The implementation API for the caller with only label shares.
  virtual void matrixVectorMultiplicationImpl(
      const frontend::Bit<true, schedulerId, true>& labels,
      const std::vector<double>& dpNoise) const = 0;

 private:
  std::pair<uint64_t, uint64_t> engineTraffic_{0, 0};
};

} // namespace fbpcf::mpc_std_lib::walr
