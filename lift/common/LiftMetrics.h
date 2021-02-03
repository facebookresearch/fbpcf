/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>
#include <vector>
#include "folly/dynamic.h"

namespace private_lift {
/*
 * Simple struct representing the metrics in a Lift computation
 */
struct LiftMetrics {
  int64_t testPopulation;
  int64_t controlPopulation;
  int64_t testBuyers;
  int64_t controlBuyers;
  int64_t testValue;
  int64_t controlValue;
  int64_t testSquared;
  int64_t controlSquared;

  bool operator==(const LiftMetrics& other) const noexcept;
  LiftMetrics operator+(const LiftMetrics& other) const noexcept;
  LiftMetrics operator^(const LiftMetrics& other) const noexcept;

  std::string toJson() const;
  static LiftMetrics fromJson(const std::string& str);

 private:
  folly::dynamic toDynamic() const;
  static LiftMetrics fromDynamic(const folly::dynamic& obj);

  friend struct GroupedLiftMetrics;
};
} // namespace private_lift
