/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/mpc_framework/mpc_std_lib/oram/DummySinglePointArrayGenerator.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include "fbpcf/mpc_framework/engine/util/util.h"
#include "fbpcf/mpc_framework/mpc_std_lib/util/util.h"

namespace fbpcf::mpc_std_lib::oram::insecure {

std::vector<std::pair<std::vector<bool>, std::vector<__m128i>>>
DummySinglePointArrayGenerator::generateSinglePointArrays(
    const std::vector<std::vector<bool>>& indexShares,
    size_t length) {
  auto width = indexShares.size();
  auto batchSize = indexShares.at(0).size();
  if ((width == 0) || (batchSize == 0)) {
    throw std::invalid_argument("Empty input!");
  }

  std::vector<std::pair<std::vector<bool>, std::vector<__m128i>>> rst;
  std::vector<std::vector<bool>> boolData(
      batchSize, std::vector<bool>(width, false));

  for (size_t t = 0; t < width; t++) {
    if (indexShares.at(t).size() != batchSize) {
      throw std::invalid_argument("Inconsistent input size");
    }
    agent_->sendBool(indexShares.at(t));
    auto otherShare = agent_->receiveBool(batchSize);
    if (otherShare.size() != batchSize) {
      throw std::runtime_error("unexpected size!");
    }
    for (size_t i = 0; i < batchSize; i++) {
      boolData[i][t] = otherShare.at(i) ^ indexShares.at(t).at(i);
    }
  }

  for (size_t i = 0; i < batchSize; i++) {
    auto index = util::Adapters<uint32_t>::convertFromBits(boolData.at(i));
    std::vector<bool> sharedIndicator(length, false);
    std::vector<__m128i> keys(length, _mm_set_epi64x(0, 0));

    sharedIndicator[index] = thisPartyToSetIndicator_;
    keys[index] = engine::util::getRandomM128iFromSystemNoise();

    rst.push_back({std::move(sharedIndicator), std::move(keys)});
  }
  return rst;
}

} // namespace fbpcf::mpc_std_lib::oram::insecure
