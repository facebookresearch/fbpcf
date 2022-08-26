/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/engine/tuple_generator/DummyProductShareGenerator.h"
#include <cstdint>

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
  for (size_t i = 0; i < left.size(); i++) {
    rst[i] = left[i] & partnerRight[i];
  }
  return rst;
}

std::vector<uint64_t> DummyProductShareGenerator::generateIntegerProductShares(
    const std::vector<uint64_t>& left,
    const std::vector<uint64_t>& right) {
  if (left.size() != right.size()) {
    throw std::runtime_error("Inconsistent length in inputs");
  }

  agent_->sendInt64(right);
  auto partnerRight = agent_->receiveInt64(left.size());

  std::vector<uint64_t> result(left.size());
  for (size_t i = 0; i < left.size(); i++) {
    result.at(i) = left.at(i) * partnerRight.at(i);
  }
  return result;
}

} // namespace fbpcf::engine::tuple_generator::insecure
