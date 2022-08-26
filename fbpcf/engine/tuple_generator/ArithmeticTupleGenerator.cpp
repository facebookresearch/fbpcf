/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/engine/tuple_generator/ArithmeticTupleGenerator.h"

namespace fbpcf::engine::tuple_generator {
ArithmeticTupleGenerator::ArithmeticTupleGenerator(
    std::map<int, std::unique_ptr<IProductShareGenerator>>&&
        productShareGeneratorMap,
    std::unique_ptr<util::IPrg> prg,
    uint64_t bufferSize)
    : productShareGeneratorMap_{std::move(productShareGeneratorMap)},
      prg_{std::move(prg)},
      asyncBuffer_{bufferSize, [this](uint64_t size) {
                     return std::async(
                         [this](uint64_t size) { return generateTuples(size); },
                         size);
                   }} {}

std::vector<IArithmeticTupleGenerator::IntegerTuple>
ArithmeticTupleGenerator::getIntegerTuple(uint32_t size) {
  return asyncBuffer_.getData(size);
}

/**
 * Tuple generation algorithm:
 * we want to achieve (a1 + a2 + a3 +... + an) * (b1 + b2 + b3 +...+ bn) =
 * (c1 + c2 + c3 +...+ cn)
 * Party i and j will randomly choose ai, bi and aj, bj and use the product
 * share generator to generate shares of aibj+ajbi
 */
std::vector<ArithmeticTupleGenerator::IntegerTuple>
ArithmeticTupleGenerator::generateTuples(uint64_t size) {
  auto vectorA = prg_->getRandomUInt64(size);
  auto vectorB = prg_->getRandomUInt64(size);
  std::vector<uint64_t> vectorC(size, 0);

  for (auto& item : productShareGeneratorMap_) {
    auto shares = item.second->generateIntegerProductShares(vectorA, vectorB);
    assert(shares.size() == size);
    for (size_t i = 0; i < size; i++) {
      vectorC[i] = vectorC[i] + shares[i];
    }
  }

  std::vector<ArithmeticTupleGenerator::IntegerTuple> integerTuples(size);
  for (size_t i = 0; i < size; i++) {
    integerTuples[i] = IntegerTuple(
        vectorA[i], vectorB[i], (vectorA[i] * vectorB[i]) + vectorC[i]);
  }
  return integerTuples;
}

std::pair<uint64_t, uint64_t> ArithmeticTupleGenerator::getTrafficStatistics()
    const {
  std::pair<uint64_t, uint64_t> rst = {0, 0};
  for (auto& item : productShareGeneratorMap_) {
    auto cost = item.second->getTrafficStatistics();
    rst.first += cost.first;
    rst.second += cost.second;
  }
  return rst;
}
} // namespace fbpcf::engine::tuple_generator
