/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emmintrin.h>
#include <smmintrin.h>
#include <functional>
#include <random>
#include <stdexcept>

#include "fbpcf/mpc_std_lib/oram/LinearOram.h"
#include "fbpcf/mpc_std_lib/util/util.h"

#include "fbpcf/engine/communication/InMemoryPartyCommunicationAgentFactory.h"
#include "fbpcf/mpc_std_lib/util/test/util.h"
#include "fbpcf/scheduler/EagerScheduler.h"
#include "fbpcf/scheduler/SchedulerHelper.h"
#include "fbpcf/scheduler/WireKeeper.h"
#include "fbpcf/test/TestHelper.h"

namespace fbpcf::mpc_std_lib::oram {

inline std::pair<std::vector<uint32_t>, std::vector<std::vector<bool>>>
generateTestDataForComparison(uint32_t batchSize, uint32_t range) {
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint32_t> randomNumber(0, range - 1);
  std::vector<uint32_t> plaintextValue(batchSize);
  for (auto& item : plaintextValue) {
    item = randomNumber(e);
  }
  auto bitWidth = std::ceil(log2(range));
  std::vector<std::vector<bool>> plaintextBit(
      bitWidth, std::vector<bool>(batchSize));
  for (size_t i = 0; i < bitWidth; i++) {
    for (size_t j = 0; j < batchSize; j++) {
      plaintextBit[i][j] = (plaintextValue.at(j) >> i) & 1;
    }
  }
  return {plaintextValue, plaintextBit};
}

template <int schedulerId>
std::pair<std::vector<std::vector<bool>>, std::vector<uint32_t>>
comparisonTestHelper(
    uint32_t batchSize,
    uint32_t range,
    const std::vector<std::vector<frontend::Bit<false, schedulerId, true>>>&
        publicBits) {
  auto [value, bits] = generateTestDataForComparison(batchSize, range);
  std::vector<frontend::Bit<true, schedulerId, true>> secretBits;
  for (auto& item : bits) {
    secretBits.push_back(frontend::Bit<true, schedulerId, true>(
        typename frontend::Bit<true, schedulerId, true>::ExtractedBit(item)));
  }

  std::vector<frontend::Bit<true, schedulerId, true>> comparisonResults;
  for (auto& item : publicBits) {
    comparisonResults.push_back(
        LinearOram<uint32_t, schedulerId>::Helper::comparison(
            secretBits, item));
  }
  std::vector<std::vector<bool>> rst;
  for (auto& item : comparisonResults) {
    rst.push_back(item.openToParty(0).getValue());
  }
  return {rst, value};
}

} // namespace fbpcf::mpc_std_lib::oram
