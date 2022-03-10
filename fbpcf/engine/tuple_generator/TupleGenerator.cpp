/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <assert.h>

#include "fbpcf/engine/tuple_generator/TupleGenerator.h"

namespace fbpcf::engine::tuple_generator {

TupleGenerator::TupleGenerator(
    std::map<int, std::unique_ptr<IProductShareGenerator>>&&
        productShareGeneratorMap,
    std::unique_ptr<util::IPrg> prg,
    uint64_t bufferSize)
    : productShareGeneratorMap_{std::move(productShareGeneratorMap)},
      prg_{std::move(prg)},
      asyncBuffer_{bufferSize, [this](uint64_t size) {
                     return generateTuples(size);
                   }} {}

std::vector<ITupleGenerator::BooleanTuple> TupleGenerator::getBooleanTuple(
    uint32_t size) {
  return asyncBuffer_.getData(size);
}

/**
 * Tuple generation algorithm:
 * we want to achieve (a1 + a2 + a3 +... + an) * (b1 + b2 + b3 +...+ bn) =
 * (c1 + c2 + c3 +...+ cn)
 * Party i and j will randomly choose ai, bi and aj, bj and use the product
 * share generator to generate shares of aibj+ajbi
 */
std::vector<TupleGenerator::BooleanTuple> TupleGenerator::generateTuples(
    uint64_t size) {
  auto vectorA = prg_->getRandomBits(size);
  auto vectorB = prg_->getRandomBits(size);
  std::vector<bool> vectorC(size, false);

  for (auto& item : productShareGeneratorMap_) {
    auto shares = item.second->generateBooleanProductShares(vectorA, vectorB);
    assert(shares.size() == size);
    for (size_t i = 0; i < size; i++) {
      vectorC[i] = vectorC[i] ^ shares[i];
    }
  }

  std::vector<TupleGenerator::BooleanTuple> booleanTuples(size);
  for (size_t i = 0; i < size; i++) {
    booleanTuples[i] = BooleanTuple(
        vectorA[i], vectorB[i], (vectorA[i] & vectorB[i]) ^ vectorC[i]);
  }
  return booleanTuples;
}

std::pair<uint64_t, uint64_t> TupleGenerator::getTrafficStatistics() const {
  std::pair<uint64_t, uint64_t> rst = {0, 0};
  for (auto& item : productShareGeneratorMap_) {
    auto cost = item.second->getTrafficStatistics();
    rst.first += cost.first;
    rst.second += cost.second;
  }
  return rst;
}

} // namespace fbpcf::engine::tuple_generator
