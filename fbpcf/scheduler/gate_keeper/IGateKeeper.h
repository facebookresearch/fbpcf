/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include "fbpcf/scheduler/IScheduler.h"
#include "fbpcf/scheduler/gate_keeper/IArithmeticGate.h"
#include "fbpcf/scheduler/gate_keeper/ICompositeGate.h"
#include "fbpcf/scheduler/gate_keeper/IGate.h"
#include "fbpcf/scheduler/gate_keeper/INormalGate.h"
#include "fbpcf/scheduler/gate_keeper/RebatchingGate.h"

namespace fbpcf::scheduler {

/**
 * This object stores gates for a circuit that have yet to be executed.
 */
class IGateKeeper {
 public:
  template <bool usingBatch>
  using BoolType = typename std::
      conditional<usingBatch, const std::vector<bool>&, bool>::type;
  template <bool usingBatch>
  using IntType = typename std::
      conditional<usingBatch, const std::vector<uint64_t>&, uint64_t>::type;

  virtual ~IGateKeeper() = default;

  // Create a boolean input gate and return its output wire ID.
  virtual IScheduler::WireId<IScheduler::Boolean> inputGate(
      BoolType<false> initialValue) = 0;

  // Create an arithmetic input gate and return its output wire ID.
  virtual IScheduler::WireId<IScheduler::Arithmetic> inputGate(
      IntType<false> initialValue) = 0;

  // Create a batch boolean input gate and return its output wire ID.
  virtual IScheduler::WireId<IScheduler::Boolean> inputGateBatch(
      BoolType<true> initialValue) = 0;

  // Create a batch arithmetic input gate and return its output wire ID.
  virtual IScheduler::WireId<IScheduler::Arithmetic> inputGateBatch(
      IntType<true> initialValue) = 0;

  // Create a boolean output gate and return its output wire ID.
  virtual IScheduler::WireId<IScheduler::Boolean> outputGate(
      IScheduler::WireId<IScheduler::Boolean> src,
      int partyID) = 0;

  // Create an arithmetic output gate and return its output wire ID.
  virtual IScheduler::WireId<IScheduler::Arithmetic> outputGate(
      IScheduler::WireId<IScheduler::Arithmetic> src,
      int partyID) = 0;

  // Create a batch boolean output gate and return its output wire ID.
  virtual IScheduler::WireId<IScheduler::Boolean> outputGateBatch(
      IScheduler::WireId<IScheduler::Boolean> src,
      int partyID) = 0;

  // Create a batch arithmetic output gate and return its output wire ID.
  virtual IScheduler::WireId<IScheduler::Arithmetic> outputGateBatch(
      IScheduler::WireId<IScheduler::Arithmetic> src,
      int partyID) = 0;

  // Create a unary/binary gate (e.g. AND, XOR, NOT) and return its output wire
  // ID.
  virtual IScheduler::WireId<IScheduler::Boolean> normalGate(
      INormalGate::GateType gateType,
      IScheduler::WireId<IScheduler::Boolean> left,
      IScheduler::WireId<IScheduler::Boolean> right =
          IScheduler::WireId<IScheduler::Boolean>()) = 0;

  // Create a batch unary/binary gate (e.g. AND, XOR, NOT) and return its output
  // wire ID.
  virtual IScheduler::WireId<IScheduler::Boolean> normalGateBatch(
      INormalGate::GateType gateType,
      IScheduler::WireId<IScheduler::Boolean> left,
      IScheduler::WireId<IScheduler::Boolean> right =
          IScheduler::WireId<IScheduler::Boolean>()) = 0;

  // Create an integer gate (e.g. MULT, ADD, NEG) and return its output wire
  // ID.
  virtual IScheduler::WireId<IScheduler::Arithmetic> arithmeticGate(
      IArithmeticGate::GateType gateType,
      IScheduler::WireId<IScheduler::Arithmetic> left,
      IScheduler::WireId<IScheduler::Arithmetic> right =
          IScheduler::WireId<IScheduler::Arithmetic>()) = 0;

  // Create a batch integer gate (e.g. MULT, ADD, NEG) and return its output
  // wire ID.
  virtual IScheduler::WireId<IScheduler::Arithmetic> arithmeticGateBatch(
      IArithmeticGate::GateType gateType,
      IScheduler::WireId<IScheduler::Arithmetic> left,
      IScheduler::WireId<IScheduler::Arithmetic> right =
          IScheduler::WireId<IScheduler::Arithmetic>()) = 0;

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

  // band a number of boolean batches into one batch.
  virtual IScheduler::WireId<IScheduler::Boolean> batchingUp(
      std::vector<IScheduler::WireId<IScheduler::Boolean>> src) = 0;

  // decompose a batch of boolean values into several smaller batches.
  virtual std::vector<IScheduler::WireId<IScheduler::Boolean>> unbatching(
      IScheduler::WireId<IScheduler::Boolean> src,
      std::shared_ptr<std::vector<uint32_t>> unbatchingStrategy) = 0;

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
