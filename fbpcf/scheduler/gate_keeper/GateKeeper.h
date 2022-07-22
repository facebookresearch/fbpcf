/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <deque>
#include <memory>
#include <stdexcept>

#include <fbpcf/scheduler/gate_keeper/INormalGate.h>
#include "fbpcf/scheduler/gate_keeper/IGateKeeper.h"

namespace fbpcf::scheduler {

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
      INormalGate::GateType gateType,
      IScheduler::WireId<IScheduler::Boolean> left,
      IScheduler::WireId<IScheduler::Boolean> right =
          IScheduler::WireId<IScheduler::Boolean>()) override;

  /**
   * @inherit doc
   */
  IScheduler::WireId<IScheduler::Boolean> normalGateBatch(
      INormalGate::GateType gateType,
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

  // band a number of batches into one batch.
  IScheduler::WireId<IScheduler::Boolean> batchingUp(
      std::vector<IScheduler::WireId<IScheduler::Boolean>> src) override;

  // decompose a batch of values into several smaller batches.
  std::vector<IScheduler::WireId<IScheduler::Boolean>> unbatching(
      IScheduler::WireId<IScheduler::Boolean> src,
      std::shared_ptr<std::vector<uint32_t>> unbatchingStrategy) override;

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
      INormalGate>::type;

  template <bool isCompositeWire>
  using GateType = typename std::conditional<
      isCompositeWire,
      ICompositeGate::GateType,
      INormalGate::GateType>::type;

  template <bool isCompositeWire>
  using RightWireType = typename std::conditional<
      isCompositeWire,
      std::vector<IScheduler::WireId<IScheduler::Boolean>>,
      IScheduler::WireId<IScheduler::Boolean>>::type;

  std::deque<std::vector<std::unique_ptr<IGate>>> gatesByLevelOffset_;
  std::shared_ptr<IWireKeeper> wireKeeper_;

  uint32_t firstUnexecutedLevel_ = 0;

  uint32_t numUnexecutedGates_ = 0;
  const uint32_t kMaxUnexecutedGates = 100000;

  // below are helper functions. They are inlined for the sake of performance.
  inline std::vector<std::unique_ptr<IGate>>& getLevel(uint32_t level) {
    while (gatesByLevelOffset_.size() <= level - firstUnexecutedLevel_) {
      gatesByLevelOffset_.emplace_back(std::vector<std::unique_ptr<IGate>>());
    }

    return gatesByLevelOffset_.at(level - firstUnexecutedLevel_);
  }

  inline void addGate(std::unique_ptr<IGate> gate, uint32_t level) {
    auto& levelOfGate = getLevel(level);
    levelOfGate.push_back(std::move(gate));
    numUnexecutedGates_++;
  }

  inline IScheduler::WireId<IScheduler::Boolean> allocateNewWire(
      bool v,
      uint32_t level) const {
    return wireKeeper_->allocateBooleanValue(v, level);
  }

  inline IScheduler::WireId<IScheduler::Boolean> allocateNewWire(
      const std::vector<bool>& v,
      uint32_t level) const {
    return wireKeeper_->allocateBatchBooleanValue(v, level);
  }

  inline uint32_t getOutputLevel(bool isGateFree, uint32_t maxInputLevel)
      const {
    uint32_t outputLevel =
        std::max(maxInputLevel, firstUnexecutedLevel_) + (isGateFree ? 0 : 1);

    return outputLevel +
        ((IGateKeeper::isLevelFree(outputLevel) != isGateFree) ? 1 : 0);
  }

  template <bool usingBatch>
  inline uint32_t getMaxLevel(
      IScheduler::WireId<IScheduler::Boolean> id) const {
    if (id.isEmpty()) {
      return 0;
    } else if constexpr (usingBatch) {
      return wireKeeper_->getBatchFirstAvailableLevel(id);
    } else {
      return wireKeeper_->getFirstAvailableLevel(id);
    }
  }

  template <bool usingBatch>
  inline uint32_t getMaxLevel(
      const std::vector<IScheduler::WireId<IScheduler::Boolean>>& id) const {
    if (id.size() == 0) {
      throw std::runtime_error("Empty wire id vector!");
    }
    auto rst = getMaxLevel<usingBatch>(id.at(0));
    for (size_t i = 1; i < id.size(); i++) {
      rst = std::max(rst, getMaxLevel<usingBatch>(id.at(i)));
    }
    return rst;
  }
};

} // namespace fbpcf::scheduler
