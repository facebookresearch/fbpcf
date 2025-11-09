/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <assert.h>
#include <emmintrin.h>
#include <cstring>

#include <iterator>
#include <locale>
#include <memory>

#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/RcotExtender.h"
#include "fbpcf/engine/util/util.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::ferret {

RcotExtender::RcotExtender(
    std::unique_ptr<IMatrixMultiplier> MatrixMultiplier,
    IMultiPointCotFactory& multiPointCotFactory)
    : MatrixMultiplier_(std::move(MatrixMultiplier)),
      multiPointCot_(multiPointCotFactory.create(agent_)) {}

int RcotExtender::senderInit(
    __m128i delta,
    int64_t extendedSize,
    int64_t baseSize,
    int64_t weight) {
  role_ = util::Role::sender;
  extendedSize_ = extendedSize;

  multiPointCot_->senderInit(delta, extendedSize_, weight);

  matrixMultiplicationBaseRcotSize_ = baseSize;
  mpcotBaseRcotSize_ = multiPointCot_->getBaseCotNeeds();

  baseCotSize_ = matrixMultiplicationBaseRcotSize_ + mpcotBaseRcotSize_;
  return baseCotSize_;
}

int RcotExtender::receiverInit(
    int64_t extendedSize,
    int64_t baseSize,
    int64_t weight) {
  role_ = util::Role::receiver;
  extendedSize_ = extendedSize;

  multiPointCot_->receiverInit(extendedSize_, weight);

  matrixMultiplicationBaseRcotSize_ = baseSize;
  mpcotBaseRcotSize_ = multiPointCot_->getBaseCotNeeds();
  baseCotSize_ = matrixMultiplicationBaseRcotSize_ + mpcotBaseRcotSize_;
  return baseCotSize_;
}

std::vector<__m128i> RcotExtender::senderExtendRcot(
    std::vector<__m128i>&& baseCot) {
  assert(baseCot.size() == baseCotSize_);

  auto seed = agent_->receiveSingleT<__m128i>();

  return extendRcot(seed, std::move(baseCot));
}

std::vector<__m128i> RcotExtender::receiverExtendRcot(
    std::vector<__m128i>&& baseCot) {
  assert(
      baseCot.size() == matrixMultiplicationBaseRcotSize_ + mpcotBaseRcotSize_);

  auto seed = util::getRandomM128iFromSystemNoise();

  agent_->sendSingleT<__m128i>(seed);

  return extendRcot(seed, std::move(baseCot));
}

std::vector<__m128i> RcotExtender::extendRcot(
    __m128i seed,
    std::vector<__m128i>&& baseCot) {
  auto matrixMultiplicationResult = MatrixMultiplier_->multiplyWithRandomMatrix(
      seed,
      extendedSize_,
      std::vector<__m128i>(
          std::make_move_iterator(baseCot.begin()),
          std::make_move_iterator(
              baseCot.begin() + matrixMultiplicationBaseRcotSize_)));
  std::vector<__m128i> mpCotResult;
  if (role_ == util::Role::sender) {
    mpCotResult = multiPointCot_->senderExtend(std::vector<__m128i>(
        std::make_move_iterator(baseCot.end() - mpcotBaseRcotSize_),
        std::make_move_iterator(baseCot.end())));
  } else {
    mpCotResult = multiPointCot_->receiverExtend(std::vector<__m128i>(
        std::make_move_iterator(baseCot.end() - mpcotBaseRcotSize_),
        std::make_move_iterator(baseCot.end())));
  }
  assert(mpCotResult.size() == matrixMultiplicationResult.size());
  for (size_t i = 0; i < matrixMultiplicationResult.size(); i++) {
    matrixMultiplicationResult[i] =
        _mm_xor_si128(matrixMultiplicationResult[i], mpCotResult[i]);
  }
  return matrixMultiplicationResult;
}

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::ferret
