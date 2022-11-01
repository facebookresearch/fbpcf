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
#include <fbpcf/scheduler/gate_keeper/INormalGate.h>

namespace fbpcf::scheduler {

class BatchNormalGate final : public INormalGate {
 public:
  using INormalGate::gateType_;
  using INormalGate::left_;
  using INormalGate::numberOfResults_;
  using INormalGate::partyID_;
  using INormalGate::right_;
  using INormalGate::scheduledResultIndex_;
  using INormalGate::wireID_;
  using INormalGate::wireKeeper_;
  using typename INormalGate::GateType;
  BatchNormalGate(
      GateType gateType,
      IScheduler::WireId<IScheduler::Boolean> wireID,
      IScheduler::WireId<IScheduler::Boolean> left,
      IScheduler::WireId<IScheduler::Boolean> right,
      int partyID,
      uint32_t numberOfResults,
      IWireKeeper& wireKeeper)
      : INormalGate{
            gateType,
            wireID,
            left,
            right,
            partyID,
            numberOfResults,
            wireKeeper} {
    increaseReferenceCount(wireID_);
    increaseReferenceCount(left_);
    increaseReferenceCount(right_);
  }

  ~BatchNormalGate() override {
    decreaseReferenceCount(wireID_);
    decreaseReferenceCount(left_);
    decreaseReferenceCount(right_);
  }

  void compute(
      engine::ISecretShareEngine& engine,
      std::map<int64_t, IGate::Secrets>& secretSharesByParty) override {
    switch (gateType_) {
        // Free gates
      case GateType::AsymmetricNot: {
        auto& values = wireKeeper_.getBatchBooleanValue(left_);
        numberOfResults_ = values.size();
        wireKeeper_.setBatchBooleanValue(
            wireID_, engine.computeBatchAsymmetricNOT(values));
        break;
      }

      case GateType::AsymmetricXOR: {
        auto& leftValues = wireKeeper_.getBatchBooleanValue(left_);
        auto& rightValues = wireKeeper_.getBatchBooleanValue(right_);
        numberOfResults_ = leftValues.size();
        wireKeeper_.setBatchBooleanValue(
            wireID_, engine.computeBatchAsymmetricXOR(leftValues, rightValues));
        break;
      }

      case GateType::FreeAnd: {
        auto& leftValues = wireKeeper_.getBatchBooleanValue(left_);
        auto& rightValues = wireKeeper_.getBatchBooleanValue(right_);
        numberOfResults_ = leftValues.size();
        wireKeeper_.setBatchBooleanValue(
            wireID_, engine.computeBatchFreeAND(leftValues, rightValues));
        break;
      }

      case GateType::Input:
        break;

      case GateType::SymmetricNot: {
        auto& values = wireKeeper_.getBatchBooleanValue(left_);
        numberOfResults_ = values.size();
        wireKeeper_.setBatchBooleanValue(
            wireID_, engine.computeBatchSymmetricNOT(values));
        break;
      }

      case GateType::SymmetricXOR: {
        auto& leftValues = wireKeeper_.getBatchBooleanValue(left_);
        auto& rightValues = wireKeeper_.getBatchBooleanValue(right_);
        numberOfResults_ = leftValues.size();
        wireKeeper_.setBatchBooleanValue(
            wireID_, engine.computeBatchSymmetricXOR(leftValues, rightValues));
        break;
      }

      // Non-free gates
      case GateType::Output: {
        if (secretSharesByParty.find(partyID_) == secretSharesByParty.end()) {
          secretSharesByParty.emplace(
              partyID_,
              IGate::Secrets(std::vector<bool>(), std::vector<uint64_t>()));
        }
        auto& secretShares = secretSharesByParty.at(partyID_).booleanSecrets;
        scheduledResultIndex_ = secretShares.size();

        auto& values = wireKeeper_.getBatchBooleanValue(left_);
        numberOfResults_ = values.size();
        secretShares.insert(secretShares.end(), values.begin(), values.end());
        break;
      }

      case GateType::NonFreeAnd: {
        auto& leftValues = wireKeeper_.getBatchBooleanValue(left_);
        auto& rightValues = wireKeeper_.getBatchBooleanValue(right_);

        numberOfResults_ = leftValues.size();
        if (numberOfResults_ == 0) {
          break;
        }
        scheduledResultIndex_ =
            engine.scheduleBatchAND(leftValues, rightValues);
        break;
      }
    }
  }

  void collectScheduledResult(
      engine::ISecretShareEngine& engine,
      std::map<int64_t, IGate::Secrets>& revealedSecretsByParty) override {
    switch (gateType_) {
      case GateType::NonFreeAnd: {
        wireKeeper_.setBatchBooleanValue(
            wireID_, engine.getBatchANDExecutionResult(scheduledResultIndex_));
        break;
      }

      case GateType::Output: {
        auto iterator =
            revealedSecretsByParty.at(partyID_).booleanSecrets.begin() +
            scheduledResultIndex_;
        std::vector<bool> results(iterator, iterator + numberOfResults_);
        wireKeeper_.setBatchBooleanValue(wireID_, results);
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
