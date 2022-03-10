/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <emmintrin.h>
#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/IBidirectionObliviousTransfer.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/IRandomCorrelatedObliviousTransfer.h"
#include "fbpcf/mpc_framework/engine/util/Masker.h"
#include "fbpcf/mpc_framework/engine/util/aes.h"
#include "fbpcf/mpc_framework/engine/util/util.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer {

/**
 * This is an Rcot based oblivious transfer object.
 */
template <class T>
class RcotBasedBidirectionObliviousTransfer final
    : public IBidirectionObliviousTransfer<T> {
 public:
  RcotBasedBidirectionObliviousTransfer(
      std::unique_ptr<communication::IPartyCommunicationAgent> agent,
      __m128i delta,
      std::unique_ptr<IRandomCorrelatedObliviousTransfer> senderRcot,
      std::unique_ptr<IRandomCorrelatedObliviousTransfer> receiverRcot);

  /**
   * @inherit doc
   */
  std::vector<T> biDirectionOT(
      const std::vector<T>& input0,
      const std::vector<T>& input1,
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

#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/RcotBasedBidirectionObliviousTransfer_impl.h"
