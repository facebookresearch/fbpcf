/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "folly/dynamic.h"
#include "folly/json.h"

namespace fbpcf::mpc_std_lib::oram {
class OramMappingConfig {
 public:
  OramMappingConfig(
      std::unordered_map<std::string, uint32_t> breakdownMapping,
      bool wereBreakdownsFiltered,
      uint32_t filterIndex)
      : breakdownMapping_(breakdownMapping),
        wereBreakdownsFiltered_{wereBreakdownsFiltered},
        filterIndex_{filterIndex} {}

  std::string toJson() {
    return "";
  }

  static OramMappingConfig fromJson(std::string /* str */) {
    throw std::runtime_error("Not implemented");
  }

  const std::unordered_map<std::string, uint32_t>& getBreakdownMapping() const {
    return breakdownMapping_;
  }

  bool wereBreakdownsFiltered() const {
    return wereBreakdownsFiltered_;
  }

  uint32_t getFilterIndex() {
    return filterIndex_;
  }

 private:
  std::unordered_map<std::string, uint32_t> breakdownMapping_{};
  bool wereBreakdownsFiltered_ = false;
  uint32_t filterIndex_ = 0;
};

} // namespace fbpcf::mpc_std_lib::oram
