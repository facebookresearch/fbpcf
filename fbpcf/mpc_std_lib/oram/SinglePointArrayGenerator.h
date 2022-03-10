/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emmintrin.h>
#include <cstdint>
#include <memory>
#include <vector>

#include "fbpcf/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/engine/util/util.h"
#include "fbpcf/mpc_std_lib/oram/IObliviousDeltaCalculator.h"
#include "fbpcf/mpc_std_lib/oram/ISinglePointArrayGenerator.h"

namespace fbpcf::mpc_std_lib::oram {

/**
 * A single point array generator allow two parties jointly generate a pair of
 * single point array. The point position is shared by two parties.
 * This object uses an oblivious delta calculator as an underlying object.
 */
class SinglePointArrayGenerator final : public ISinglePointArrayGenerator {
 public:
  SinglePointArrayGenerator(
      bool firstShare /* which value to start with when generating the
                         array, the two parties must use different values*/
      ,
      std::unique_ptr<IObliviousDeltaCalculator> obliviousDeltaCalculator)
      : firstShare_(firstShare),
        obliviousDeltaCalculator_(std::move(obliviousDeltaCalculator)) {
    expander_ = std::make_unique<engine::util::Expander>(
        0 /* this index is not important, any PUBLIC CONSTANT works*/);
  }

  /**
   * @inherit doc
   */
  std::vector<std::pair<std::vector<bool>, std::vector<__m128i>>>
  generateSinglePointArrays(
      const std::vector<std::vector<bool>>& indexShares,
      size_t length) override;

  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    return obliviousDeltaCalculator_->getTrafficStatistics();
  }

 private:
  ArrayType expandArray(
      ArrayType&& src,
      const std::vector<bool>& indicatorShare) const;

  bool firstShare_;
  std::unique_ptr<IObliviousDeltaCalculator> obliviousDeltaCalculator_;
  std::unique_ptr<engine::util::Expander> expander_;
};

} // namespace fbpcf::mpc_std_lib::oram
