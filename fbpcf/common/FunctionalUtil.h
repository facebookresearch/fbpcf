
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

namespace fbpcf::functional {
template <typename I, typename O>
std::vector<O> map(
    const std::vector<I>& input,
    const std::function<O(const I&)>& f) {
  std::vector<O> output;
  std::transform(input.begin(), input.end(), std::back_inserter(output), f);
  return output;
}

template <typename T>
T reduce(
    const std::vector<T>& input,
    const std::function<T(const T&, const T&)>& f) {
  if (input.size() == 0) {
    throw std::invalid_argument("Can't reduce empty vector.");
  }

  return std::accumulate(++input.begin(), input.end(), input[0], f);
}
} // namespace fbpcf::functional
