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
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/ISinglePointCot.h"
#include "fbpcf/mpc_framework/engine/util/IPrg.h"
#include "fbpcf/mpc_framework/engine/util/aes.h"
#include "fbpcf/mpc_framework/engine/util/util.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::ferret {

/**
 * This is a real single point COT. See https://eprint.iacr.org/2020/924.pdf for
 * more details
 */
class SinglePointCot final : public ISinglePointCot {
 public:
  explicit SinglePointCot(
      std::unique_ptr<communication::IPartyCommunicationAgent>& agent)
      : agent_(agent), index_(0) {}

  /**
   * @inherit doc
   */
  void senderInit(__m128i delta) override;
  /**
   * @inherit doc
   */
  void receiverInit() override;

  /**
   * @inherit doc
   */
  std::vector<__m128i> senderExtend(std::vector<__m128i>&& baseCot) override;

  /**
   * @inherit doc
   */
  std::vector<__m128i> receiverExtend(std::vector<__m128i>&& baseCot) override;

  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    // we are returning {0, 0} because this object doesn't own the agent.
    return {0, 0};
  }

 private:
  std::vector<__m128i> constructALayerOfKeyForSender(
      std::vector<__m128i>&& previousLayer,
      __m128i baseCot);
  std::vector<__m128i> constructALayerOfKeyForReceiver(
      std::vector<__m128i>&& previousLayer,
      __m128i baseCot,
      int missingPosition);

  std::unique_ptr<communication::IPartyCommunicationAgent>& agent_;

  std::unique_ptr<util::Expander> expander_;

  std::unique_ptr<util::Aes> cipherForHash_;

  util::Role role_;
  __m128i delta_;

  int64_t index_;
};

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::ferret
