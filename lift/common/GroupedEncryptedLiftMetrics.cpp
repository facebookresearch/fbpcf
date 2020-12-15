// Copyright 2004-present Facebook. All Rights Reserved.

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
