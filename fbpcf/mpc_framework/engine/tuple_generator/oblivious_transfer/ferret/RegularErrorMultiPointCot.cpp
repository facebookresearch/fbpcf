/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <assert.h>
#include <emmintrin.h>
#include <algorithm>
#include <cmath>
#include <iterator>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/RegularErrorMultiPointCot.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::ferret {

void RegularErrorMultiPointCot::init(int64_t length, int64_t weight) {
  assert(length % weight == 0);

  spcotLength_ = length / weight;
  spcotCount_ = weight;
  baseCotSize_ = std::log2(spcotLength_);

  assert(std::pow(2, baseCotSize_) == spcotLength_);
}

void RegularErrorMultiPointCot::senderInit(
    __m128i delta,
    int64_t length,
    int64_t weight) {
  init(length, weight);
  role_ = util::Role::sender;
  delta_ = delta;
  singlePointCot_->senderInit(delta);
}

void RegularErrorMultiPointCot::receiverInit(int64_t length, int64_t weight) {
  init(length, weight);
  role_ = util::Role::receiver;
  singlePointCot_->receiverInit();
}

std::vector<__m128i> RegularErrorMultiPointCot::extend(
    std::vector<__m128i>&& baseCot) {
  if (baseCot.size() != baseCotSize_ * spcotCount_) {
    throw std::invalid_argument(
        "unexpected amount of base COT: actual:" +
        std::to_string(baseCot.size()) +
        " vs expected:" + std::to_string(baseCotSize_ * spcotCount_));
  }

  // We will perform single point cot for "weight" times, where the errors are
  // regularily distributed across position 1 to position length. With that
  // said, we are performing single point cot with either length/weight.

  std::vector<__m128i> rst;
  int64_t index = 0;

  for (int i = 0; i < spcotCount_; i++) {
    auto tmp = singleCotExtend(std::vector(
        baseCot.begin() + index, baseCot.begin() + index + baseCotSize_));

    rst.insert(
        rst.end(),
        std::make_move_iterator(tmp.begin()),
        std::make_move_iterator(tmp.begin() + spcotLength_));
    index += baseCotSize_;
  }
  return rst;
}

std::vector<__m128i> RegularErrorMultiPointCot::senderExtend(
    std::vector<__m128i>&& baseCot) {
  assert(role_ == util::Role::sender);
  return extend(std::move(baseCot));
}

std::vector<__m128i> RegularErrorMultiPointCot::receiverExtend(
    std::vector<__m128i>&& baseCot) {
  assert(role_ == util::Role::receiver);
  return extend(std::move(baseCot));
}

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::ferret
