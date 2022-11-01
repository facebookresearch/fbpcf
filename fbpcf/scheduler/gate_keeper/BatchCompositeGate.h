/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <exception>
#include <map>
#include <stdexcept>

#include "fbpcf/scheduler/gate_keeper/ICompositeGate.h"

namespace fbpcf::scheduler {

class BatchCompositeGate final : public ICompositeGate {
 public:
  BatchCompositeGate(
      GateType gateType,
      std::vector<IScheduler::WireId<IScheduler::Boolean>> outputWireIDs,
      IScheduler::WireId<IScheduler::Boolean> left,
      std::vector<IScheduler::WireId<IScheduler::Boolean>> rights,
      IWireKeeper& wireKeeper)
      : ICompositeGate{gateType, outputWireIDs, left, rights, 0, wireKeeper} {
    for (auto wireID : outputWireIDs_) {
      increaseReferenceCount(wireID);
    }
    increaseReferenceCount(left_);
    for (auto wireID : rights_) {
      increaseReferenceCount(wireID);
    }
  }

  ~BatchCompositeGate() override {
    for (auto wireID : outputWireIDs_) {
      decreaseReferenceCount(wireID);
    }
    decreaseReferenceCount(left_);
    for (auto wireID : rights_) {
      decreaseReferenceCount(wireID);
    }
  }

  void compute(
      engine::ISecretShareEngine& engine,
      std::map<int64_t, IGate::Secrets>& /*secretSharesByParty*/) override {
    switch (gateType_) {
        // Free gates
      case GateType::FreeAnd: {
        auto& leftValues = wireKeeper_.getBatchBooleanValue(left_);
        numberOfResults_ = leftValues.size() * outputWireIDs_.size();

        for (size_t i = 0; i < outputWireIDs_.size(); i++) {
          wireKeeper_.setBatchBooleanValue(
              outputWireIDs_[i],
              engine.computeBatchFreeAND(
                  leftValues, wireKeeper_.getBatchBooleanValue(rights_[i])));
        }
      }

      case GateType::NonFreeAnd: {
        auto& leftValues = wireKeeper_.getBatchBooleanValue(left_);
        std::vector<std::vector<bool>> rightWireValues(outputWireIDs_.size());
        for (size_t i = 0; i < outputWireIDs_.size(); i++) {
          rightWireValues[i] = wireKeeper_.getBatchBooleanValue(rights_[i]);
        }
        numberOfResults_ = leftValues.size() * outputWireIDs_.size();
        scheduledResultIndex_ =
            engine.scheduleBatchCompositeAND(leftValues, rightWireValues);
        break;
      }
    }
  }

  void collectScheduledResult(
      engine::ISecretShareEngine& engine,
      std::map<int64_t, IGate::Secrets>& /*revealedSecretsByParty*/) override {
    std::vector<std::vector<bool>> result;
    switch (gateType_) {
      case GateType::NonFreeAnd: {
        result =
            engine.getBatchCompositeANDExecutionResult(scheduledResultIndex_);
        for (size_t i = 0; i < result.size(); i++) {
          wireKeeper_.setBatchBooleanValue(outputWireIDs_[i], result[i]);
        }
        break;
      }

      default:
        break;
    }
  }

  void increaseReferenceCount(
      IScheduler::WireId<IScheduler::Boolean> wire) override {
    if (!wire.isEmpty()) {
      wireKeeper_.increaseBatchReferenceCount(wire);
    }
  }

  void decreaseReferenceCount(
      IScheduler::WireId<IScheduler::Boolean> wire) override {
    if (!wire.isEmpty()) {
      wireKeeper_.decreaseBatchReferenceCount(wire);
    }
  }
};

} // namespace fbpcf::scheduler
