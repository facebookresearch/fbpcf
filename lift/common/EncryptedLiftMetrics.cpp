/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "EncryptedLiftMetrics.h"

#include <vector>

namespace private_lift {
EncryptedLiftMetrics EncryptedLiftMetrics::operator+(
    const EncryptedLiftMetrics& other) const noexcept {
  return EncryptedLiftMetrics{
      testPopulation + other.testPopulation,
      controlPopulation + other.controlPopulation,
      testConversions + other.testConversions,
      controlConversions + other.controlConversions,
      testConverters + other.testConverters,
      controlConverters + other.controlConverters,
      testValue + other.testValue,
      controlValue + other.controlValue,
      testSquared + other.testSquared,
      controlSquared + other.controlSquared,
      testMatchCount + other.testMatchCount,
      controlMatchCount + other.controlMatchCount,
      testImpressions + other.testImpressions,
      controlImpressions + other.controlImpressions,
      testClicks + other.testClicks,
      controlClicks + other.controlClicks,
      testSpend + other.testSpend,
      controlSpend + other.controlSpend};
}

EncryptedLiftMetrics EncryptedLiftMetrics::operator^(
    const EncryptedLiftMetrics& other) const noexcept {
  return EncryptedLiftMetrics{
      testPopulation ^ other.testPopulation,
      controlPopulation ^ other.controlPopulation,
      testConversions ^ other.testConversions,
      controlConversions ^ other.controlConversions,
      testConverters ^ other.testConverters,
      controlConverters ^ other.controlConverters,
      testValue ^ other.testValue,
      controlValue ^ other.controlValue,
      testSquared ^ other.testSquared,
      controlSquared ^ other.controlSquared,
      testMatchCount ^ other.testMatchCount,
      controlMatchCount ^ other.controlMatchCount,
      testImpressions ^ other.testImpressions,
      controlImpressions ^ other.controlImpressions,
      testClicks ^ other.testClicks,
      controlClicks ^ other.controlClicks,
      testSpend ^ other.testSpend,
      controlSpend ^ other.controlSpend};
}
} // namespace private_lift
