/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "PrivateData.h"

namespace private_lift::emp_utils {

inline const std::vector<emp::Bit> intsToBits(
    const std::vector<emp::Integer>& in) {
  std::vector<emp::Bit> bits;
  bits.reserve(in.size());
  for (auto i = 0; i < in.size(); ++i) {
    // We only take the first (zero-th) bit. It's up to the caller to ensure
    // that the input vector of Integers actually represents Bits.
    bits.push_back(in[i][0]);
  }
  return bits;
}

inline const std::vector<emp::Integer> bitsToInts(
    const std::vector<emp::Bit>& in) {
  // auto bitLen = std::ceil(std::log2(in.size() + 1));
  // TODO: Braced initialization
  const emp::Integer zero(INT_SIZE, 0, emp::PUBLIC);
  const emp::Integer one(INT_SIZE, 1, emp::PUBLIC);

  std::vector<emp::Integer> ints;
  ints.reserve(in.size());
  for (auto i = 0; i < in.size(); ++i) {
    ints.emplace_back(emp::If(in[i], one, zero));
  }
  return ints;
}

inline const emp::Integer getMin(emp::Integer value1, emp::Integer value2) {
  emp::Bit cmp = value1 > value2;
  return emp::If(cmp, value2, value1);
}

inline const emp::Integer getMin(const std::vector<emp::Integer>& values) {
  emp::Integer minValue(INT_SIZE, __INT_MAX__, emp::PUBLIC);
  for (auto i = 0; i < values.size(); i++) {
    minValue = getMin(minValue, values.at(i));
  }
  return minValue;
}

template <int TO>
const int64_t sum(const std::vector<emp::Integer>& in) {
  // TODO: If `in` is empty for some reason, immediately return 0.
  const emp::Integer zero{in.at(0).size(), 0, emp::PUBLIC};
  const auto res = std::accumulate(in.begin(), in.end(), zero);

  // Support 32 bit and 64 bit integers
  if (res.size() == 32) {
    return res.reveal<int32_t>(TO);
  } else if (res.size() == 64) {
    return res.reveal<int64_t>(TO);
  } else {
    throw std::runtime_error(
        "Only 32 and 64 bit integers are supported by sum()");
  }
}

template <int TO>
const int64_t sum(const std::vector<emp::Bit>& in) {
  // POTENTIAL OPTIMIZATION: this wastes memory since it stores an additional
  // vector<Integer> whereas we could instead calculate on the fly.
  return sum<TO>(bitsToInts(in));
}

} // namespace private_lift::emp_utils
