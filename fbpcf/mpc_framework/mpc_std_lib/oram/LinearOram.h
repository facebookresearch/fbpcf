/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_framework/engine/util/IPrg.h"
#include "fbpcf/mpc_framework/mpc_std_lib/oram/ISinglePointArrayGenerator.h"
#include "fbpcf/mpc_framework/mpc_std_lib/oram/IWriteOnlyOram.h"
#include "fbpcf/mpc_framework/mpc_std_lib/util/util.h"

namespace fbpcf::mpc_framework::mpc_std_lib::oram {

/**
 *A linear ORAM behaviors the same way as the ideal ORAM functionality when
 *viewed from outside in a secure manner. However, it's implementation is
 *pretty much straightforward inside. A linear ORAM will traverse all the
 *available index and compare with the input secret index to decide whether to
 *perform a dummy operation (e.g. add 0 to a running sum) or a meaningful one
 *(e.g. add the secret input to the running sum). Usually this is a very
 *inefficient approach with any meaningful ORAM size. However it will outperform
 *other implementations when the ORAM size is extremely small.

 **/

template <typename T, int schedulerId>
class LinearOram final : public IWriteOnlyOram<T> {
  using Role = typename IWriteOnlyOram<T>::Role;

 public:
  LinearOram(
      size_t size,
      Role myRole,
      int32_t party0Id,
      int32_t party1Id,
      std::unique_ptr<engine::communication::IPartyCommunicationAgent> agent,
      std::unique_ptr<engine::util::IPrg> prg)
      : size_(size),
        myRole_(myRole),
        party0Id_(party0Id),
        party1Id_(party1Id),
        agent_(std::move(agent)),
        prg_(std::move(prg)),
        memory_(size_, T(0)) {}

  /**
   * @inherit doc
   */
  T publicRead(size_t publicIndex, Role receiver) const override;

  /**
   * @inherit doc
   */
  T secretRead(size_t publicIndex) const override;

  /**
   * @inherit doc
   */
  void obliviousAddBatch(
      const std::vector<std::vector<bool>>& indexShares,
      const std::vector<std::vector<bool>>& values) override;

  /**
   * @inherit doc
   */
  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    return agent_->getTrafficStatistics();
  }

  class Helper {
   public:
    // generate the index of all ORAM slots (e.g. 0, 1, 2, 3,..) in batched
    // format. The output is a vector of vector of indexes, each element in the
    // output vector is the (batched) bit vector of "0", "1", "2", ..."range -
    // 1".
    static std::vector<std::vector<frontend::Bit<false, schedulerId, true>>>
    generateIndex(size_t batchSize, size_t range);

    // Compare (in batch) if there are any difference between a secret bit
    // vector and public bit vector
    static frontend::Bit<true, schedulerId, true> comparison(
        const std::vector<frontend::Bit<true, schedulerId, true>>& src1,
        const std::vector<frontend::Bit<false, schedulerId, true>>& src2);
  };

 private:
  size_t size_;

  Role myRole_;
  int32_t party0Id_;
  int32_t party1Id_;

  std::unique_ptr<engine::communication::IPartyCommunicationAgent> agent_;
  std::unique_ptr<engine::util::IPrg> prg_;

  std::vector<T> memory_;
};

} // namespace fbpcf::mpc_framework::mpc_std_lib::oram

#include "fbpcf/mpc_framework/mpc_std_lib/oram/LinearOram_impl.h"
