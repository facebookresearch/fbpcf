/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <map>

#include <fbpcf/scheduler/IScheduler.h>
#include "fbpcf/scheduler/gate_keeper/INormalGate.h"

namespace fbpcf::scheduler {

class NormalGate final : public INormalGate {
  using INormalGate::gateType_;
  using INormalGate::left_;
  using INormalGate::numberOfResults_;
  using INormalGate::partyID_;
  using INormalGate::right_;
  using INormalGate::scheduledResultIndex_;
  using INormalGate::wireID_;
  using INormalGate::wireKeeper_;
  using typename INormalGate::GateType;

 public:
  NormalGate(
      GateType gateType,
      IScheduler::WireId<IScheduler::Boolean> wireID,
      IScheduler::WireId<IScheduler::Boolean> left,
      IScheduler::WireId<IScheduler::Boolean> right,
      int partyID,
      IWireKeeper& wireKeeper)
      : INormalGate{
            gateType,
            wireID,
            left,
            right,
            partyID,
            /*numberOfResults*/ 1,
            wireKeeper} {
    increaseReferenceCount(wireID_);
    increaseReferenceCount(left_);
    increaseReferenceCount(right_);
  }

  ~NormalGate() override {
    decreaseReferenceCount(wireID_);
    decreaseReferenceCount(left_);
    decreaseReferenceCount(right_);
  }

  void compute(
      engine::ISecretShareEngine& engine,
      std::map<int64_t, IGate::Secrets>& secretSharesByParty) override {
    switch (gateType_) {
        // Free gates
      case GateType::AsymmetricNot:
        wireKeeper_.setBooleanValue(
            wireID_,
            engine.computeAsymmetricNOT(wireKeeper_.getBooleanValue(left_)));
        break;

      case GateType::AsymmetricXOR:
        wireKeeper_.setBooleanValue(
            wireID_,
            engine.computeAsymmetricXOR(
                wireKeeper_.getBooleanValue(left_),
                wireKeeper_.getBooleanValue(right_)));
        break;

      case GateType::FreeAnd:
        wireKeeper_.setBooleanValue(
            wireID_,
            engine.computeFreeAND(
                wireKeeper_.getBooleanValue(left_),
                wireKeeper_.getBooleanValue(right_)));
        break;

      case GateType::Input:
        break;

      case GateType::SymmetricNot:
        wireKeeper_.setBooleanValue(
            wireID_,
            engine.computeSymmetricNOT(wireKeeper_.getBooleanValue(left_)));
        break;

      case GateType::SymmetricXOR:
        wireKeeper_.setBooleanValue(
            wireID_,
            engine.computeSymmetricXOR(
                wireKeeper_.getBooleanValue(left_),
                wireKeeper_.getBooleanValue(right_)));
        break;

      // Non-free gates
      case GateType::Output: {
        if (secretSharesByParty.find(partyID_) == secretSharesByParty.end()) {
          secretSharesByParty.emplace(
              partyID_,
              IGate::Secrets(std::vector<bool>(), std::vector<uint64_t>()));
        }
        auto& secretShares = secretSharesByParty.at(partyID_).booleanSecrets;
        scheduledResultIndex_ = secretShares.size();
        secretShares.push_back(wireKeeper_.getBooleanValue(left_));
        break;
      }

      case GateType::NonFreeAnd:
        scheduledResultIndex_ = engine.scheduleAND(
            wireKeeper_.getBooleanValue(left_),
            wireKeeper_.getBooleanValue(right_));
        break;
    }
  }

  void collectScheduledResult(
      engine::ISecretShareEngine& engine,
      std::map<int64_t, IGate::Secrets>& revealedSecretsByParty) override {
    switch (gateType_) {
      case GateType::NonFreeAnd:
        wireKeeper_.setBooleanValue(
            wireID_, engine.getANDExecutionResult(scheduledResultIndex_));
        break;

      case GateType::Output:
        wireKeeper_.setBooleanValue(
            wireID_,
            revealedSecretsByParty.at(partyID_).booleanSecrets.at(
                scheduledResultIndex_));
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
