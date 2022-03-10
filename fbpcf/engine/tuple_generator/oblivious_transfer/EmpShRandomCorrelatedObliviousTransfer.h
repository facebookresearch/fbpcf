/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emmintrin.h>
#include <emp-ot/emp-ot.h>
#include <array>
#include "fbpcf/engine/tuple_generator/oblivious_transfer/IFlexibleRandomCorrelatedObliviousTransfer.h"
#include "fbpcf/engine/util/EmpNetworkAdapter.h"
#include "fbpcf/engine/util/IPrg.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer {

/**
 * This object is a wrapper that align our RCOT APIs and emp::IKNP APIs.
 */

class EmpShRandomCorrelatedObliviousTransfer final
    : public IFlexibleRandomCorrelatedObliviousTransfer {
 public:
  // sender constructor
  EmpShRandomCorrelatedObliviousTransfer(
      __m128i delta,
      std::unique_ptr<communication::IPartyCommunicationAgent> agent);

  // receiver constructor
  EmpShRandomCorrelatedObliviousTransfer(
      std::unique_ptr<communication::IPartyCommunicationAgent> agent,
      std::unique_ptr<util::IPrg> prg);

  /**
   * @inherit doc
   */
  std::vector<__m128i> rcot(int64_t size) override;

  /**
   * @inherit doc
   */
  std::unique_ptr<communication::IPartyCommunicationAgent>
  extractCommunicationAgent() override {
    return std::move(agent_);
  }

  /**
   * @inherit doc
   */
  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    auto traffic1 = io_->getTrafficStatistics();
    auto traffic2 = agent_->getTrafficStatistics();
    return {traffic1.first + traffic2.first, traffic1.second + traffic2.second};
  }

 private:
  util::Role role_;

  std::unique_ptr<emp::IKNP<util::EmpNetworkAdapter>> shot_;
  std::unique_ptr<util::EmpNetworkAdapter> io_;

  std::unique_ptr<communication::IPartyCommunicationAgent> agent_;
  std::unique_ptr<util::IPrg> prg_;
};

} // namespace fbpcf::engine::tuple_generator::oblivious_transfer
