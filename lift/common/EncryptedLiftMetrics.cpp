/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/

#include "EncryptedLiftMetrics.h"

#include <vector>

namespace private_lift {
EncryptedLiftMetrics EncryptedLiftMetrics::operator+(const EncryptedLiftMetrics& other) const noexcept {
  return EncryptedLiftMetrics{idMatch + other.idMatch,
                     testPopulation + other.testPopulation,
                     controlPopulation + other.controlPopulation,
                     testConverters + other.testConverters,
                     controlConverters + other.controlConverters,
                     testValue + other.testValue,
                     controlValue + other.controlValue,
                     testSquared + other.testSquared,
                     controlSquared + other.controlSquared};
}

EncryptedLiftMetrics EncryptedLiftMetrics::operator^(const EncryptedLiftMetrics& other) const noexcept {
  return EncryptedLiftMetrics{idMatch ^ other.idMatch,
                     testPopulation ^ other.testPopulation,
                     controlPopulation ^ other.controlPopulation,
                     testConverters ^ other.testConverters,
                     controlConverters ^ other.controlConverters,
                     testValue ^ other.testValue,
                     controlValue ^ other.controlValue,
                     testSquared ^ other.testSquared,
                     controlSquared ^ other.controlSquared};
}
} // namespace private_lift
