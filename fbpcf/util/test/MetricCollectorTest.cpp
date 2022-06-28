/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/util/MetricCollector.h"
#include <folly/dynamic.h>
#include <gtest/gtest.h>

namespace fbpcf::util {

using namespace ::testing;

class TestMetricRecorder final : public IMetricRecorder {
 public:
  explicit TestMetricRecorder(folly::dynamic output) : output_(output) {}
  folly::dynamic getMetrics() const override {
    return output_;
  }

 private:
  folly::dynamic output_;
};

TEST(MetricCollectorTest, testMetricCollector) {
  MetricCollector collector("test");

  folly::dynamic metric1 = folly::dynamic::object("time", 3)("speed", 2);
  folly::dynamic metric2 =
      folly::dynamic::object("volume", "high")("price", 99.3);
  folly::dynamic expectedOutcome =
      folly::dynamic::object("test.traffic", metric1)("test.liquid", metric2);
  {
    std::shared_ptr<IMetricRecorder> r1 =
        std::make_shared<TestMetricRecorder>(metric1);
    std::shared_ptr<IMetricRecorder> r2 =
        std::make_shared<TestMetricRecorder>(metric2);

    collector.addNewRecorder("traffic", r1);
    collector.addNewRecorder("liquid", r2);
    // the recorders are going out of scope.
  }
  auto outcome = collector.collectMetrics();
  EXPECT_EQ(expectedOutcome, outcome);
}

} // namespace fbpcf::util
