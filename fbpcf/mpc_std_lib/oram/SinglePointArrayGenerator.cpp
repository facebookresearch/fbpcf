/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/mpc_std_lib/oram/SinglePointArrayGenerator.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include "fbpcf/engine/util/util.h"
#include "fbpcf/mpc_std_lib/util/util.h"

namespace fbpcf::mpc_std_lib::oram {

std::vector<std::pair<std::vector<bool>, std::vector<__m128i>>>
SinglePointArrayGenerator::generateSinglePointArrays(
    const std::vector<std::vector<bool>>& indexShares,
    size_t length) {
  auto width = indexShares.size();
  if (width == 0) {
    throw std::invalid_argument("Empty input!");
  }
  if (width >= 64) {
    // width can not be larger than 63, otherwise 2^width will overflow.
    throw std::invalid_argument("Width is too large!");
  }

  size_t batchSize = indexShares.at(0).size();

  if (batchSize == 0) {
    throw std::invalid_argument("Empty input!");
  }

  ArrayType rst(batchSize, {{firstShare_}, std::vector<__m128i>(1)});

  for (auto& item : rst) {
    item.second[0] = engine::util::getRandomM128iFromSystemNoise();
  }

  for (size_t i = 0; i < width; i++) {
    rst = expandArray(std::move(rst), indexShares.at(width - 1 - i));
    // neededLength is the smallest integer such that neededLength << (width - 1
    // - i) >= length
    auto neededLength =
        std::ceil(length * 1.0 / ((uint64_t)1 << (width - 1 - i)));
    for (auto& item : rst) {
      item.first.erase(item.first.begin() + neededLength, item.first.end());
      item.second.erase(item.second.begin() + neededLength, item.second.end());
    }
  }

  return rst;
}

ISinglePointArrayGenerator::ArrayType SinglePointArrayGenerator::expandArray(
    ArrayType&& src,
    const std::vector<bool>& indicatorShare) const {
  size_t batchSize = src.size();
  if (indicatorShare.size() != batchSize) {
    throw std::invalid_argument("Inconsistent size.");
  }
  ArrayType rst(batchSize);

  std::vector<__m128i> delta0(batchSize, _mm_set_epi64x(0, 0));
  std::vector<__m128i> delta1(batchSize, _mm_set_epi64x(0, 0));

  for (size_t i = 0; i < batchSize; i++) {
    rst[i].second = expander_->expand(std::move(src[i].second));
    for (size_t j = 0; j < rst.at(i).second.size(); j += 2) {
      delta0[i] = _mm_xor_si128(delta0.at(i), rst.at(i).second.at(j));
      delta1[i] = _mm_xor_si128(delta1.at(i), rst.at(i).second.at(j + 1));
    }
  }
  auto [delta, t0, t1] =
      obliviousDeltaCalculator_->calculateDelta(delta0, delta1, indicatorShare);
  for (size_t i = 0; i < batchSize; i++) {
    rst[i].first = std::vector<bool>(rst.at(i).second.size());

    for (size_t j = 0; j < rst.at(i).second.size(); j += 2) {
      rst[i].first[j] = engine::util::getLsb(rst.at(i).second.at(j));
      rst[i].first[j + 1] = engine::util::getLsb(rst.at(i).second.at(j + 1));
      if (src.at(i).first.at(j >> 1)) {
        rst[i].first[j] = rst.at(i).first.at(j) ^ t0.at(i);
        rst[i].first[j + 1] = rst.at(i).first.at(j + 1) ^ t1.at(i);

        rst[i].second[j] = _mm_xor_si128(rst.at(i).second.at(j), delta.at(i));
        rst[i].second[j + 1] =
            _mm_xor_si128(rst.at(i).second.at(j + 1), delta.at(i));
      }
    }
  }
  return rst;
}

} // namespace fbpcf::mpc_std_lib::oram
