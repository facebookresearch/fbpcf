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

namespace fbpcf::mpc_std_lib::walr {

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
    // Initialize engine traffic recording
    auto initEngineTraffic =
        scheduler::SchedulerKeeper<schedulerId>::getTrafficStatistics();

    auto rst = matrixVectorMultiplicationImpl(features, labels);

    // Calculate engine traffic
    auto finalEngineTraffic =
        scheduler::SchedulerKeeper<schedulerId>::getTrafficStatistics();
    engineTraffic_.first += finalEngineTraffic.first - initEngineTraffic.first;
    engineTraffic_.second +=
        finalEngineTraffic.second - initEngineTraffic.second;

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
    // Initialize engine traffic recording
    auto initEngineTraffic =
        scheduler::SchedulerKeeper<schedulerId>::getTrafficStatistics();

    matrixVectorMultiplicationImpl(labels, dpNoise);

    // Calculate engine traffic
    auto finalEngineTraffic =
        scheduler::SchedulerKeeper<schedulerId>::getTrafficStatistics();
    engineTraffic_.first += finalEngineTraffic.first - initEngineTraffic.first;
    engineTraffic_.second +=
        finalEngineTraffic.second - initEngineTraffic.second;
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
