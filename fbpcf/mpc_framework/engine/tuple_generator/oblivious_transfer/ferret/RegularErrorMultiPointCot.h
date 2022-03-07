/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <memory>
#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/IMultiPointCot.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/ISinglePointCot.h"
#include "fbpcf/mpc_framework/engine/util/IPrg.h"

namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer::
    ferret {

const int64_t kExtendedSize = 10805248;
const int64_t kWeight = 1319;
const int64_t kBaseSize = 589760;

/**
 * This object realize multi-point cot under the regular-error lpn assumption.
 * See https://eprint.iacr.org/2019/1159.pdf for more details.
 */
class RegularErrorMultiPointCot final : public IMultiPointCot {
 public:
  explicit RegularErrorMultiPointCot(
      std::unique_ptr<ISinglePointCot> singlePointCot)
      : singlePointCot_(std::move(singlePointCot)) {}

  /**
   * @inherit doc
   */
  void senderInit(__m128i delta, int64_t length, int64_t weight) override;

  /**
   * @inherit doc
   */
  void receiverInit(int64_t length, int64_t weight) override;

  /**
   * Return the base cot results needed per iteration.
   */
  int getBaseCotNeeds() const override {
    return baseCotSize_ * spcotCount_;
  }

  /**
   * @inherit doc
   */
  std::vector<__m128i> senderExtend(std::vector<__m128i>&& baseCot) override;

  /**
   * @inherit doc
   */
  std::vector<__m128i> receiverExtend(std::vector<__m128i>&& baseCot) override;

  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    return singlePointCot_->getTrafficStatistics();
  }

 private:
  /**
   * This is the initialization shared by both sender and receiver.
   */
  void init(int64_t length, int64_t weight);

  /**
   * This is merely a helper to unify the underlying single point cot API
   */
  std::vector<__m128i> singleCotExtend(std::vector<__m128i>&& baseCot) {
    if (role_ == util::Role::sender) {
      return singlePointCot_->senderExtend(std::move(baseCot));
    } else {
      return singlePointCot_->receiverExtend(std::move(baseCot));
    }
  }

  /**
   * This is merely a helper for avoiding duplicated code.
   */
  std::vector<__m128i> extend(std::vector<__m128i>&& baseCot);

  std::unique_ptr<ISinglePointCot> singlePointCot_;

  __m128i delta_;

  util::Role role_;
  int baseCotSize_;

  int64_t spcotLength_;
  int spcotCount_;
};

} // namespace
  // fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer::ferret
