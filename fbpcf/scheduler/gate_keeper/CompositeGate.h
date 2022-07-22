/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <exception>
#include <map>
#include <stdexcept>

#include <fbpcf/scheduler/gate_keeper/IGate.h>
#include "fbpcf/scheduler/gate_keeper/ICompositeGate.h"

namespace fbpcf::scheduler {

class CompositeGate final : public ICompositeGate {
 public:
  CompositeGate(
      GateType gateType,
      std::vector<IScheduler::WireId<IScheduler::Boolean>> outputWireIDs,
      IScheduler::WireId<IScheduler::Boolean> left,
      std::vector<IScheduler::WireId<IScheduler::Boolean>> rights,
      IWireKeeper& wireKeeper)
      : ICompositeGate{
            gateType,
            outputWireIDs,
            left,
            rights,
            static_cast<uint32_t>(outputWireIDs.size()),
            wireKeeper} {
    for (auto wireID : outputWireIDs_) {
      increaseReferenceCount(wireID);
    }
    increaseReferenceCount(left_);
    for (auto wireID : rights_) {
      increaseReferenceCount(wireID);
    }
  }

  ~CompositeGate() override {
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
      case GateType::FreeAnd:
        for (size_t i = 0; i < outputWireIDs_.size(); i++) {
          wireKeeper_.setBooleanValue(
              outputWireIDs_[i],
              engine.computeFreeAND(
                  wireKeeper_.getBooleanValue(left_),
                  wireKeeper_.getBooleanValue(rights_[i])));
        }
        break;

      // Non-free gates
      case GateType::NonFreeAnd:
        std::vector<bool> rightWireValues(outputWireIDs_.size());
        for (size_t i = 0; i < outputWireIDs_.size(); i++) {
          rightWireValues[i] = wireKeeper_.getBooleanValue(rights_[i]);
        }
        scheduledResultIndex_ = engine.scheduleCompositeAND(
            wireKeeper_.getBooleanValue(left_), rightWireValues);
        break;
    }
  }

  void collectScheduledResult(
      engine::ISecretShareEngine& engine,
      std::map<int64_t, IGate::Secrets>& /*revealedSecretsByParty*/) override {
    std::vector<bool> result;
    switch (gateType_) {
      case GateType::NonFreeAnd:
        result = engine.getCompositeANDExecutionResult(scheduledResultIndex_);
        for (size_t i = 0; i < result.size(); i++) {
          wireKeeper_.setBooleanValue(outputWireIDs_[i], result[i]);
        }
        break;

      default:
        break;
    }
  }

  void increaseReferenceCount(
      IScheduler::WireId<IScheduler::Boolean> wire) override {
    if (!wire.isEmpty()) {
      wireKeeper_.increaseReferenceCount(wire);
    }
  }

  void decreaseReferenceCount(
      IScheduler::WireId<IScheduler::Boolean> wire) override {
    if (!wire.isEmpty()) {
      wireKeeper_.decreaseReferenceCount(wire);
    }
  }
};

} // namespace fbpcf::scheduler
