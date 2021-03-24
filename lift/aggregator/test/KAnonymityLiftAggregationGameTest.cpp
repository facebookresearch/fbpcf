/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>
#include <thread>
#include <vector>

#include <emp-sh2pc/emp-sh2pc.h>
#include <gtest/gtest.h>

#include "folly/Random.h"

#include "../../../pcf/common/VectorUtil.h"
#include "../../../pcf/mpc/EmpGame.h"
#include "../../../pcf/mpc/EmpTestUtil.h"
#include "../../../pcf/mpc/QueueIO.h"
#include "../../common/GroupedLiftMetrics.h"
#include "../KAnonymityLiftAggregationGame.h"

namespace private_lift {

// nullifyNonExposedMetrics is a small helper function to nullify specific
// metrics in a GroupLiftMetrics (used for generated test data)
void nullifyNonExposedMetrics(GroupedLiftMetrics& groupMetrics) {
  // Set the nullable metrics to -1
  groupMetrics.metrics.controlSquared = -1;
  groupMetrics.metrics.testSquared = -1;
  for (auto& subGroup : groupMetrics.subGroupMetrics) {
    subGroup.testSquared = -1;
    subGroup.controlSquared = -1;
  }
}

class KAnonymityLiftAggregationGameTest : public ::testing::Test {
 protected:
  LiftMetrics fakeLiftMetrics(bool lowPop) {
    auto r = lowPop ? []() { return folly::Random::rand32(0, 5); } : []() {
      return folly::Random::rand32(
          KAnonymityLiftAggregationGame<pcf::QueueIO>::kAnonymityThreshold,
          std::numeric_limits<uint32_t>::max());
    };
    return LiftMetrics{
        r(),
        r(),
        r(),
        r(),
        r(),
        r(),
        r(),
        r(),
        r(),
        r(),
        r(),
        r(),
        r(),
        r(),
        r(),
        r(),
        r(),
        r()};
  }

  GroupedLiftMetrics fakeGroupedMetrics(bool allLowPop, bool subgroupLowPop) {
    return GroupedLiftMetrics{
        fakeLiftMetrics(allLowPop),
        {fakeLiftMetrics(allLowPop || subgroupLowPop),
         fakeLiftMetrics(allLowPop || subgroupLowPop)}};
  }

  std::vector<GroupedLiftMetrics> fakeMetricsVector(
      bool allLowPop = false,
      bool subgroupLowPop = false) {
    return std::vector<GroupedLiftMetrics>{
        fakeGroupedMetrics(allLowPop, subgroupLowPop),
        fakeGroupedMetrics(allLowPop, subgroupLowPop),
        fakeGroupedMetrics(allLowPop, subgroupLowPop)};
  }

  LiftMetrics filteredMetric(LiftMetrics metrics) {
    if (metrics.controlConverters + metrics.testConverters >=
        KAnonymityLiftAggregationGame<pcf::QueueIO>::kAnonymityThreshold)
      return metrics;

    return LiftMetrics{
        metrics.testPopulation,
        metrics.controlPopulation,
        KAnonymityLiftAggregationGame<pcf::QueueIO>::kHiddenMetricConstant,
        KAnonymityLiftAggregationGame<pcf::QueueIO>::kHiddenMetricConstant,
        KAnonymityLiftAggregationGame<pcf::QueueIO>::kHiddenMetricConstant,
        KAnonymityLiftAggregationGame<pcf::QueueIO>::kHiddenMetricConstant,
        KAnonymityLiftAggregationGame<pcf::QueueIO>::kHiddenMetricConstant,
        KAnonymityLiftAggregationGame<pcf::QueueIO>::kHiddenMetricConstant,
        KAnonymityLiftAggregationGame<pcf::QueueIO>::kHiddenMetricConstant,
        KAnonymityLiftAggregationGame<pcf::QueueIO>::kHiddenMetricConstant,
        KAnonymityLiftAggregationGame<pcf::QueueIO>::kHiddenMetricConstant,
        KAnonymityLiftAggregationGame<pcf::QueueIO>::kHiddenMetricConstant,
        KAnonymityLiftAggregationGame<pcf::QueueIO>::kHiddenMetricConstant,
        KAnonymityLiftAggregationGame<pcf::QueueIO>::kHiddenMetricConstant,
        KAnonymityLiftAggregationGame<pcf::QueueIO>::kHiddenMetricConstant,
        KAnonymityLiftAggregationGame<pcf::QueueIO>::kHiddenMetricConstant,
        KAnonymityLiftAggregationGame<pcf::QueueIO>::kHiddenMetricConstant,
        KAnonymityLiftAggregationGame<pcf::QueueIO>::kHiddenMetricConstant};
  }

  void SetUp() override {
    metricsVector_ = fakeMetricsVector();
    metricsVector_Alice_ = fakeMetricsVector();
    metricsVector_Bob_ = pcf::vector::Xor(metricsVector_, metricsVector_Alice_);
    expectedAggregatedMetrics_ = pcf::functional::reduce<GroupedLiftMetrics>(
        metricsVector_, [](const auto& a, const auto& b) { return a + b; });
    // Set the nullable metrics to -1 in the "expected" results
    nullifyNonExposedMetrics(expectedAggregatedMetrics_);
  }

  std::vector<GroupedLiftMetrics> metricsVector_;
  std::vector<GroupedLiftMetrics> metricsVector_Alice_;
  std::vector<GroupedLiftMetrics> metricsVector_Bob_;
  GroupedLiftMetrics expectedAggregatedMetrics_;
};

TEST_F(KAnonymityLiftAggregationGameTest, TestRandomMetrics) {
  auto res = pcf::mpc::test<
      KAnonymityLiftAggregationGame<pcf::QueueIO>,
      std::vector<GroupedLiftMetrics>,
      GroupedLiftMetrics>(metricsVector_Alice_, metricsVector_Bob_);

  EXPECT_EQ(expectedAggregatedMetrics_, res.first);
  EXPECT_EQ(expectedAggregatedMetrics_, res.second);
}

TEST_F(KAnonymityLiftAggregationGameTest, TestAllLowPopulationMetrics) {
  auto metricsVector = fakeMetricsVector(true);
  auto metricsVector_Alice = fakeMetricsVector(true);
  auto metricsVector_Bob = pcf::vector::Xor(metricsVector, metricsVector_Alice);
  auto unfilteredAggregatedMetrics =
      pcf::functional::reduce<GroupedLiftMetrics>(
          metricsVector, [](const auto& a, const auto& b) { return a + b; });
  nullifyNonExposedMetrics(unfilteredAggregatedMetrics);

  auto res = pcf::mpc::test<
      KAnonymityLiftAggregationGame<pcf::QueueIO>,
      std::vector<GroupedLiftMetrics>,
      GroupedLiftMetrics>(metricsVector_Alice, metricsVector_Bob);

  // If the subgroups and the overall metrics are small, the metrics
  // and subgroup metrics should all be negative.
  for (int i = 0; i < unfilteredAggregatedMetrics.subGroupMetrics.size(); ++i) {
    EXPECT_EQ(
        filteredMetric(unfilteredAggregatedMetrics.subGroupMetrics[i]),
        res.first.subGroupMetrics[i]);
    EXPECT_EQ(
        filteredMetric(unfilteredAggregatedMetrics.subGroupMetrics[i]),
        res.second.subGroupMetrics[i]);
  }
  EXPECT_EQ(
      filteredMetric(unfilteredAggregatedMetrics.metrics), res.first.metrics);
  EXPECT_EQ(
      filteredMetric(unfilteredAggregatedMetrics.metrics), res.second.metrics);
}

TEST_F(KAnonymityLiftAggregationGameTest, TestSubgroupLowPopulationMetrics) {
  auto metricsVector = fakeMetricsVector(false, true);
  auto metricsVector_Alice = fakeMetricsVector(false, true);
  auto metricsVector_Bob = pcf::vector::Xor(metricsVector, metricsVector_Alice);
  auto unfilteredAggregatedMetrics =
      pcf::functional::reduce<GroupedLiftMetrics>(
          metricsVector, [](const auto& a, const auto& b) { return a + b; });
  nullifyNonExposedMetrics(unfilteredAggregatedMetrics);
  auto res = pcf::mpc::test<
      KAnonymityLiftAggregationGame<pcf::QueueIO>,
      std::vector<GroupedLiftMetrics>,
      GroupedLiftMetrics>(metricsVector_Alice, metricsVector_Bob);

  // If the subgroups are small but the overall metrics are not, the metrics
  // should be unfiltered and the subgroups should be filtered.
  for (int i = 0; i < unfilteredAggregatedMetrics.subGroupMetrics.size(); ++i) {
    EXPECT_EQ(
        filteredMetric(unfilteredAggregatedMetrics.subGroupMetrics[i]),
        res.first.subGroupMetrics[i]);
    EXPECT_EQ(
        filteredMetric(unfilteredAggregatedMetrics.subGroupMetrics[i]),
        res.second.subGroupMetrics[i]);
  }
  EXPECT_EQ(unfilteredAggregatedMetrics.metrics, res.first.metrics);
  EXPECT_EQ(unfilteredAggregatedMetrics.metrics, res.second.metrics);
}

} // namespace private_lift
