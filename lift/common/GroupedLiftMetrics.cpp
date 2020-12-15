// Copyright 2004-present Facebook. All Rights Reserved.

#include "GroupedLiftMetrics.h"

#include <vector>

#include "folly/json.h"

#include "../../pcf/common/FunctionalUtil.h"
#include "../../pcf/common/VectorUtil.h"

namespace private_lift {
bool GroupedLiftMetrics::operator==(const GroupedLiftMetrics& other) const
    noexcept {
  return metrics == other.metrics && subGroupMetrics == other.subGroupMetrics;
}

bool GroupedLiftMetrics::operator!=(const GroupedLiftMetrics& other) const
    noexcept {
      return !(*this == other);
}

GroupedLiftMetrics GroupedLiftMetrics::operator+(
    const GroupedLiftMetrics& other) const noexcept {
  return GroupedLiftMetrics{
      metrics + other.metrics,
      pcf::vector::Add(subGroupMetrics, other.subGroupMetrics)};
}

GroupedLiftMetrics GroupedLiftMetrics::operator^(
    const GroupedLiftMetrics& other) const noexcept {
  return GroupedLiftMetrics{
      metrics ^ other.metrics,
      pcf::vector::Xor(subGroupMetrics, other.subGroupMetrics)};
}

std::string GroupedLiftMetrics::toJson() const {
  auto container = folly::dynamic::array();
  std::transform(
      subGroupMetrics.begin(),
      subGroupMetrics.end(),
      std::back_inserter(container),
      [](auto m) { return m.toDynamic(); });
  folly::dynamic obj = folly::dynamic::object("metrics", metrics.toDynamic())(
      "subGroupMetrics", container);

  return folly::toJson(obj);
}

GroupedLiftMetrics GroupedLiftMetrics::fromJson(const std::string& str) {
  auto obj = folly::parseJson(str);
  std::vector<LiftMetrics> container;
  std::transform(
      obj["subGroupMetrics"].begin(),
      obj["subGroupMetrics"].end(),
      std::back_inserter(container),
      [](auto m) { return LiftMetrics::fromDynamic(m); });

  return GroupedLiftMetrics{LiftMetrics::fromDynamic(obj["metrics"]),
                            container};
}
} // namespace private_lift
