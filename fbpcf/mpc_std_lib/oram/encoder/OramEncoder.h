/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdexcept>
#include <unordered_map>
#include "folly/String.h"

#include "fbpcf/mpc_std_lib/oram/encoder/IOramEncoder.h"

namespace fbpcf::mpc_std_lib::oram {

class OramEncoder : public IOramEncoder {
 public:
  explicit OramEncoder() : currentIndex_(1), breakdownMapping_() {}

  std::vector<uint32_t> generateORAMIndexes(
      const std::vector<std::vector<uint32_t>>& tuples) override;

  std::unique_ptr<OramMappingConfig> exportMappingConfig() const override;

 private:
  uint32_t currentIndex_;
  std::unordered_map<std::string, uint32_t> breakdownMapping_;

  std::string convertBreakdownsToKey(
      const std::vector<uint32_t>& breakdownValues) const {
    return folly::join(",", breakdownValues);
  }
};

} // namespace fbpcf::mpc_std_lib::oram
