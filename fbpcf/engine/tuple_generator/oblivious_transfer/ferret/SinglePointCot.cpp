/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <emmintrin.h>
#include <string.h>
#include <memory>

#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/SinglePointCot.h"
#include "fbpcf/engine/util/util.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::ferret {

std::vector<__m128i> SinglePointCot::constructALayerOfKeyForSender(
    std::vector<__m128i>&& previousLayer,
    __m128i baseCot) {
  auto rst = expander_->expand(std::move(previousLayer));
  std::vector<__m128i> masks = {baseCot, _mm_xor_si128(baseCot, delta_)};
  cipherForHash_->encryptInPlace(masks);

  masks[0] = _mm_xor_si128(masks[0], baseCot);
  masks[1] = _mm_xor_si128(masks[1], _mm_xor_si128(baseCot, delta_));

  for (size_t i = 0; i < rst.size(); i += 2) {
    masks[0] = _mm_xor_si128(masks[0], rst[i]);
    masks[1] = _mm_xor_si128(masks[1], rst[i + 1]);
  }

  agent_->sendT<__m128i>(masks);

  return rst;
}

std::vector<__m128i> SinglePointCot::constructALayerOfKeyForReceiver(
    std::vector<__m128i>&& previousLayer,
    __m128i baseCot,
    int missingPosition) {
  auto rst = expander_->expand(std::move(previousLayer));

  size_t positionToFix = (missingPosition << 1) + util::getLsb(baseCot);

  std::vector<__m128i> tmp({baseCot});
  cipherForHash_->encryptInPlace(tmp);
  rst[positionToFix] = _mm_xor_si128(tmp[0], baseCot);

  auto masks = agent_->receiveT<__m128i>(2);

  rst[positionToFix] =
      _mm_xor_si128(masks[util::getLsb(baseCot)], rst[positionToFix]);

  for (size_t i = util::getLsb(baseCot); i < rst.size(); i += 2) {
    if (i != positionToFix) {
      rst[positionToFix] = _mm_xor_si128(rst[i], rst[positionToFix]);
    }
  }

  return rst;
}

void SinglePointCot::senderInit(__m128i delta) {
  delta_ = delta;
  role_ = util::Role::sender;
}

void SinglePointCot::receiverInit() {
  role_ = util::Role::receiver;
}

std::vector<__m128i> SinglePointCot::senderExtend(
    std::vector<__m128i>&& baseCot) {
  assert(role_ == util::Role::sender);
  expander_ = std::make_unique<util::Expander>(index_);
  cipherForHash_ = std::make_unique<util::Aes>(_mm_set_epi64x(index_, 0));

  std::vector<__m128i> rst{util::getRandomM128iFromSystemNoise()};

  // contruct the ggm tree
  for (size_t i = 0; i < baseCot.size(); i++) {
    rst = constructALayerOfKeyForSender(std::move(rst), baseCot[i]);
  }
  __m128i totalXor = delta_;

  for (size_t i = 0; i < rst.size(); i++) {
    util::setLsbTo0(rst[i]);
    totalXor = _mm_xor_si128(totalXor, rst[i]);
  }

  agent_->sendSingleT<__m128i>(totalXor);
  index_++;

  return rst;
}

std::vector<__m128i> SinglePointCot::receiverExtend(
    std::vector<__m128i>&& baseCot) {
  assert(role_ == util::Role::receiver);
  expander_ = std::make_unique<util::Expander>(index_);
  cipherForHash_ = std::make_unique<util::Aes>(_mm_set_epi64x(index_, 0));

  std::vector<__m128i> rst{_mm_set_epi32(0, 0, 0, 0)};

  int64_t position = 0;

  // reconstruct the ggm tree. Only m_position is missing
  for (size_t i = 0; i < baseCot.size(); i++) {
    rst = constructALayerOfKeyForReceiver(std::move(rst), baseCot[i], position);
    position <<= 1;
    position ^= !util::getLsb(baseCot[i]);
  }
  // totalXor = delta + m_0 + m_1 + ...
  __m128i totalXor = agent_->receiveSingleT<__m128i>();

  rst[position] = _mm_set_epi64x(0, 0);
  for (size_t i = 0; i < rst.size(); i++) {
    util::setLsbTo0(rst[i]);
    totalXor = _mm_xor_si128(totalXor, rst[i]);
  }
  // totalXor = m_position + delta
  rst[position] = totalXor;
  index_++;

  return rst;
}

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::ferret
