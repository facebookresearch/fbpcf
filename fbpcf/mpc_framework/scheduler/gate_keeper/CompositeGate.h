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

#include "fbpcf/mpc_framework/scheduler/gate_keeper/ICompositeGate.h"

namespace fbpcf::mpc_framework::scheduler {

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
            /*numberOfResults*/ 1,
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
      engine::ISecretShareEngine& /*engine*/,
      std::map<int64_t, std::vector<bool>>& /*secretSharesByParty*/) override {
    switch (gateType_) {
        // Free gates
      case GateType::FreeAnd:
        throw std::runtime_error("Not implemented");
        break;

      // Non-free gates
      case GateType::NonFreeAnd:
        throw std::runtime_error("Not implemented");
        break;
    }
  }

  void collectScheduledResult(
      engine::ISecretShareEngine& /*engine*/,
      std::map<int64_t, std::vector<bool>>& /*revealedSecretsByParty*/)
      override {
    switch (gateType_) {
      case GateType::NonFreeAnd:
        throw std::runtime_error("Not implemented");
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

} // namespace fbpcf::mpc_framework::scheduler
