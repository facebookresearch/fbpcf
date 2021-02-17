/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/

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
  emp::Integer testConversions;
  emp::Integer controlConversions;
  emp::Integer testConverters;
  emp::Integer controlConverters;
  emp::Integer testValue;
  emp::Integer controlValue;
  emp::Integer testSquared;
  emp::Integer controlSquared;

  EncryptedLiftMetrics operator+(const EncryptedLiftMetrics& other) const noexcept;
  EncryptedLiftMetrics operator^(const EncryptedLiftMetrics& other) const noexcept;
};
} // namespace private_lift
