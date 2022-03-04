/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/mpc_framework/scheduler/gate_keeper/GateKeeper.h"
#include "fbpcf/mpc_framework/scheduler/IScheduler.h"
#include "fbpcf/mpc_framework/scheduler/gate_keeper/BatchCompositeGate.h"
#include "fbpcf/mpc_framework/scheduler/gate_keeper/BatchNormalGate.h"
#include "fbpcf/mpc_framework/scheduler/gate_keeper/CompositeGate.h"
#include "fbpcf/mpc_framework/scheduler/gate_keeper/IGate.h"
#include "fbpcf/mpc_framework/scheduler/gate_keeper/NormalGate.h"

namespace fbpcf::mpc_framework::scheduler {
GateKeeper::GateKeeper(std::shared_ptr<IWireKeeper> wireKeeper)
    : wireKeeper_{wireKeeper} {}

IScheduler::WireId<IScheduler::Boolean> GateKeeper::inputGate(
    BoolType<false> initialValue) {
  return addGate<false, false>(
      INormalGate<IScheduler::Boolean>::GateType::Input,
      IScheduler::WireId<IScheduler::Boolean>(),
      IScheduler::WireId<IScheduler::Boolean>(),
      initialValue);
}

IScheduler::WireId<IScheduler::Boolean> GateKeeper::inputGateBatch(
    BoolType<true> initialValue) {
  return addGate<true, false>(
      INormalGate<IScheduler::Boolean>::GateType::Input,
      IScheduler::WireId<IScheduler::Boolean>(),
      IScheduler::WireId<IScheduler::Boolean>(),
      initialValue);
}

IScheduler::WireId<IScheduler::Boolean> GateKeeper::outputGate(
    IScheduler::WireId<IScheduler::Boolean> src,
    int partyID) {
  return addGate<false, false>(
      INormalGate<IScheduler::Boolean>::GateType::Output,
      src,
      IScheduler::WireId<IScheduler::Boolean>(),
      false,
      partyID);
}

IScheduler::WireId<IScheduler::Boolean> GateKeeper::outputGateBatch(
    IScheduler::WireId<IScheduler::Boolean> src,
    int partyID) {
  return addGate<true, false>(
      INormalGate<IScheduler::Boolean>::GateType::Output,
      src,
      IScheduler::WireId<IScheduler::Boolean>(),
      {},
      partyID);
}

IScheduler::WireId<IScheduler::Boolean> GateKeeper::normalGate(
    INormalGate<IScheduler::Boolean>::GateType gateType,
    IScheduler::WireId<IScheduler::Boolean> left,
    IScheduler::WireId<IScheduler::Boolean> right) {
  return addGate<false, false>(gateType, left, right, false);
}

IScheduler::WireId<IScheduler::Boolean> GateKeeper::normalGateBatch(
    INormalGate<IScheduler::Boolean>::GateType gateType,
    IScheduler::WireId<IScheduler::Boolean> left,
    IScheduler::WireId<IScheduler::Boolean> right) {
  return addGate<true, false>(gateType, left, right, {});
}

std::vector<IScheduler::WireId<IScheduler::Boolean>> GateKeeper::compositeGate(
    ICompositeGate::GateType gateType,
    IScheduler::WireId<IScheduler::Boolean> left,
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) {
  return addGate<false, true>(gateType, left, rights, 0);
}

std::vector<IScheduler::WireId<IScheduler::Boolean>>
GateKeeper::compositeGateBatch(
    ICompositeGate::GateType gateType,
    IScheduler::WireId<IScheduler::Boolean> left,
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) {
  return addGate<true, true>(gateType, left, rights, {});
}

uint32_t GateKeeper::getFirstUnexecutedLevel() const {
  return firstUnexecutedLevel_;
}

std::vector<std::unique_ptr<IGate>> GateKeeper::popFirstUnexecutedLevel() {
  auto gates = std::move(gatesByLevelOffset_.front());
  gatesByLevelOffset_.pop_front();
  ++firstUnexecutedLevel_;
  numUnexecutedGates_ -= gates.size();
  return gates;
}

bool GateKeeper::hasReachedBatchingLimit() const {
  return numUnexecutedGates_ > kMaxUnexecutedGates;
}

template <bool usingBatch, bool isCompositeWire>
GateKeeper::RightWireType<isCompositeWire> GateKeeper::addGate(
    GateType<isCompositeWire> gateType,
    IScheduler::WireId<IScheduler::Boolean> left,
    RightWireType<isCompositeWire> right,
    BoolType<usingBatch> initialValue,
    int partyID) {
  numUnexecutedGates_++;

  auto level = getFirstAvailableLevelForNewWire<usingBatch, isCompositeWire>(
      gateType, left, right);

  while (gatesByLevelOffset_.size() <= level - firstUnexecutedLevel_) {
    gatesByLevelOffset_.emplace_back(std::vector<std::unique_ptr<IGate>>());
  }

  auto& gatesForLevel = gatesByLevelOffset_.at(level - firstUnexecutedLevel_);

  RightWireType<isCompositeWire> outputWire;
  if constexpr (isCompositeWire) {
    if constexpr (usingBatch) {
      for (int i = 0; i < right.size(); i++) {
        outputWire.push_back(wireKeeper_->allocateBatchBooleanValue({}, level));
      }
      gatesForLevel.push_back(std::make_unique<BatchCompositeGate>(
          gateType, outputWire, left, right, 0, *wireKeeper_));
    } else {
      for (int i = 0; i < right.size(); i++) {
        outputWire.push_back(wireKeeper_->allocateBooleanValue(0, level));
      }
      gatesForLevel.push_back(std::make_unique<CompositeGate>(
          gateType, outputWire, left, right, *wireKeeper_));
    }
  } else {
    if constexpr (usingBatch) {
      outputWire = wireKeeper_->allocateBatchBooleanValue(initialValue, level);
      auto numberOfResults = initialValue.size();
      gatesForLevel.push_back(
          std::make_unique<BatchNormalGate<IScheduler::Boolean>>(
              gateType,
              outputWire,
              left,
              right,
              partyID,
              numberOfResults,
              *wireKeeper_));

    } else {
      outputWire = wireKeeper_->allocateBooleanValue(initialValue, level);
      gatesForLevel.push_back(std::make_unique<NormalGate<IScheduler::Boolean>>(
          gateType, outputWire, left, right, partyID, *wireKeeper_));
    }
  }

  return outputWire;
}

// Free gates are added to even levels, and non-free gates are added to odd
// levels. A gate can depend on a free gate at the same level, but cannot
// depend on a non-free gate at the same level.
template <bool usingBatch, bool isCompositeWire>
uint32_t GateKeeper::getFirstAvailableLevelForNewWire(
    GateType<isCompositeWire> gateType,
    IScheduler::WireId<IScheduler::Boolean> left,
    RightWireType<isCompositeWire> right) const {
  uint32_t leftMaxLevel = 0;
  if (left.isEmpty()) {
    leftMaxLevel = 0;
  } else if constexpr (usingBatch) {
    leftMaxLevel = wireKeeper_->getBatchFirstAvailableLevel(left);
  } else {
    leftMaxLevel = wireKeeper_->getFirstAvailableLevel(left);
  }

  uint32_t rightMaxLevel = 0;
  if constexpr (isCompositeWire) {
    for (auto rightWire : right) {
      if (!rightWire.isEmpty()) {
        if constexpr (usingBatch) {
          rightMaxLevel = std::max(
              rightMaxLevel,
              wireKeeper_->getBatchFirstAvailableLevel(rightWire));
        } else {
          rightMaxLevel = std::max(
              rightMaxLevel, wireKeeper_->getFirstAvailableLevel(rightWire));
        }
      }
    }
  } else {
    if (right.isEmpty()) {
      rightMaxLevel = 0;
    } else if constexpr (usingBatch) {
      rightMaxLevel = wireKeeper_->getBatchFirstAvailableLevel(right);
    } else {
      rightMaxLevel = wireKeeper_->getFirstAvailableLevel(right);
    }
  }

  auto isFreeGate = GateClass<isCompositeWire>::isFree(gateType);

  auto minAvailableLeft = leftMaxLevel +
      (IGateKeeper::isLevelFree(leftMaxLevel) ? (isFreeGate ? 0 : 1)
                                              : (isFreeGate ? 1 : 2));
  auto minAvailableRight = rightMaxLevel +
      (IGateKeeper::isLevelFree(rightMaxLevel) ? (isFreeGate ? 0 : 1)
                                               : (isFreeGate ? 1 : 2));
  auto minAvailableFirstUnexecuted = firstUnexecutedLevel_ +
      (IGateKeeper::isLevelFree(firstUnexecutedLevel_) ? (isFreeGate ? 0 : 1)
                                                       : (isFreeGate ? 1 : 0));

  return std::max(
      std::max(minAvailableLeft, minAvailableRight),
      minAvailableFirstUnexecuted);
}
} // namespace fbpcf::mpc_framework::scheduler
