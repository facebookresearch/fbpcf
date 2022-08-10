/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <map>

#include "fbpcf/exception/exceptions.h"
#include "fbpcf/scheduler/gate_keeper/IArithmeticGate.h"

namespace fbpcf::scheduler {

class ArithmeticGate final : public IArithmeticGate {
  using IArithmeticGate::gateType_;
  using IArithmeticGate::left_;
  using IArithmeticGate::numberOfResults_;
  using IArithmeticGate::partyID_;
  using IArithmeticGate::right_;
  using IArithmeticGate::scheduledResultIndex_;
  using IArithmeticGate::wireID_;
  using IArithmeticGate::wireKeeper_;
  using typename IArithmeticGate::GateType;

 public:
  ArithmeticGate(
      GateType gateType,
      IScheduler::WireId<IScheduler::WireType::Arithmetic> wireID,
      IScheduler::WireId<IScheduler::WireType::Arithmetic> left,
      IScheduler::WireId<IScheduler::WireType::Arithmetic> right,
      int partyID,
      IWireKeeper& wireKeeper)
      : IArithmeticGate{
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

  ~ArithmeticGate() override {
    decreaseReferenceCount(wireID_);
    decreaseReferenceCount(left_);
    decreaseReferenceCount(right_);
  }

  void compute(
      engine::ISecretShareEngine& engine,
      std::map<int64_t, Secrets>& secretSharesByParty) override {
    switch (gateType_) {
        // Free gates
      case GateType::Neg:
        wireKeeper_.setIntegerValue(
            wireID_,
            engine.computeSymmetricNeg(wireKeeper_.getIntegerValue(left_)));
        break;

      case GateType::AsymmetricPlus:
        wireKeeper_.setIntegerValue(
            wireID_,
            engine.computeAsymmetricPlus(
                wireKeeper_.getIntegerValue(left_),
                wireKeeper_.getIntegerValue(right_)));
        break;

      case GateType::FreeMult:
        wireKeeper_.setIntegerValue(
            wireID_,
            engine.computeFreeMult(
                wireKeeper_.getIntegerValue(left_),
                wireKeeper_.getIntegerValue(right_)));
        break;

      case GateType::Input:
        break;

      case GateType::SymmetricPlus:
        wireKeeper_.setIntegerValue(
            wireID_,
            engine.computeSymmetricPlus(
                wireKeeper_.getIntegerValue(left_),
                wireKeeper_.getIntegerValue(right_)));
        break;

      // Non-free gates
      case GateType::Output: {
        if (secretSharesByParty.find(partyID_) == secretSharesByParty.end()) {
          secretSharesByParty.emplace(
              partyID_,
              IGate::Secrets(std::vector<bool>(), std::vector<uint64_t>()));
        }
        auto& secretShares = secretSharesByParty.at(partyID_).integerSecrets;
        scheduledResultIndex_ = secretShares.size();
        secretShares.push_back(wireKeeper_.getIntegerValue(left_));
        break;
      }

      case GateType::NonFreeMult:
        scheduledResultIndex_ = engine.scheduleMult(
            wireKeeper_.getIntegerValue(left_),
            wireKeeper_.getIntegerValue(right_));
        break;
    }
  }

  void collectScheduledResult(
      engine::ISecretShareEngine& engine,
      std::map<int64_t, Secrets>& revealedSecretsByParty) override {
    switch (gateType_) {
      case GateType::NonFreeMult:
        wireKeeper_.setIntegerValue(
            wireID_, engine.getMultExecutionResult(scheduledResultIndex_));
        break;

      case GateType::Output:
        wireKeeper_.setIntegerValue(
            wireID_,
            revealedSecretsByParty.at(partyID_).integerSecrets.at(
                scheduledResultIndex_));
        break;

      default:
        break;
    }
  }

  void increaseReferenceCount(
      IScheduler::WireId<IScheduler::WireType::Arithmetic> wire) override {
    if (!wire.isEmpty()) {
      wireKeeper_.increaseReferenceCount(wire);
    }
  }

  void decreaseReferenceCount(
      IScheduler::WireId<IScheduler::WireType::Arithmetic> wire) override {
    if (!wire.isEmpty()) {
      wireKeeper_.decreaseReferenceCount(wire);
    }
  }
};

} // namespace fbpcf::scheduler
