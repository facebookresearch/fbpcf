// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

#include <vector>
#include <emp-tool/emp-tool.h>

namespace private_lift {
/*
 * Simple struct representing the metrics in an encrypted Lift computation.
 * These values are not readable until they have been revealed.
 */
struct EncryptedLiftMetrics {
  emp::Integer idMatch;
  emp::Integer testPopulation;
  emp::Integer controlPopulation;
  emp::Integer testBuyers;
  emp::Integer controlBuyers;
  emp::Integer testSales;
  emp::Integer controlSales;
  emp::Integer testSquared;
  emp::Integer controlSquared;

  EncryptedLiftMetrics operator+(const EncryptedLiftMetrics& other) const noexcept;
  EncryptedLiftMetrics operator^(const EncryptedLiftMetrics& other) const noexcept;
};
} // namespace private_lift
