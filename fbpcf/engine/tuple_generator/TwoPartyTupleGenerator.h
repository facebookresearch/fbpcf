/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <future>

#include "fbpcf/engine/tuple_generator/ITupleGenerator.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/IRandomCorrelatedObliviousTransfer.h"
#include "fbpcf/engine/util/AsyncBuffer.h"
#include "fbpcf/engine/util/aes.h"

namespace fbpcf::engine::tuple_generator {

class TwoPartyTupleGenerator final : public ITupleGenerator {
 public:
  TwoPartyTupleGenerator(
      std::unique_ptr<oblivious_transfer::IRandomCorrelatedObliviousTransfer>
          senderRcot,
      std::unique_ptr<oblivious_transfer::IRandomCorrelatedObliviousTransfer>
          receiverRcot,
      __m128i delta,
      uint64_t bufferSize = kDefaultBufferSize);

  /**
   * @inherit doc
   */
  std::vector<BooleanTuple> getBooleanTuple(uint32_t size) override;

  /**
   * @inherit doc
   */
  std::unordered_map<size_t, std::vector<CompositeBooleanTuple>>
  getCompositeTuple(std::unordered_map<size_t, uint32_t>& tupleSizes) override;

  /**
   * @inherit doc
   */
  std::pair<
      std::vector<BooleanTuple>,
      std::unordered_map<size_t, std::vector<CompositeBooleanTuple>>>
  getNormalAndCompositeBooleanTuples(
      uint32_t tupleSize,
      std::unordered_map<size_t, uint32_t>& compositeTupleSizes) override;

  /**
   * @inherit doc
   */
  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override;

 private:
  inline std::vector<BooleanTuple> generateTuples(uint64_t size);

  util::Aes hashFromAes_;

  std::unique_ptr<oblivious_transfer::IRandomCorrelatedObliviousTransfer>
      senderRcot_;
  std::unique_ptr<oblivious_transfer::IRandomCorrelatedObliviousTransfer>
      receiverRcot_;
  __m128i delta_;

  util::AsyncBuffer<BooleanTuple> buffer_;
};

} // namespace fbpcf::engine::tuple_generator
