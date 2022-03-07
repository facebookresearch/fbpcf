/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <assert.h>

#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/IRandomCorrelatedObliviousTransfer.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/IRcotExtender.h"

namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer {

/**
 * A Random Correlated Oblivious Transfer from a RCOT extender. This object
 * securely realize RCOT with a secure RCOT extender.
 */

class ExtenderBasedRandomCorrelatedObliviousTransfer final
    : public IRandomCorrelatedObliviousTransfer {
 public:
  ExtenderBasedRandomCorrelatedObliviousTransfer(
      util::Role role,
      std::unique_ptr<ferret::IRcotExtender> rcotExtender);

  // get how many base RCOT results are needed to bootstrapping the underlying
  // extender.
  int getNumberOfBaseRcotResultsNeeded() const {
    return baseRcotSize_;
  }

  // set the base RCOT results, this function only needs to be called once. This
  // object will automatically reserve some of the extended RCOT for future
  // iteration.
  void setBaseRcotResults(std::vector<__m128i>&& baseRcotResults) {
    assert(baseRcotResults.size() == baseRcotSize_);
    assert(baseRcotResults_.size() == 0);
    baseRcotResults_ = std::move(baseRcotResults);
    extendRcot();
  }

  /**
   * @inherit doc
   */
  std::vector<__m128i> rcot(int64_t size) override;

  /**
   * @inherit doc
   */
  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    return rcotExtender_->getTrafficStatistics();
  }

 private:
  void extendRcot();
  util::Role role_;

  int64_t baseRcotSize_;
  std::unique_ptr<ferret::IRcotExtender> rcotExtender_;

  // base RCOT for future iterations
  std::vector<__m128i> baseRcotResults_;

  // buffered RCOT results
  std::vector<__m128i> rcotResults_;
  int64_t otIndex_;
};

} // namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer
