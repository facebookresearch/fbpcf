/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "LiftMetrics.h"

#include <cstdint>
#include <vector>

#include "folly/dynamic.h"
#include "folly/json.h"

#include "../../pcf/common/FunctionalUtil.h"
#include "../../pcf/common/VectorUtil.h"

namespace private_lift {
bool LiftMetrics::operator==(const LiftMetrics& other) const noexcept {
  return testPopulation == other.testPopulation &&
      controlPopulation == other.controlPopulation &&
      testConverters == other.testConverters && controlConverters == other.controlConverters &&
      testValue == other.testValue && controlValue == other.controlValue &&
      testSquared == other.testSquared &&
      controlSquared == other.controlSquared;
}

LiftMetrics LiftMetrics::operator+(const LiftMetrics& other) const noexcept {
  return LiftMetrics{testPopulation + other.testPopulation,
                     controlPopulation + other.controlPopulation,
                     testConverters + other.testConverters,
                     controlConverters + other.controlConverters,
                     testValue + other.testValue,
                     controlValue + other.controlValue,
                     testSquared + other.testSquared,
                     controlSquared + other.controlSquared};
}

LiftMetrics LiftMetrics::operator^(const LiftMetrics& other) const noexcept {
  return LiftMetrics{testPopulation ^ other.testPopulation,
                     controlPopulation ^ other.controlPopulation,
                     testConverters ^ other.testConverters,
                     controlConverters ^ other.controlConverters,
                     testValue ^ other.testValue,
                     controlValue ^ other.controlValue,
                     testSquared ^ other.testSquared,
                     controlSquared ^ other.controlSquared};
}

std::string LiftMetrics::toJson() const {
  auto obj = toDynamic();
  return folly::toJson(obj);
}

LiftMetrics LiftMetrics::fromJson(const std::string& str) {
  auto obj = folly::parseJson(str);
  return fromDynamic(obj);
}

folly::dynamic LiftMetrics::toDynamic() const {
  return folly::dynamic::object("testPopulation", testPopulation)(
      "controlPopulation", controlPopulation)("testConverters", testConverters)(
      "controlConverters", controlConverters)("testValue", testValue)(
      "controlValue", controlValue)("testSquared", testSquared)(
      "controlSquared", controlSquared);
}

LiftMetrics LiftMetrics::fromDynamic(const folly::dynamic& obj) {
  LiftMetrics metrics;

  metrics.testPopulation = obj["testPopulation"].asInt();
  metrics.controlPopulation = obj["controlPopulation"].asInt();
  metrics.testConverters = obj["testConverters"].asInt();
  metrics.controlConverters = obj["controlConverters"].asInt();
  metrics.testValue = obj["testValue"].asInt();
  metrics.controlValue = obj["controlValue"].asInt();
  metrics.testSquared = obj["testSquared"].asInt();
  metrics.controlSquared = obj["controlSquared"].asInt();

  return metrics;
}
} // namespace private_lift
