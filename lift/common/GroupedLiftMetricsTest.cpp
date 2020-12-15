#include <gtest/gtest.h>

#include "folly/Random.h"

#include "GroupedLiftMetrics.h"

namespace private_lift {
class GroupedLiftMetricsTest : public ::testing::Test {
 private:
  LiftMetrics fakeLiftMetrics() {
    auto r = []() { return folly::Random::rand32(); };
    return LiftMetrics{r(), r(), r(), r(), r(), r(), r(), r(), r()};
  }

 protected:
  void SetUp() override {
    groupedMetrics_ = GroupedLiftMetrics{
        fakeLiftMetrics(), {fakeLiftMetrics(), fakeLiftMetrics()}};
  }

  GroupedLiftMetrics groupedMetrics_;
};

TEST_F(GroupedLiftMetricsTest, GroupedLiftMetrics) {
  auto json = groupedMetrics_.toJson();
  auto parsedMetrics = GroupedLiftMetrics::fromJson(json);
  EXPECT_EQ(groupedMetrics_.metrics, parsedMetrics.metrics);
  EXPECT_EQ(groupedMetrics_.subGroupMetrics, parsedMetrics.subGroupMetrics);

  // Modify the parsed metrics so they are no longer equal.
  parsedMetrics.metrics.controlBuyers++;
  parsedMetrics.subGroupMetrics[0].controlBuyers++;
  EXPECT_NE(groupedMetrics_.metrics, parsedMetrics.metrics);
  EXPECT_NE(groupedMetrics_.subGroupMetrics, parsedMetrics.subGroupMetrics);
}
} // namespace private_lift
