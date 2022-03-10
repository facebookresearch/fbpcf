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
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/IBaseObliviousTransfer.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/IFlexibleRandomCorrelatedObliviousTransfer.h"
#include "fbpcf/mpc_framework/engine/util/EmpNetworkAdapter.h"
#include "fbpcf/mpc_framework/engine/util/IPrgFactory.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer {

/**
 * This object is an in-house implementation of IKNP protocol
 * [REF] Implementation of "Extending oblivious transfers efficiently"
 * https://www.iacr.org/archive/crypto2003/27290145/27290145.pdf with
 * optimizations from [REF] With optimization of "More Efficient Oblivious
 * Transfer and Extensions for Faster Secure Computation"
 * https://eprint.iacr.org/2013/552.pdf
 * [REF] With optimization of "Better Concrete Security for Half-Gates Garbling
 * (in the Multi-Instance Setting)" https://eprint.iacr.org/2019/1168.pdf
 */

class IknpShRandomCorrelatedObliviousTransfer
    : public IFlexibleRandomCorrelatedObliviousTransfer {
 public:
  // sender constructor
  IknpShRandomCorrelatedObliviousTransfer(
      __m128i delta,
      std::unique_ptr<IBaseObliviousTransfer> baseOt,
      std::unique_ptr<util::IPrgFactory> prgFactory);

  // receiver constructor
  IknpShRandomCorrelatedObliviousTransfer(
      std::unique_ptr<IBaseObliviousTransfer> baseOt,
      std::unique_ptr<util::IPrgFactory> prgFactory);

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
    return agent_->getTrafficStatistics();
  }

 protected:
  static std::vector<__m128i> matrixTranspose(const std::vector<__m128i>& src);

 private:
  util::Role role_;

  std::unique_ptr<communication::IPartyCommunicationAgent> agent_;

  std::vector<bool> decomposedDelta_;
  std::vector<std::unique_ptr<util::IPrg>> senderPrgs_;

  std::vector<std::unique_ptr<util::IPrg>> receiverPrgs0_;
  std::vector<std::unique_ptr<util::IPrg>> receiverPrgs1_;
  std::unique_ptr<util::IPrg> choiceBitPrg_;
};

} // namespace fbpcf::engine::tuple_generator::oblivious_transfer
