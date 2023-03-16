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
    const std::vector<std::vector<uint32_t>>& /* tuples */) {
  throw std::runtime_error("Unimplemented");
}

std::unique_ptr<IOramEncoder::OramMappingConfig>
OramEncoder::exportMappingConfig() const {
  throw std::runtime_error("Unimplemented");
}

} // namespace fbpcf::mpc_std_lib::oram
