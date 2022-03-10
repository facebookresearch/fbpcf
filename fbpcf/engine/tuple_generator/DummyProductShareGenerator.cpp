/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/engine/tuple_generator/DummyProductShareGenerator.h"

namespace fbpcf::engine::tuple_generator::insecure {

std::vector<bool> DummyProductShareGenerator::generateBooleanProductShares(
    const std::vector<bool>& left,
    const std::vector<bool>& right) {
  if (left.size() != right.size()) {
    throw std::runtime_error("Inconsistent length in inputs");
  }

  agent_->sendBool(right);
  auto partnerRight = agent_->receiveBool(left.size());

  std::vector<bool> rst(left.size());
  for (int i = 0; i < left.size(); i++) {
    rst[i] = left[i] & partnerRight[i];
  }
  return rst;
}

} // namespace fbpcf::engine::tuple_generator::insecure
