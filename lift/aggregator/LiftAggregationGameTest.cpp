/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "LiftAggregationGame.h"

#include <memory>
#include <thread>
#include <vector>

#include <emp-sh2pc/emp-sh2pc.h>
#include <gtest/gtest.h>

#include "folly/Random.h"

#include "../../pcf/common/VectorUtil.h"
#include "../../pcf/mpc/EmpGame.h"
#include "../../pcf/mpc/EmpTestUtil.h"
#include "../../pcf/mpc/QueueIO.h"
#include "../common/GroupedLiftMetrics.h"

namespace private_lift {
class LiftAggregationGameTest : public ::testing::Test {
 private:
  LiftMetrics fakeLiftMetrics() {
    auto r = []() { return folly::Random::rand32(); };
    return LiftMetrics{r(), r(), r(), r(), r(), r(), r(), r(), r(), r(), r(), r(), r(), r(), r(), r()};
  }

  GroupedLiftMetrics fakeGroupedMetrics() {
    return GroupedLiftMetrics{fakeLiftMetrics(),
                              {fakeLiftMetrics(), fakeLiftMetrics()}};
  }

  std::vector<GroupedLiftMetrics> fakeMetricsVector() {
    return std::vector<GroupedLiftMetrics>{
        fakeGroupedMetrics(), fakeGroupedMetrics(), fakeGroupedMetrics()};
  }

 protected:
  void SetUp() override {
    metricsVector_ = fakeMetricsVector();
    metricsVector_Alice_ = fakeMetricsVector();
    metricsVector_Bob_ = pcf::vector::Xor(metricsVector_, metricsVector_Alice_);
    expectedAggregatedMetrics_ = pcf::functional::reduce<GroupedLiftMetrics>(
        metricsVector_, [](const auto& a, const auto& b) { return a + b; });
  }

  std::vector<GroupedLiftMetrics> metricsVector_;
  std::vector<GroupedLiftMetrics> metricsVector_Alice_;
  std::vector<GroupedLiftMetrics> metricsVector_Bob_;
  GroupedLiftMetrics expectedAggregatedMetrics_;
};

TEST_F(LiftAggregationGameTest, TestRandomMetrics) {
  auto res = pcf::mpc::test<
      LiftAggregationGame<pcf::QueueIO>,
      std::vector<GroupedLiftMetrics>,
      GroupedLiftMetrics>(metricsVector_Alice_, metricsVector_Bob_);

  EXPECT_EQ(expectedAggregatedMetrics_, res.first);
  EXPECT_EQ(expectedAggregatedMetrics_, res.second);
}
} // namespace private_lift
