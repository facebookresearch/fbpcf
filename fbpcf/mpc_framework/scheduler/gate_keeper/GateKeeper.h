/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <deque>
#include <memory>

#include "fbpcf/mpc_framework/scheduler/gate_keeper/IGateKeeper.h"

namespace fbpcf::mpc_framework::scheduler {

class GateKeeper : public IGateKeeper {
 public:
  explicit GateKeeper(std::shared_ptr<IWireKeeper> wireKeeper);

  /**
   * @inherit doc
   */
  IScheduler::WireId<IScheduler::Boolean> inputGate(
      BoolType<false> initialValue) override;

  /**
   * @inherit doc
   */
  IScheduler::WireId<IScheduler::Boolean> inputGateBatch(
      BoolType<true> initialValue) override;

  /**
   * @inherit doc
   */
  IScheduler::WireId<IScheduler::Boolean> outputGate(
      IScheduler::WireId<IScheduler::Boolean> src,
      int partyID) override;

  /**
   * @inherit doc
   */
  IScheduler::WireId<IScheduler::Boolean> outputGateBatch(
      IScheduler::WireId<IScheduler::Boolean> src,
      int partyID) override;

  /**
   * @inherit doc
   */
  IScheduler::WireId<IScheduler::Boolean> normalGate(
      INormalGate<IScheduler::Boolean>::GateType gateType,
      IScheduler::WireId<IScheduler::Boolean> left,
      IScheduler::WireId<IScheduler::Boolean> right =
          IScheduler::WireId<IScheduler::Boolean>()) override;

  /**
   * @inherit doc
   */
  IScheduler::WireId<IScheduler::Boolean> normalGateBatch(
      INormalGate<IScheduler::Boolean>::GateType gateType,
      IScheduler::WireId<IScheduler::Boolean> left,
      IScheduler::WireId<IScheduler::Boolean> right =
          IScheduler::WireId<IScheduler::Boolean>()) override;

  /**
   * @inherit doc
   */
  std::vector<IScheduler::WireId<IScheduler::Boolean>> compositeGate(
      ICompositeGate::GateType gateType,
      IScheduler::WireId<IScheduler::Boolean> left,
      std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) override;

  /**
   * @inherit doc
   */
  std::vector<IScheduler::WireId<IScheduler::Boolean>> compositeGateBatch(
      ICompositeGate::GateType gateType,
      IScheduler::WireId<IScheduler::Boolean> left,
      std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) override;

  /**
   * @inherit doc
   */
  uint32_t getFirstUnexecutedLevel() const override;

  /**
   * @inherit doc
   */
  std::vector<std::unique_ptr<IGate>> popFirstUnexecutedLevel() override;

  /**
   * @inherit doc
   */
  bool hasReachedBatchingLimit() const override;

 private:
  template <bool isCompositeWire>
  using GateClass = typename std::conditional<
      isCompositeWire,
      ICompositeGate,
      INormalGate<IScheduler::Boolean>>::type;

  template <bool isCompositeWire>
  using GateType = typename std::conditional<
      isCompositeWire,
      ICompositeGate::GateType,
      INormalGate<IScheduler::Boolean>::GateType>::type;

  template <bool isCompositeWire>
  using RightWireType = typename std::conditional<
      isCompositeWire,
      std::vector<IScheduler::WireId<IScheduler::Boolean>>,
      IScheduler::WireId<IScheduler::Boolean>>::type;

  template <bool usingBatch, bool isCompositeWire>
  RightWireType<isCompositeWire> addGate(
      GateType<isCompositeWire> gateType,
      IScheduler::WireId<IScheduler::Boolean> left,
      RightWireType<isCompositeWire> right,
      BoolType<usingBatch> initialValue,
      int partyID = 0);

  template <bool usingBatch, bool isCompositeWire>
  uint32_t getFirstAvailableLevelForNewWire(
      GateType<isCompositeWire> gateType,
      IScheduler::WireId<IScheduler::Boolean> left,
      RightWireType<isCompositeWire> right) const;

  std::deque<std::vector<std::unique_ptr<IGate>>> gatesByLevelOffset_;
  std::shared_ptr<IWireKeeper> wireKeeper_;

  uint32_t firstUnexecutedLevel_ = 0;

  uint32_t numUnexecutedGates_ = 0;
  const uint32_t kMaxUnexecutedGates = 100000;
};

} // namespace fbpcf::mpc_framework::scheduler
