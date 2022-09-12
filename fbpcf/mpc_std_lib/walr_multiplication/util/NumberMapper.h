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

  using SignedFixedPointType = std::conditional_t<
      std::is_same<FixedPointType, std::uint8_t>::value,
      int8_t,
      std::conditional_t<
          std::is_same<FixedPointType, std::uint16_t>::value,
          int16_t,
          std::conditional_t<
              std::is_same<FixedPointType, std::uint32_t>::value,
              int32_t,
              std::conditional_t<
                  std::is_same<FixedPointType, std::uint64_t>::value,
                  int64_t,
                  int64_t>>>>;

 public:
  explicit NumberMapper(uint64_t divisor) : divisor_(divisor) {}

  static constexpr double groupSizeEstimation =
      ((double)std::numeric_limits<FixedPointType>::max()) + 1;
  static constexpr double maxAbsValueEstimation =
      (double)std::numeric_limits<SignedFixedPointType>::max();

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
    if (std::abs(product) > maxAbsValueEstimation) {
      XLOG_EVERY_MS(ERR, 500)
          << "Magnitude of input number " << input
          << "might be too large. Conversion exceeds group size."
          << " May incur unwanted precision loss.";
    }
    return static_cast<FixedPointType>(
        static_cast<SignedFixedPointType>(product));
  }

  std::vector<FixedPointType> mapToFixedPointType(
      const std::vector<double>& input) const {
    std::vector<FixedPointType> rst(input.size());
    std::transform(input.cbegin(), input.cend(), rst.begin(), [this](double a) {
      return mapToFixedPointType(a);
    });
    return rst;
  }

  inline double mapToUnsignedDouble(FixedPointType input) const {
    return input / static_cast<double>(divisor_);
  }

  std::vector<double> mapToUnsignedDouble(
      const std::vector<FixedPointType>& input) const {
    std::vector<double> rst(input.size());
    std::transform(
        input.cbegin(), input.cend(), rst.begin(), [this](FixedPointType a) {
          return mapToUnsignedDouble(a);
        });
    return rst;
  }

  // The input will first be interpreted as a signed integer of the same width
  inline double mapToSignedDouble(FixedPointType input) const {
    return static_cast<SignedFixedPointType>(input) /
        static_cast<double>(divisor_);
  }

  std::vector<double> mapToSignedDouble(
      const std::vector<FixedPointType>& input) const {
    std::vector<double> rst(input.size());
    std::transform(
        input.cbegin(), input.cend(), rst.begin(), [this](FixedPointType a) {
          return mapToSignedDouble(a);
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
