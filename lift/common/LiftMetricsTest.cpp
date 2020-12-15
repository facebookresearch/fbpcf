#include "LiftMetrics.h"
#include <gtest/gtest.h>
#include "folly/Random.h"

namespace private_lift {
class LiftMetricsTest : public ::testing::Test {
 private:
  LiftMetrics fakeLiftMetrics() {
    auto r = []() { return folly::Random::rand32(); };
    return LiftMetrics{r(), r(), r(), r(), r(), r(), r(), r(), r()};
  }

 protected:
  void SetUp() override {
    metrics_ = fakeLiftMetrics();
  }

  LiftMetrics metrics_;
};

TEST_F(LiftMetricsTest, LiftMetrics) {
  auto json = metrics_.toJson();
  auto parsedMetrics = LiftMetrics::fromJson(json);
  EXPECT_EQ(metrics_, parsedMetrics);
}
} // namespace private_lift
