/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace fbpcf::vector {
template <typename T>
std::vector<T> apply(
    const std::vector<T>& a,
    const std::vector<T>& b,
    std::function<T(T, T)> f) {
  if (a.size() != b.size()) {
    throw std::invalid_argument("Size doesn't match.");
  }

  std::vector<T> res;
  res.reserve(a.size());

  for (int64_t i = 0; i < a.size(); i++) {
    res.push_back(f(a[i], b[i]));
  }

  return res;
}

template <typename T>
std::vector<T> Add(const std::vector<T>& a, const std::vector<T>& b) {
  return apply<T>(a, b, [](const T& x, const T& y) { return x + y; });
}

template <typename T>
std::vector<T> Xor(const std::vector<T>& a, const std::vector<T>& b) {
  return apply<T>(a, b, [](const T& x, const T& y) { return x ^ y; });
}
} // namespace fbpcf::vector
