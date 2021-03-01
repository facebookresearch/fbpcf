/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "../common/GroupedLiftMetrics.h"

namespace private_lift {
/*
 * Simple struct representing the metrics in a Lift computation
 */
struct OutputMetricsData {
  int64_t testPopulation = 0;
  int64_t controlPopulation = 0;
  int64_t testEvents = 0;
  int64_t controlEvents = 0;
  int64_t testConverters = 0;
  int64_t controlConverters = 0;
  int64_t testValue = 0;
  int64_t controlValue = 0;
  int64_t testSquared = 0;
  int64_t controlSquared = 0;
  int64_t testMatchCount = 0;
  int64_t controlMatchCount = 0;
  int64_t testLogValue = 0;
  int64_t controlLogValue = 0;

  OutputMetricsData() = default;

  OutputMetricsData(bool isConversionLift)
      : isConversionLift_{isConversionLift} {}

  bool isConversionLift() const {
    return isConversionLift_;
  }

  friend std::ostream& operator<<(
      std::ostream& os,
      const OutputMetricsData& out) {
    os << "Test Conversions: " << out.testEvents << "\n";
    os << "Control Conversions: " << out.controlEvents << "\n";
    os << "Test Converters: " << out.testConverters << "\n";
    os << "Control Converters: " << out.controlConverters << "\n";
    os << "Test Value: " << out.testValue << "\n";
    os << "Control Value: " << out.controlValue << "\n";
    os << "Test Squared: " << out.testSquared << "\n";
    os << "Control Squared: " << out.controlSquared << "\n";
    os << "Test Population: " << out.testPopulation << "\n";
    os << "Control Population: " << out.controlPopulation << "\n";
    os << "Test Match Count: " << out.testMatchCount << "\n";
    os << "Control Match Count: " << out.controlMatchCount << "\n";
    os << "Test LogValue: " << out.testLogValue << "\n";
    os << "Control LogValue: " << out.controlLogValue << "\n";

    return os;
  }

  // Helper method that converts the output metrics of a game implementation
  // to a common lift metrics representation. The LiftMetrics introduced in
  // D22969707 serve as the common metrics data structure between game and
  // aggregator
  LiftMetrics toLiftMetrics() const {
    LiftMetrics metrics{};
    metrics.testPopulation = testPopulation;
    metrics.controlPopulation = controlPopulation;
    metrics.testConversions = testEvents;
    metrics.controlConversions = controlEvents;
    metrics.testConverters = testConverters;
    metrics.controlConverters = controlConverters;
    metrics.testValue = testValue;
    metrics.controlValue = controlValue;
    metrics.testSquared = testSquared;
    metrics.controlSquared = controlSquared;
    metrics.testMatchCount = testMatchCount;
    metrics.controlMatchCount = controlMatchCount;
    metrics.testLogValue = testLogValue;
    metrics.controlLogValue = controlLogValue;
    return metrics;
  }

 private:
  bool isConversionLift_;
};

} // namespace private_lift
