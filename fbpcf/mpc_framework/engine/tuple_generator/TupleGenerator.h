/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <future>
#include <map>
#include <memory>

#include "fbpcf/mpc_framework/engine/tuple_generator/IProductShareGenerator.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/ITupleGenerator.h"
#include "fbpcf/mpc_framework/engine/util/AsyncBuffer.h"
#include "fbpcf/mpc_framework/engine/util/IPrg.h"

namespace fbpcf::mpc_framework::engine::tuple_generator {

/**
 * This object uses product share generators to generate tuples.
 */
class TupleGenerator final : public ITupleGenerator {
 public:
  /**
   * @param productShareGeneratorMap the underlying product share
   * generators, the size indicates total number of parties.
   * @param prg a prg to generate randomness
   * @param bufferSize how many tuples to buffer in the memory.
   */
  TupleGenerator(
      std::map<int, std::unique_ptr<IProductShareGenerator>>&&
          productShareGeneratorMap,
      std::unique_ptr<util::IPrg> prg,
      uint64_t bufferSize = kDefaultBufferSize);

  /**
   * @inherit doc
   */
  std::vector<BooleanTuple> getBooleanTuple(uint32_t size) override;

  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override;

 private:
  inline std::vector<BooleanTuple> generateTuples(uint64_t size);

  std::map<int, std::unique_ptr<IProductShareGenerator>>
      productShareGeneratorMap_;
  std::unique_ptr<util::IPrg> prg_;

  util::AsyncBuffer<BooleanTuple> asyncBuffer_;
};

} // namespace fbpcf::mpc_framework::engine::tuple_generator
