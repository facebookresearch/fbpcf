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
  // For conversion_lift, events == conversions
  // For converter_lift, events == converters
  int64_t testEvents = 0;
  int64_t controlEvents = 0;
  int64_t testValue = 0;
  int64_t controlValue = 0;
  int64_t testSquared = 0;
  int64_t controlSquared = 0;
  int64_t testLogValue = 0;
  int64_t controlLogValue = 0;
  int64_t testStddevValue = 0;
  int64_t controlStddevValue = 0;
  int64_t testStddevLogValue = 0;
  int64_t controlStddevLogValue = 0;

  OutputMetricsData() = default;

  OutputMetricsData(bool isConversionLift)
      : isConversionLift_{isConversionLift} {}

  bool isConversionLift() const {
    return isConversionLift_;
  }

  friend std::ostream& operator<<(
      std::ostream& os,
      const OutputMetricsData& out) {
    if (out.isConversionLift()) {
      os << "Test Conversions: " << out.testEvents << "\n";
      os << "Control Conversions: " << out.controlEvents << "\n";
    } else {
      os << "Test Converters: " << out.testEvents << "\n";
      os << "Control Converters: " << out.controlEvents << "\n";
    }
    os << "Test Value: " << out.testValue << "\n";
    os << "Control Value: " << out.controlValue << "\n";
    os << "Test Squared: " << out.testSquared << "\n";
    os << "Control Squared: " << out.controlSquared << "\n";
    os << "Test Population: " << out.testPopulation << "\n";
    os << "Control Population: " << out.controlPopulation << "\n";
    os << "Test LogValue: " << out.testLogValue << "\n";
    os << "Control LogValue: " << out.controlLogValue << "\n";
    os << "Test StddevValue: " << out.testStddevValue << "\n";
    os << "Control StddevValue: " << out.controlStddevValue << "\n";
    os << "Test StddevLogValue: " << out.testStddevLogValue << "\n";
    os << "Control StddevLogValue: " << out.controlStddevLogValue << "\n";

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
    metrics.testConverters = testEvents;
    metrics.controlConverters = controlEvents;
    metrics.testValue = testValue;
    metrics.controlValue = controlValue;
    metrics.testSquared = testSquared;
    metrics.controlSquared = controlSquared;
    metrics.testLogValue = testLogValue;
    metrics.controlLogValue = controlLogValue;
    metrics.testStddevValue = testStddevValue;
    metrics.controlStddevValue = controlStddevValue;
    metrics.testStddevLogValue = testStddevLogValue;
    metrics.controlStddevLogValue = controlStddevLogValue;
    return metrics;
  }

 private:
  bool isConversionLift_;
};

} // namespace private_lift
