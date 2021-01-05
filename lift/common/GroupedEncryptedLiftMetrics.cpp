/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/

#include "GroupedEncryptedLiftMetrics.h"

#include <vector>

#include "../../pcf/common/FunctionalUtil.h"
#include "../../pcf/common/VectorUtil.h"

namespace private_lift {

GroupedEncryptedLiftMetrics GroupedEncryptedLiftMetrics::operator+(
    const GroupedEncryptedLiftMetrics& other) const noexcept {
  return GroupedEncryptedLiftMetrics{
      metrics + other.metrics,
      pcf::vector::Add(subGroupMetrics, other.subGroupMetrics)};
}

GroupedEncryptedLiftMetrics GroupedEncryptedLiftMetrics::operator^(
    const GroupedEncryptedLiftMetrics& other) const noexcept {
  return GroupedEncryptedLiftMetrics{
      metrics ^ other.metrics,
      pcf::vector::Xor(subGroupMetrics, other.subGroupMetrics)};
}
} // namespace private_lift
