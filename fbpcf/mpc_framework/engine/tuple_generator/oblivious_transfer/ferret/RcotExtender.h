/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emmintrin.h>
#include <memory>

#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/IMatrixMultiplier.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/IMultiPointCotFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/IRcotExtender.h"

namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer::
    ferret {

/**
 * This is a COT extender in hybrid of an abstract LPN calculator and
 * Multi-point COT. This object's security guarantee depends on these two
 * underlying objects.
 */
class RcotExtender final : public IRcotExtender {
 public:
  RcotExtender(
      std::unique_ptr<IMatrixMultiplier> MatrixMultiplier,
      IMultiPointCotFactory& multiPointCotFactory);

  /**
   * @inherit doc
   */
  int senderInit(
      __m128i delta,
      int64_t extendedSize,
      int64_t baseSize,
      int64_t weight) override;

  /**
   * @inherit doc
   */
  int receiverInit(int64_t extendedSize, int64_t baseSize, int64_t weight)
      override;

  /**
   * @inherit doc
   */
  void setCommunicationAgent(
      std::unique_ptr<communication::IPartyCommunicationAgent> agent) override {
    agent_ = std::move(agent);
  }

  /**
   * Indicates how many base COT are needed for extension.
   */
  int getBaseCotSize() const {
    return baseCotSize_;
  }

  /**
   * @inherit doc
   */
  std::vector<__m128i> senderExtendRcot(
      std::vector<__m128i>&& baseCot) override;

  /**
   * @inherit doc
   */
  std::vector<__m128i> receiverExtendRcot(
      std::vector<__m128i>&& baseCot) override;

  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    return agent_->getTrafficStatistics();
  }

 private:
  std::vector<__m128i> extendRcot(__m128i seed, std::vector<__m128i>&& baseCot);

  std::unique_ptr<communication::IPartyCommunicationAgent> agent_;
  std::unique_ptr<IMatrixMultiplier> MatrixMultiplier_;
  std::unique_ptr<IMultiPointCot> multiPointCot_;

  util::Role role_;
  int64_t extendedSize_;
  int64_t mpcotBaseRcotSize_;
  int64_t matrixMultiplicationBaseRcotSize_;

  int64_t baseCotSize_;
  std::vector<__m128i> baseCot_;
};

} // namespace
  // fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer::ferret
