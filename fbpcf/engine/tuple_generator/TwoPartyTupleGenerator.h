/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emmintrin.h>
#include <deque>
#include <future>
#include <mutex>
#include <type_traits>

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
  std::map<size_t, std::vector<CompositeBooleanTuple>> getCompositeTuple(
      const std::map<size_t, uint32_t>& tupleSizes) override;

  /**
   * @inherit doc
   */
  std::pair<
      std::vector<BooleanTuple>,
      std::map<size_t, std::vector<CompositeBooleanTuple>>>
  getNormalAndCompositeBooleanTuples(
      uint32_t tupleSize,
      const std::map<size_t, uint32_t>& compositeTupleSizes) override;

  bool supportsCompositeTupleGeneration() override {
    return true;
  }

  /**
   * @inherit doc
   */
  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override;

 private:
  inline std::vector<BooleanTuple> generateNormalTuples(uint64_t size);
  inline std::vector<std::pair<__m128i, __m128i>> generateRcotResults(
      uint64_t size);

  template <bool isComposite>
  using TupleType = typename std::
      conditional<isComposite, CompositeBooleanTuple, BooleanTuple>::type;

  template <bool isComposite>
  std::vector<TupleType<isComposite>> expandRCOTResults(
      std::vector<__m128i> sender0Messages,
      std::vector<__m128i> receiverMessages,
      size_t requestedTupleSize // ignored if isComposite = false
  );

  enum ScheduledTupleType {
    Boolean,
    Composite,
  };

  util::Aes hashFromAes_;

  std::unique_ptr<oblivious_transfer::IRandomCorrelatedObliviousTransfer>
      senderRcot_;
  std::unique_ptr<oblivious_transfer::IRandomCorrelatedObliviousTransfer>
      receiverRcot_;
  __m128i delta_;

  std::mutex scheduleMutex_;
  std::condition_variable cv_;
  std::deque<ScheduledTupleType> toGenerate_;

  util::AsyncBuffer<BooleanTuple> booleanTupleBuffer_;
  util::AsyncBuffer<std::pair<__m128i, __m128i>> rcotBuffer_;
};

} // namespace fbpcf::engine::tuple_generator
