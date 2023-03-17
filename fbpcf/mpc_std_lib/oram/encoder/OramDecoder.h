/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/mpc_std_lib/oram/encoder/OramMappingConfig.h"

namespace fbpcf::mpc_std_lib::oram {

class OramDecoder {
 public:
  /*
   * Reconstructs an object that can perform the reverse mapping from ORAM
   * bucket indexes to the original values that were encoded.
   */
  explicit OramDecoder(std::unique_ptr<OramMappingConfig> config);

  /*
   * Retrieves the original encoded values given the ORAM index and the
   * previously saved mapping config. An empty vector indicates that the index
   * corersponds to a filtered out value.
   */
  std::vector<std::vector<uint32_t>> decodeORAMIndexes(
      const std::vector<uint32_t>& oramIndexes);

 private:
  std::unordered_map<uint32_t, std::vector<uint32_t>>
      oramIndexToBreakdownMapping_{};
};
} // namespace fbpcf::mpc_std_lib::oram
