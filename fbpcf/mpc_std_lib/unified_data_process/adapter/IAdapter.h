/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdexcept>
#include "folly/Format.h"

#include <stdint.h>
#include <vector>

namespace fbpcf::mpc_std_lib::unified_data_process::adapter {

/*
 * An adapter converts a union-like mapping result into an intersection-like
 * mapping result.
 */
class IAdapter {
 public:
  virtual ~IAdapter() = default;

  /**
   * convert a union-like mapping result to an intersection-like mapping
   * result.
   * @param unionMap the union-like mapping result, no mapping is represented
   * as -1. The i-th index in the input represents the index of this party's
   * element that corresponds to the i-th element in the union.
   * @return the corresponding intersection-like mapping result. The i-th index
   * in the output represents the index of peer's element that corresponds to
   * the i-th element in the intersection.
   */
  std::vector<int32_t> adapt(const std::vector<int32_t>& unionMap) {
    std::vector<bool> seen(unionMap.size(), false);

    for (size_t i = 0; i < unionMap.size(); i++) {
      auto unionIndex = unionMap[i];

      if (unionIndex != -1) {
        if (unionIndex < 0 || unionIndex >= unionMap.size()) {
          throw std::runtime_error(folly::sformat(
              "Union index out of bounds at index {} (Got {})", i, unionIndex));
        }

        if (seen[unionIndex]) {
          throw std::runtime_error(folly::sformat(
              "Union index seen a second time at index {} (Target index was {})",
              i,
              unionIndex));
        }

        seen[unionIndex] = true;
      }
    }

    return adapt_impl(unionMap);
  }

 private:
  virtual std::vector<int32_t> adapt_impl(
      const std::vector<int32_t>& unionMap) const = 0;
};

} // namespace fbpcf::mpc_std_lib::unified_data_process::adapter
