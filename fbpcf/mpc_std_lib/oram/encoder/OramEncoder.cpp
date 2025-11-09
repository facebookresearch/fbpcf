/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/mpc_std_lib/oram/encoder/OramEncoder.h"

namespace fbpcf::mpc_std_lib::oram {

std::vector<uint32_t> OramEncoder::generateORAMIndexes(
    const std::vector<std::vector<uint32_t>>& tuples) {
  std::vector<uint32_t> rst(0);
  rst.reserve(tuples.size());
  for (const auto& tuple : tuples) {
    bool hasThisRowBeenFiltered = false;
    for (const auto& filter : *filters_) {
      if (!filter->apply(tuple)) {
        if (!wasAnyRowFiltered_) {
          wasAnyRowFiltered_ = true;
          filteredValuesIndex_ = currentIndex_;
          currentIndex_++;
        }
        rst.push_back(filteredValuesIndex_);
        hasThisRowBeenFiltered = true;
        break;
      }
    }

    if (hasThisRowBeenFiltered) {
      continue;
    }

    std::string breakdownKey = convertBreakdownsToKey(tuple);

    if (breakdownMapping_.find(breakdownKey) != breakdownMapping_.end()) {
      rst.push_back(breakdownMapping_[breakdownKey]);
    } else {
      breakdownMapping_.emplace(breakdownKey, currentIndex_);
      rst.push_back(currentIndex_);
      currentIndex_++;
    }
  }

  return rst;
}

std::unique_ptr<OramMappingConfig> OramEncoder::exportMappingConfig() const {
  return std::make_unique<OramMappingConfig>(
      breakdownMapping_, wasAnyRowFiltered_, filteredValuesIndex_);
}

} // namespace fbpcf::mpc_std_lib::oram
