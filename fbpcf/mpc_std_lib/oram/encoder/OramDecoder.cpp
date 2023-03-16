/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/mpc_std_lib/oram/encoder/OramDecoder.h"
#include <memory>
#include <stdexcept>

namespace fbpcf::mpc_std_lib::oram {

OramDecoder::OramDecoder(std::unique_ptr<OramMappingConfig> config) {
  // breakdown tuple string -> oram index
  for (const auto& singleMapping : config->getBreakdownMapping()) {
    // parse the key into a vector of uint values
    std::vector<std::string> tuple(0);
    folly::split(',', singleMapping.first, tuple);
    std::vector<uint32_t> converted(tuple.size());
    std::transform(
        tuple.begin(), tuple.end(), converted.begin(), [](std::string str) {
          return std::stoul(str);
        });

    // store the mapping of oram index -> parsed tuples
    oramIndexToBreakdownMapping_[singleMapping.second] = converted;
  }

  // Filtered ORAM values map to an empty vector (since there could be many)
  if (config->wereBreakdownsFiltered()) {
    oramIndexToBreakdownMapping_[config->getFilterIndex()] =
        std::vector<uint32_t>();
  }
}

std::vector<std::vector<uint32_t>> OramDecoder::decodeORAMIndexes(
    const std::vector<uint32_t>& oramIndexes) {
  std::vector<std::vector<uint32_t>> rst(0);
  rst.reserve(oramIndexes.size());
  for (auto oramIndex : oramIndexes) {
    if (oramIndexToBreakdownMapping_.find(oramIndex) ==
        oramIndexToBreakdownMapping_.end()) {
      throw std::runtime_error("Attempted to decode invalid ORAM index");
    }

    rst.push_back(oramIndexToBreakdownMapping_[oramIndex]);
  }

  return rst;
}

} // namespace fbpcf::mpc_std_lib::oram
