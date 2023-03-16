/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/mpc_std_lib/oram/encoder/OramEncoder.h"
#include <stdexcept>

namespace fbpcf::mpc_std_lib::oram {

std::vector<uint32_t> OramEncoder::generateORAMIndexes(
    const std::vector<std::vector<uint32_t>>& tuples) {
  std::vector<uint32_t> rst(0);
  rst.reserve(tuples.size());
  for (const auto& tuple : tuples) {
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

std::unique_ptr<IOramEncoder::OramMappingConfig>
OramEncoder::exportMappingConfig() const {
  throw std::runtime_error("Unimplemented");
}

} // namespace fbpcf::mpc_std_lib::oram
