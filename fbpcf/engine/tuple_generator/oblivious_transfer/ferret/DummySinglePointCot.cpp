/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/DummySinglePointCot.h"
#include <assert.h>
#include <emmintrin.h>
#include <algorithm>
#include <cstddef>
#include <random>
#include "fbpcf/engine/util/util.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::ferret::insecure {

void DummySinglePointCot::senderInit(__m128i delta) {
  if (!util::getLsb(delta)) {
    throw std::invalid_argument("The LSB of delta must be 1.");
  }
  delta_ = delta;
  role_ = util::Role::sender;
}

void DummySinglePointCot::receiverInit() {
  role_ = util::Role::receiver;
}

std::vector<__m128i> DummySinglePointCot::senderExtend(
    std::vector<__m128i>&& baseCot) {
  assert(role_ == util::Role::sender);

  agent_->sendSingleT<__m128i>(delta_);

  int64_t length = pow(2, baseCot.size());
  std::vector<__m128i> rst(length);
  for (int i = 0; i < length; i++) {
    rst[i] = prg_->getRandomM128i();
    util::setLsbTo0(rst[i]);
  }

  agent_->sendT<__m128i>(rst);

  return rst;
}

std::vector<__m128i> DummySinglePointCot::receiverExtend(
    std::vector<__m128i>&& baseCot) {
  assert(role_ == util::Role::receiver);
  delta_ = agent_->receiveSingleT<__m128i>();

  int64_t length = pow(2, baseCot.size());
  auto rst = agent_->receiveT<__m128i>(length);

  int64_t position = 0;
  for (size_t i = 0; i < baseCot.size(); i++) {
    position <<= 1;
    position ^= !util::getLsb(baseCot[i]);
  }
  rst[position] = _mm_xor_si128(rst[position], delta_);
  return rst;
}

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::ferret::insecure
