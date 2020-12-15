// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

#include <vector>

#include "EncryptedLiftMetrics.h"

namespace private_lift {
struct GroupedEncryptedLiftMetrics {
  EncryptedLiftMetrics metrics;
  std::vector<EncryptedLiftMetrics> subGroupMetrics;

  GroupedEncryptedLiftMetrics operator+(const GroupedEncryptedLiftMetrics& other) const noexcept;
  GroupedEncryptedLiftMetrics operator^(const GroupedEncryptedLiftMetrics& other) const noexcept;
};

} // namespace private_lift
