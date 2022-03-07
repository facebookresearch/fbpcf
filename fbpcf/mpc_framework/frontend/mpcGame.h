/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <memory>
#include <type_traits>
#include <vector>
#include "fbpcf/mpc_framework/frontend/Bit.h"
#include "fbpcf/mpc_framework/frontend/Int.h"
#include "fbpcf/mpc_framework/frontend/util.h"
#include "fbpcf/mpc_framework/scheduler/IScheduler.h"

namespace fbpcf::mpc_framework::frontend {

template <int schedulerId>
class MpcGame {
 private:
  template <typename T, bool usingBatch>
  using BatchedType = typename std::conditional<usingBatch, Batch<T>, T>::type;

 public:
  explicit MpcGame(std::unique_ptr<scheduler::IScheduler> scheduler) {
    scheduler::SchedulerKeeper<schedulerId>::setScheduler(std::move(scheduler));
  }

  ~MpcGame() {
    scheduler::SchedulerKeeper<schedulerId>::freeScheduler();
  }

  /**
   * Get the total amount of traffic transmitted.
   * @return a pair of (sent, received) data in bytes.
   */
  std::pair<uint64_t, uint64_t> getTrafficStatistics() const {
    return scheduler::SchedulerKeeper<schedulerId>::getTrafficStatistics();
  }

 public:
  template <bool usingBatch = false>
  using PubBit = frontend::Bit<false, schedulerId, usingBatch>;

  template <bool usingBatch = false>
  using SecBit = frontend::Bit<true, schedulerId, usingBatch>;

  template <size_t width, bool usingBatch = false>
  using PubSignedInt = frontend::
      Integer<Public<BatchedType<Signed<width>, usingBatch>>, schedulerId>;
  template <size_t width, bool usingBatch = false>
  using SecSignedInt =
      Integer<Secret<BatchedType<Signed<width>, usingBatch>>, schedulerId>;

  template <size_t width, bool usingBatch = false>
  using PubUnsignedInt = frontend::
      Integer<Public<BatchedType<Unsigned<width>, usingBatch>>, schedulerId>;
  template <size_t width, bool usingBatch = false>
  using SecUnsignedInt =
      Integer<Secret<BatchedType<Unsigned<width>, usingBatch>>, schedulerId>;
};

} // namespace fbpcf::mpc_framework::frontend
