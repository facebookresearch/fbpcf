// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

#include <string>
#include <vector>

#include "LiftMetrics.h"

namespace private_lift {
struct GroupedLiftMetrics {
  LiftMetrics metrics;
  std::vector<LiftMetrics> subGroupMetrics;

  bool operator==(const GroupedLiftMetrics& other) const noexcept;
  bool operator!=(const GroupedLiftMetrics& other) const noexcept;
  GroupedLiftMetrics operator+(const GroupedLiftMetrics& other) const noexcept;
  GroupedLiftMetrics operator^(const GroupedLiftMetrics& other) const noexcept;

  std::string toJson() const;
  static GroupedLiftMetrics fromJson(const std::string& str);
};

} // namespace private_lift
