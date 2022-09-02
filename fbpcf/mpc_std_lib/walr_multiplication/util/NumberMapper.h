/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>
#include "folly/logging/xlog.h"

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
  static constexpr double groupSize =
      double(std::numeric_limits<FixedPointType>::max()) + 1;

  /*
   * Methods supporting converting from/to FixedPointType.
   * Given an nonnegative double input, mapToDouble(mapToFixedPointType(input))
   * preserves the input (up to precision loss).
   * (1) When abs(input) >= 1.0, the RELATIVE precision loss will be
   * less than abs(input) / divisor.
   * (2) When abs(input) < 1.0, the ABSOLUTE precision loss will be
   * less than 1 / divisor.
   */
  inline FixedPointType mapToFixedPointType(double input) const {
    double product = input * divisor_;
    if (std::abs(product) > groupSize) {
      XLOG_EVERY_MS(WARN, 500) << "Magnitude of input number " << input
                               << " too large. Conversion exceeds group size."
                               << " May incur unwanted precision loss.";
    }
    uintmax_t rst = static_cast<uintmax_t>(static_cast<intmax_t>(product));
    return static_cast<FixedPointType>(rst);
  }

  std::vector<FixedPointType> mapToFixedPointType(
      const std::vector<double>& input) const {
    std::vector<FixedPointType> rst(input.size());
    std::transform(input.cbegin(), input.cend(), rst.begin(), [this](double a) {
      return mapToFixedPointType(a);
    });
    return rst;
  }

  inline double mapToDouble(FixedPointType input) const {
    return input / static_cast<double>(divisor_);
  }

  std::vector<double> mapToDouble(
      const std::vector<FixedPointType>& input) const {
    std::vector<double> rst(input.size());
    std::transform(
        input.cbegin(), input.cend(), rst.begin(), [this](FixedPointType a) {
          return mapToDouble(a);
        });
    return rst;
  }

  uint64_t getDivisor() const {
    return divisor_;
  }

  void setDivisor(uint64_t divisor) {
    if (divisor > std::numeric_limits<FixedPointType>::max()) {
      throw std::invalid_argument(
          "The divisor's value should not exceed the max value a FixedPointType can represent.");
    }
    divisor_ = divisor;
  }

 private:
  uint64_t divisor_;
};

} // namespace fbpcf::mpc_std_lib::walr::util
