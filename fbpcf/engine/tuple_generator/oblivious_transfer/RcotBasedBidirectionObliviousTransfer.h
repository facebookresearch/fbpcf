/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <emmintrin.h>
#include <cstdint>
#include "fbpcf/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/IBidirectionObliviousTransfer.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/IRandomCorrelatedObliviousTransfer.h"
#include "fbpcf/engine/util/Masker.h"
#include "fbpcf/engine/util/aes.h"
#include "fbpcf/engine/util/util.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer {

/**
 * This is an Rcot based oblivious transfer object.
 */
class RcotBasedBidirectionObliviousTransfer final
    : public IBidirectionObliviousTransfer {
 public:
  RcotBasedBidirectionObliviousTransfer(
      std::unique_ptr<communication::IPartyCommunicationAgent> agent,
      __m128i delta,
      std::unique_ptr<IRandomCorrelatedObliviousTransfer> senderRcot,
      std::unique_ptr<IRandomCorrelatedObliviousTransfer> receiverRcot);

  /**
   * @inherit doc
   */
  std::vector<bool> biDirectionOT(
      const std::vector<bool>& input0,
      const std::vector<bool>& input1,
      const std::vector<bool>& choice) override;

  /**
   * @inherit doc
   */
  std::vector<uint64_t> biDirectionOT(
      const std::vector<uint64_t>& input0,
      const std::vector<uint64_t>& input1,
      const std::vector<bool>& choice) override;

  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override;

 private:
  // this cipher is merely used for instantiate a hash h(x) = \pi(x) xor x
  util::Aes hashFromAes_;

  std::unique_ptr<communication::IPartyCommunicationAgent> agent_;
  __m128i delta_;
  std::unique_ptr<IRandomCorrelatedObliviousTransfer> senderRcot_;
  std::unique_ptr<IRandomCorrelatedObliviousTransfer> receiverRcot_;
};

} // namespace fbpcf::engine::tuple_generator::oblivious_transfer
