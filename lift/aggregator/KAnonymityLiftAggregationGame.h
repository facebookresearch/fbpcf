/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <vector>

#include <folly/logging/xlog.h>

#include "../../pcf/common/FunctionalUtil.h"
#include "../../pcf/common/VectorUtil.h"
#include "../../pcf/mpc/EmpGame.h"
#include "../common/GroupedEncryptedLiftMetrics.h"
#include "../common/GroupedLiftMetrics.h"
#include "MetricsMapper.h"

namespace private_lift {

constexpr int64_t INT_SIZE = 64;

template <class IOChannel>
class KAnonymityLiftAggregationGame : public pcf::EmpGame<
                                          IOChannel,
                                          std::vector<GroupedLiftMetrics>,
                                          GroupedLiftMetrics> {
 public:
  KAnonymityLiftAggregationGame(
      std::unique_ptr<IOChannel> ioChannel,
      pcf::Party party,
      pcf::Visibility visibility = pcf::Visibility::Public,
      int64_t threshold = kAnonymityThreshold)
      : pcf::EmpGame<
            IOChannel,
            std::vector<GroupedLiftMetrics>,
            GroupedLiftMetrics>(std::move(ioChannel), party),
        visibility_{visibility},
        threshold_{threshold} {}

  static constexpr int64_t kHiddenMetricConstant = -1;
  static constexpr int64_t kAnonymityThreshold = 100;

  GroupedLiftMetrics play(
      const std::vector<GroupedLiftMetrics>& inputData) override {
    XLOG(INFO) << "Decoding metrics...";
    // XOR all metrics, return std::vector<std::vector<emp::Integer>>
    auto vv =
        pcf::functional::map<GroupedLiftMetrics, std::vector<emp::Integer>>(
            inputData, [](const auto& metrics) {
              auto empVector = mapGroupedLiftMetricsToEmpVector(metrics);
              return empVector.map(
                  [](const auto& x, const auto& y) { return x ^ y; });
            });

    XLOG(INFO) << "Aggregating metrics...";
    // Aggregate all metrics, returns std::vector<emp::Integer>
    std::vector<emp::Integer> v =
        pcf::functional::reduce<std::vector<emp::Integer>>(
            vv, pcf::vector::Add<emp::Integer>);
    XLOGF(INFO, "Applying k-anonymity threshold {}...", threshold_);
    auto anonymized = kAnonymizeGrouped(v);

    XLOG(INFO) << "Revealing metrics...";
    XLOGF(DBG, "Visibility: {}", this->visibility_);
    // Reveal aggregated metrics
    auto revealed = pcf::functional::map<emp::Integer, int64_t>(
        anonymized, [visibility = visibility_](const emp::Integer& i) {
          return i.reveal<int64_t>(static_cast<int>(visibility));
        });
    return mapVectorToGroupedLiftMetrics(revealed);
  }

 private:
  pcf::Visibility visibility_;
  int64_t threshold_;

  std::vector<emp::Integer> kAnonymizeGrouped(
      std::vector<emp::Integer> metrics) {
    auto groupedMetrics = mapVectorToGroupedLiftMetrics(metrics);
    GroupedEncryptedLiftMetrics anonymizedMetrics;

    anonymizedMetrics.metrics = kAnonymizeMetrics(groupedMetrics.metrics);
    for (auto group : groupedMetrics.cohortMetrics) {
      anonymizedMetrics.cohortMetrics.push_back(kAnonymizeMetrics(group));
    }

    return mapGroupedLiftMetricsToEmpVector(anonymizedMetrics);
  }

  EncryptedLiftMetrics kAnonymizeMetrics(EncryptedLiftMetrics metrics) {
    const emp::Integer hiddenMetric{
        INT_SIZE, kHiddenMetricConstant, emp::PUBLIC};
    const emp::Integer kAnonymityLevel{INT_SIZE, threshold_, emp::PUBLIC};
    auto condition =
        metrics.testConverters + metrics.controlConverters >= kAnonymityLevel;

    EncryptedLiftMetrics anonymized{};

    anonymized.testPopulation = metrics.testPopulation;
    anonymized.controlPopulation = metrics.controlPopulation;
    anonymized.testConversions =
        emp::If(condition, metrics.testConversions, hiddenMetric);
    anonymized.controlConversions =
        emp::If(condition, metrics.controlConversions, hiddenMetric);
    anonymized.testConverters =
        emp::If(condition, metrics.testConverters, hiddenMetric);
    anonymized.controlConverters =
        emp::If(condition, metrics.controlConverters, hiddenMetric);
    anonymized.testValue = emp::If(condition, metrics.testValue, hiddenMetric);
    anonymized.controlValue =
        emp::If(condition, metrics.controlValue, hiddenMetric);
    anonymized.testValueSquared =
        emp::If(condition, metrics.testValueSquared, hiddenMetric);
    anonymized.controlValueSquared =
        emp::If(condition, metrics.controlValueSquared, hiddenMetric);
    anonymized.testNumConvSquared =
        emp::If(condition, metrics.testNumConvSquared, hiddenMetric);
    anonymized.controlNumConvSquared =
        emp::If(condition, metrics.controlNumConvSquared, hiddenMetric);
    anonymized.testMatchCount =
        emp::If(condition, metrics.testMatchCount, hiddenMetric);
    anonymized.controlMatchCount =
        emp::If(condition, metrics.controlMatchCount, hiddenMetric);
    anonymized.testImpressions =
        emp::If(condition, metrics.testImpressions, hiddenMetric);
    anonymized.controlImpressions =
        emp::If(condition, metrics.controlImpressions, hiddenMetric);
    anonymized.testClicks =
        emp::If(condition, metrics.testClicks, hiddenMetric);
    anonymized.controlClicks =
        emp::If(condition, metrics.controlClicks, hiddenMetric);
    anonymized.testSpend = emp::If(condition, metrics.testSpend, hiddenMetric);
    anonymized.controlSpend =
        emp::If(condition, metrics.controlSpend, hiddenMetric);
    anonymized.testReach = emp::If(condition, metrics.testReach, hiddenMetric);
    anonymized.controlReach =
        emp::If(condition, metrics.controlReach, hiddenMetric);
    anonymized.testClickers =
        emp::If(condition, metrics.testClickers, hiddenMetric);
    anonymized.controlClickers =
        emp::If(condition, metrics.controlClickers, hiddenMetric);

    return anonymized;
  }
};
} // namespace private_lift
