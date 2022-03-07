/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <map>

#include "fbpcf/mpc_framework/scheduler/gate_keeper/INormalGate.h"

namespace fbpcf::mpc_framework::scheduler {

template <IScheduler::WireType T>
class NormalGate final : public INormalGate<T> {
  using typename INormalGate<T>::GateType;
  using INormalGate<T>::gateType_;
  using INormalGate<T>::wireID_;
  using INormalGate<T>::left_;
  using INormalGate<T>::right_;
  using INormalGate<T>::partyID_;
  using INormalGate<T>::scheduledResultIndex_;
  using INormalGate<T>::numberOfResults_;
  using INormalGate<T>::wireKeeper_;

 public:
  NormalGate(
      GateType gateType,
      IScheduler::WireId<T> wireID,
      IScheduler::WireId<T> left,
      IScheduler::WireId<T> right,
      int partyID,
      IWireKeeper& wireKeeper)
      : INormalGate<T>{
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
      std::map<int64_t, std::vector<bool>>& secretSharesByParty) override {
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
          secretSharesByParty.emplace(partyID_, std::vector<bool>());
        }
        auto& secretShares = secretSharesByParty.at(partyID_);
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
      std::map<int64_t, std::vector<bool>>& revealedSecretsByParty) override {
    switch (gateType_) {
      case GateType::NonFreeAnd:
        wireKeeper_.setBooleanValue(
            wireID_, engine.getANDExecutionResult(scheduledResultIndex_));
        break;

      case GateType::Output:
        wireKeeper_.setBooleanValue(
            wireID_,
            revealedSecretsByParty.at(partyID_).at(scheduledResultIndex_));
        break;

      default:
        break;
    }
  }

  void increaseReferenceCount(IScheduler::WireId<T> wire) override {
    if (!wire.isEmpty()) {
      wireKeeper_.increaseReferenceCount(wire);
    }
  }

  void decreaseReferenceCount(IScheduler::WireId<T> wire) override {
    if (!wire.isEmpty()) {
      wireKeeper_.decreaseReferenceCount(wire);
    }
  }
};

} // namespace fbpcf::mpc_framework::scheduler
