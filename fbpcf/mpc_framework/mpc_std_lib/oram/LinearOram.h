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

 private:
  size_t size_;

  Role myRole_;
  int32_t party0Id_;
  int32_t party1Id_;

  std::unique_ptr<engine::communication::IPartyCommunicationAgent> agent_;
  std::unique_ptr<engine::util::IPrg> prg_;

  std::vector<T> memory_;

  using SecBatchT = typename util::SecBatchType<T, schedulerId>::type;

  /**
   * create a vector of values where only a certain position is the given value
   * and the rest of them are zero.
   * @param src the value in the output
   * @param conditions indicate the position of the given value in the output
   * @param range how many items are there in the output
   * @param batchSize how many actual items in the batches.
   */
  std::vector<SecBatchT> generateInputValue(
      SecBatchT&& src,
      const std::vector<frontend::Bit<true, schedulerId, true>>&& conditions,
      size_t range,
      size_t batchSize) const;

  /**
   * expand each element in the input into two elements in the output, one of
   * the two elements is the corresponding input element while the other one is
   * zero.
   * @param src the input vector
   * @param zero representation of zero
   * @param condition whether the first or the second is going to be the input
   * value: 0 means the first, 1 means the second
   */
  std::vector<SecBatchT> conditionalExpansionOneLayer(
      std::vector<SecBatchT>&& src,
      const SecBatchT& zero,
      const frontend::Bit<true, schedulerId, true>& condition) const;
};

} // namespace fbpcf::mpc_framework::mpc_std_lib::oram

#include "fbpcf/mpc_framework/mpc_std_lib/oram/LinearOram_impl.h"
