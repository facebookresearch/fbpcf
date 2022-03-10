/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <memory>
#include "fbpcf/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/IMultiPointCot.h"
#include "fbpcf/engine/util/IPrg.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::ferret::insecure {

/**
 * A Dummy MPCOT implementation. "Dummy" means it realize the desired
 * functionality but in an insecure way.
 */

class DummyMultiPointCot final : public IMultiPointCot {
 public:
  // we use a reference for communication agent since this agent will be shared
  // across multiple instances by the underlying SPCOT. We use a reference of
  // the pointer, instead of the object itself, because the agent owned by the
  // extender may haven't been set yet.
  DummyMultiPointCot(
      std::unique_ptr<communication::IPartyCommunicationAgent>& agent,
      std::unique_ptr<util::IPrg> prg)
      : agent_(agent), prg_(std::move(prg)) {}

  /**
   * @inherit doc
   */
  void senderInit(__m128i delta, int64_t length, int64_t weight) override;

  /**
   * @inherit doc
   */
  void receiverInit(int64_t length, int64_t weight) override;

  int getBaseCotNeeds() const override {
    return 0;
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
    // we are returning {0, 0} because this object doesn't own the agent.
    return {0, 0};
  }

 private:
  std::vector<int64_t> getRandomPositions();

  std::unique_ptr<communication::IPartyCommunicationAgent>& agent_;
  std::unique_ptr<util::IPrg> prg_;

  util::Role role_;
  __m128i delta_;
  int64_t length_;
  int64_t weight_;
};

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::ferret::insecure
