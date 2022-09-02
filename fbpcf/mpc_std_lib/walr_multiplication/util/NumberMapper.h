/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <limits.h>
#include <sys/types.h>
#include <cstdint>
#include <stdexcept>

namespace fbpcf::mpc_std_lib::walr::util {

template <typename FixedPointType>
class NumberMapper {
  static_assert(
      std::is_integral<FixedPointType>::value &&
          std::is_unsigned_v<FixedPointType>,
      "Currently only support FixedPointType being some uint type with width no more than 64.");

 public:
  explicit NumberMapper(uint64_t divisor) : divisor_(divisor) {
    if (divisor > std::numeric_limits<FixedPointType>::max()) {
      throw std::invalid_argument(
          "The divisor's value should not exceed the max value a FixedPointType can represent.");
    }
  }

  // Methods supporting converting from/to FixedPointType
  inline FixedPointType mapToFixedPointType(double input) const;

  std::vector<FixedPointType> mapToFixedPointType(
      const std::vector<double>& input) const;

  inline double mapToDouble(FixedPointType input) const;

  std::vector<double> mapToDouble(
      const std::vector<FixedPointType>& input) const;

  uint64_t getDivisor() const {
    return divisor_;
  }

  void setDivisor(uint64_t divisor) {
    divisor_ = divisor;
  }

 private:
  uint64_t divisor_;
};

} // namespace fbpcf::mpc_std_lib::walr::util
