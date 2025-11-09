/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <assert.h>
#include <emmintrin.h>
#include <algorithm>
#include <random>
#include "fbpcf/engine/util/util.h"

#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/DummyMultiPointCot.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::ferret::insecure {

void DummyMultiPointCot::senderInit(
    __m128i delta,
    int64_t length,
    int64_t weight) {
  if (!util::getLsb(delta)) {
    throw std::invalid_argument("The LSB of delta must be 1.");
  }
  delta_ = delta;
  length_ = length;
  weight_ = weight;
  role_ = util::Role::sender;
}

void DummyMultiPointCot::receiverInit(int64_t length, int64_t weight) {
  length_ = length;
  weight_ = weight;
  role_ = util::Role::receiver;
}

std::vector<__m128i> DummyMultiPointCot::senderExtend(
    std::vector<__m128i>&& /*baseCot*/) {
  assert(role_ == util::Role::sender);

  agent_->sendSingleT<__m128i>(delta_);

  std::vector<__m128i> rst(length_);
  for (int i = 0; i < length_; i++) {
    rst[i] = prg_->getRandomM128i();
    util::setLsbTo0(rst[i]);
  }

  agent_->sendT<__m128i>(rst);

  return rst;
}

std::vector<__m128i> DummyMultiPointCot::receiverExtend(
    std::vector<__m128i>&& /*baseCot*/) {
  assert(role_ == util::Role::receiver);

  delta_ = agent_->receiveSingleT<__m128i>();

  auto rst = agent_->receiveT<__m128i>(length_);

  auto positions = getRandomPositions();
  for (int i = 0; i < weight_; i++) {
    assert(rst.size() > positions[i]);
    rst[positions[i]] = _mm_xor_si128(rst[positions[i]], delta_);
  }
  return rst;
}

std::vector<int64_t> DummyMultiPointCot::getRandomPositions() {
  std::vector<int64_t> rst(length_);
  for (int i = 0; i < length_; i++) {
    rst[i] = i;
  }
  std::random_shuffle(rst.begin(), rst.end());
  rst.erase(rst.begin() + weight_, rst.end());
  return rst;
}

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::ferret::insecure
