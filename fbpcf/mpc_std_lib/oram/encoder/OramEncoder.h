/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdexcept>
#include "fbpcf/mpc_std_lib/oram/encoder/IOramEncoder.h"

namespace fbpcf::mpc_std_lib::oram {

class OramEncoder : public IOramEncoder {
 public:
  explicit OramEncoder() {}

  std::vector<uint32_t> generateORAMIndexes(
      const std::vector<std::vector<uint32_t>>& /* tuples */) override;

  std::unique_ptr<OramMappingConfig> exportMappingConfig() const override;

 private:
};

} // namespace fbpcf::mpc_std_lib::oram
