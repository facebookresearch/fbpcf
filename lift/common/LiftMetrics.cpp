// Copyright 2004-present Facebook. All Rights Reserved.

#include "LiftMetrics.h"

#include <cstdint>
#include <vector>

#include "folly/dynamic.h"
#include "folly/json.h"

#include "../../pcf/common/FunctionalUtil.h"
#include "../../pcf/common/VectorUtil.h"

namespace private_lift {
bool LiftMetrics::operator==(const LiftMetrics& other) const noexcept {
  return idMatch == other.idMatch && testPopulation == other.testPopulation &&
      controlPopulation == other.controlPopulation &&
      testBuyers == other.testBuyers && controlBuyers == other.controlBuyers &&
      testSales == other.testSales && controlSales == other.controlSales &&
      testSquared == other.testSquared &&
      controlSquared == other.controlSquared;
}

bool LiftMetrics::operator!=(const LiftMetrics& other) const noexcept {
  return !(*this == other);
}

LiftMetrics LiftMetrics::operator+(const LiftMetrics& other) const noexcept {
  return LiftMetrics{idMatch + other.idMatch,
                     testPopulation + other.testPopulation,
                     controlPopulation + other.controlPopulation,
                     testBuyers + other.testBuyers,
                     controlBuyers + other.controlBuyers,
                     testSales + other.testSales,
                     controlSales + other.controlSales,
                     testSquared + other.testSquared,
                     controlSquared + other.controlSquared};
}

LiftMetrics LiftMetrics::operator^(const LiftMetrics& other) const noexcept {
  return LiftMetrics{idMatch ^ other.idMatch,
                     testPopulation ^ other.testPopulation,
                     controlPopulation ^ other.controlPopulation,
                     testBuyers ^ other.testBuyers,
                     controlBuyers ^ other.controlBuyers,
                     testSales ^ other.testSales,
                     controlSales ^ other.controlSales,
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
  return folly::dynamic::object("idMatch", idMatch)(
      "testPopulation", testPopulation)("controlPopulation", controlPopulation)(
      "testBuyers", testBuyers)("controlBuyers", controlBuyers)(
      "testSales", testSales)("controlSales", controlSales)(
      "testSquared", testSquared)("controlSquared", controlSquared);
}

LiftMetrics LiftMetrics::fromDynamic(const folly::dynamic& obj) {
  LiftMetrics metrics;

  metrics.idMatch = obj["idMatch"].asInt();
  metrics.testPopulation = obj["testPopulation"].asInt();
  metrics.controlPopulation = obj["controlPopulation"].asInt();
  metrics.testBuyers = obj["testBuyers"].asInt();
  metrics.controlBuyers = obj["controlBuyers"].asInt();
  metrics.testSales = obj["testSales"].asInt();
  metrics.controlSales = obj["controlSales"].asInt();
  metrics.testSquared = obj["testSquared"].asInt();
  metrics.controlSquared = obj["controlSquared"].asInt();

  return metrics;
}
} // namespace private_lift
