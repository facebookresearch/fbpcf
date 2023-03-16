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

#include "fbpcf/mpc_std_lib/oram/encoder/IFilter.h"
#include "fbpcf/mpc_std_lib/oram/encoder/IOramEncoder.h"

namespace fbpcf::mpc_std_lib::oram {

class OramEncoder : public IOramEncoder {
 public:
  explicit OramEncoder(
      std::unique_ptr<std::vector<std::unique_ptr<IFilter>>> filters)
      : filters_{std::move(filters)} {}

  std::vector<uint32_t> generateORAMIndexes(
      const std::vector<std::vector<uint32_t>>& tuples) override;

  std::unique_ptr<OramMappingConfig> exportMappingConfig() const override;

 private:
  std::unique_ptr<std::vector<std::unique_ptr<IFilter>>> filters_;

  uint32_t filteredValuesIndex_ = 0;
  bool wasAnyRowFiltered_ = false;
  uint32_t currentIndex_ = 0;
  std::unordered_map<std::string, uint32_t> breakdownMapping_{};

  std::string convertBreakdownsToKey(
      const std::vector<uint32_t>& breakdownValues) const {
    return folly::join(",", breakdownValues);
  }
};

} // namespace fbpcf::mpc_std_lib::oram
