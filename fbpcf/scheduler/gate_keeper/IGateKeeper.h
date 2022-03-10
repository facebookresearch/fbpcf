/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/scheduler/IScheduler.h"
#include "fbpcf/scheduler/gate_keeper/ICompositeGate.h"
#include "fbpcf/scheduler/gate_keeper/IGate.h"
#include "fbpcf/scheduler/gate_keeper/INormalGate.h"

namespace fbpcf::scheduler {

/**
 * This object stores gates for a circuit that have yet to be executed.
 */
class IGateKeeper {
 public:
  template <bool usingBatch>
  using BoolType = typename std::
      conditional<usingBatch, const std::vector<bool>&, bool>::type;

  virtual ~IGateKeeper() = default;

  // Create an input gate and return its output wire ID.
  virtual IScheduler::WireId<IScheduler::Boolean> inputGate(
      BoolType<false> initialValue) = 0;

  // Create a batch input gate and return its output wire ID.
  virtual IScheduler::WireId<IScheduler::Boolean> inputGateBatch(
      BoolType<true> initialValue) = 0;

  // Create an output gate and return its output wire ID.
  virtual IScheduler::WireId<IScheduler::Boolean> outputGate(
      IScheduler::WireId<IScheduler::Boolean> src,
      int partyID) = 0;

  // Create a batch output gate and return its output wire ID.
  virtual IScheduler::WireId<IScheduler::Boolean> outputGateBatch(
      IScheduler::WireId<IScheduler::Boolean> src,
      int partyID) = 0;

  // Create a unary/binary gate (e.g. AND, XOR, NOT) and return its output wire
  // ID.
  virtual IScheduler::WireId<IScheduler::Boolean> normalGate(
      INormalGate<IScheduler::Boolean>::GateType gateType,
      IScheduler::WireId<IScheduler::Boolean> left,
      IScheduler::WireId<IScheduler::Boolean> right =
          IScheduler::WireId<IScheduler::Boolean>()) = 0;

  // Create a batch unary/binary gate (e.g. AND, XOR, NOT) and return its output
  // wire ID.
  virtual IScheduler::WireId<IScheduler::Boolean> normalGateBatch(
      INormalGate<IScheduler::Boolean>::GateType gateType,
      IScheduler::WireId<IScheduler::Boolean> left,
      IScheduler::WireId<IScheduler::Boolean> right =
          IScheduler::WireId<IScheduler::Boolean>()) = 0;

  // Create a binary composite gate (e.g. AND, XOR) from a single left input and
  // multiple right inputs. Returns its output wire ID's
  virtual std::vector<IScheduler::WireId<IScheduler::Boolean>> compositeGate(
      ICompositeGate::GateType gateType,
      IScheduler::WireId<IScheduler::Boolean> left,
      std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) = 0;

  // Create a batch binary composite gate (e.g. AND, XOR) from a single left
  // input and multiple right inputs. Returns its output wire ID's
  virtual std::vector<IScheduler::WireId<IScheduler::Boolean>>
  compositeGateBatch(
      ICompositeGate::GateType gateType,
      IScheduler::WireId<IScheduler::Boolean> left,
      std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) = 0;

  // Return the first level of gates that has not been executed yet.
  // NOTE: Free gates are added to even levels, and non-free gates are added
  // to odd levels.
  virtual uint32_t getFirstUnexecutedLevel() const = 0;

  // Extract all the gates at the level that should be executed next.
  virtual std::vector<std::unique_ptr<IGate>> popFirstUnexecutedLevel() = 0;

  // Whether we've exceeded the maximum number of unexecuted gates. In this
  // case, gates should be executed in order to free up memory.
  virtual bool hasReachedBatchingLimit() const = 0;

  // Even levels contain free gates, and odd levels contain non-free gates.
  static inline bool isLevelFree(uint32_t level) {
    return !(level & 1);
  }
};

} // namespace fbpcf::scheduler
