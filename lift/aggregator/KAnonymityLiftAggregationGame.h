#pragma once

#include <memory>
#include <vector>

#include <folly/logging/xlog.h>

#include "../../pcf/common/FunctionalUtil.h"
#include "../../pcf/common/VectorUtil.h"
#include "../../pcf/mpc/EmpGame.h"
#include "../common/GroupedLiftMetrics.h"
#include "../common/GroupedEncryptedLiftMetrics.h"
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
      pcf::Visibility visibility = pcf::Visibility::Public)
      : pcf::EmpGame<
            IOChannel,
            std::vector<GroupedLiftMetrics>,
            GroupedLiftMetrics>(std::move(ioChannel), party),
        visibility_{visibility} {}

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
    std::vector<emp::Integer> v = pcf::functional::reduce<std::vector<emp::Integer>>(
        vv, pcf::vector::Add<emp::Integer>);

    XLOG(INFO) << "Applying k-anonymity...";
    // Applies k-anonymity with k = 100
    auto anonymized = kAnonymizeGrouped(v);

    XLOG(INFO) << "Revealing metrics...";
    XLOGF(DBG, "Visibility: {}", this->visibility_);
    // Reveal aggregated metrics
    auto revealed = pcf::functional::map<emp::Integer, int64_t>(
        anonymized, [this](const emp::Integer& i) {
          return i.reveal<int64_t>(static_cast<int>(this->visibility_));
        });
    return mapVectorToGroupedLiftMetrics(revealed);
  }

 private:
  pcf::Visibility visibility_;

  std::vector<emp::Integer> kAnonymizeGrouped(
      std::vector<emp::Integer> metrics) {
    auto groupedMetrics = mapVectorToGroupedLiftMetrics(metrics);
    GroupedEncryptedLiftMetrics anonymizedMetrics;

    anonymizedMetrics.metrics = kAnonymizeMetrics(groupedMetrics.metrics);
    for (auto group : groupedMetrics.subGroupMetrics)
    {
        anonymizedMetrics.subGroupMetrics.push_back(kAnonymizeMetrics(group));
    }

    return mapGroupedLiftMetricsToEmpVector(anonymizedMetrics);
  }

  EncryptedLiftMetrics kAnonymizeMetrics(EncryptedLiftMetrics metrics) {
    const emp::Integer hiddenMetric{INT_SIZE, kHiddenMetricConstant, emp::PUBLIC};
    const emp::Integer kAnonymityLevel{
        INT_SIZE, kAnonymityThreshold, emp::PUBLIC};
    auto condition =
        metrics.testBuyers + metrics.controlBuyers >= kAnonymityLevel;

    EncryptedLiftMetrics anonymized {};

    anonymized.idMatch = emp::If(condition, metrics.idMatch, hiddenMetric);
    anonymized.testPopulation = metrics.testPopulation;
    anonymized.controlPopulation = metrics.controlPopulation;
    anonymized.testBuyers = emp::If(condition, metrics.testBuyers, hiddenMetric);
    anonymized.controlBuyers = emp::If(condition, metrics.controlBuyers, hiddenMetric);
    anonymized.testSales = emp::If(condition, metrics.testSales, hiddenMetric);
    anonymized.controlSales = emp::If(condition, metrics.controlSales, hiddenMetric);
    anonymized.testSquared = emp::If(condition, metrics.testSquared, hiddenMetric);
    anonymized.controlSquared = emp::If(condition, metrics.controlSquared, hiddenMetric);

    return anonymized;
  }
};
} // namespace private_lift
