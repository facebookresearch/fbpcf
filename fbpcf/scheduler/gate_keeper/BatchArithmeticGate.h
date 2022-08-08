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

class BatchArithmeticGate final : public IArithmeticGate {
 public:
  using IArithmeticGate::gateType_;
  using IArithmeticGate::left_;
  using IArithmeticGate::numberOfResults_;
  using IArithmeticGate::partyID_;
  using IArithmeticGate::right_;
  using IArithmeticGate::scheduledResultIndex_;
  using IArithmeticGate::wireID_;
  using IArithmeticGate::wireKeeper_;
  using typename IArithmeticGate::GateType;
  BatchArithmeticGate(
      GateType gateType,
      IScheduler::WireId<IScheduler::WireType::Arithmetic> wireID,
      IScheduler::WireId<IScheduler::WireType::Arithmetic> left,
      IScheduler::WireId<IScheduler::WireType::Arithmetic> right,
      int partyID,
      uint32_t numberOfResults,
      IWireKeeper& wireKeeper)
      : IArithmeticGate{
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

  ~BatchArithmeticGate() override {
    decreaseReferenceCount(wireID_);
    decreaseReferenceCount(left_);
    decreaseReferenceCount(right_);
  }

  void compute(
      engine::ISecretShareEngine& engine,
      std::map<int64_t, IGate::Secrets>& secretSharesByParty) override {
    switch (gateType_) {
        // Free gates
      case GateType::Neg: {
        auto values = wireKeeper_.getBatchIntegerValue(left_);
        numberOfResults_ = values.size();
        wireKeeper_.setBatchIntegerValue(
            wireID_, engine.computeBatchSymmetricNeg(values));
        break;
      }

      case GateType::AsymmetricPlus: {
        auto leftValues = wireKeeper_.getBatchIntegerValue(left_);
        auto rightValues = wireKeeper_.getBatchIntegerValue(right_);
        numberOfResults_ = leftValues.size();
        wireKeeper_.setBatchIntegerValue(
            wireID_,
            engine.computeBatchAsymmetricPlus(leftValues, rightValues));
        break;
      }

      case GateType::FreeMult: {
        auto leftValues = wireKeeper_.getBatchIntegerValue(left_);
        auto rightValues = wireKeeper_.getBatchIntegerValue(right_);
        numberOfResults_ = leftValues.size();
        wireKeeper_.setBatchIntegerValue(
            wireID_, engine.computeBatchFreeMult(leftValues, rightValues));
        break;
      }

      case GateType::Input:
        break;

      case GateType::SymmetricPlus: {
        auto leftValues = wireKeeper_.getBatchIntegerValue(left_);
        auto rightValues = wireKeeper_.getBatchIntegerValue(right_);
        numberOfResults_ = leftValues.size();
        wireKeeper_.setBatchIntegerValue(
            wireID_, engine.computeBatchSymmetricPlus(leftValues, rightValues));
        break;
      }

      // Non-free gates
      case GateType::Output: {
        if (secretSharesByParty.find(partyID_) == secretSharesByParty.end()) {
          secretSharesByParty.emplace(
              partyID_,
              IGate::Secrets(std::vector<bool>(), std::vector<uint64_t>()));
        }
        auto& secretShares = secretSharesByParty.at(partyID_).integerSecrets;
        scheduledResultIndex_ = secretShares.size();

        auto values = wireKeeper_.getBatchIntegerValue(left_);
        numberOfResults_ = values.size();
        secretShares.insert(secretShares.end(), values.begin(), values.end());
        break;
      }

      case GateType::NonFreeMult: {
        auto leftValues = wireKeeper_.getBatchIntegerValue(left_);
        auto rightValues = wireKeeper_.getBatchIntegerValue(right_);

        numberOfResults_ = leftValues.size();
        if (numberOfResults_ == 0) {
          break;
        }
        scheduledResultIndex_ =
            engine.scheduleBatchMult(leftValues, rightValues);
        break;
      }
    }
  }

  void collectScheduledResult(
      engine::ISecretShareEngine& engine,
      std::map<int64_t, IGate::Secrets>& revealedSecretsByParty) override {
    switch (gateType_) {
      case GateType::NonFreeMult: {
        wireKeeper_.setBatchIntegerValue(
            wireID_, engine.getBatchMultExecutionResult(scheduledResultIndex_));
        break;
      }

      case GateType::Output: {
        auto iterator =
            revealedSecretsByParty.at(partyID_).integerSecrets.begin() +
            scheduledResultIndex_;
        std::vector<uint64_t> results(iterator, iterator + numberOfResults_);
        wireKeeper_.setBatchIntegerValue(wireID_, results);
        break;
      }

      default:
        break;
    }
  }

  void increaseReferenceCount(
      IScheduler::WireId<IScheduler::WireType::Arithmetic> wire) override {
    if (!wire.isEmpty()) {
      wireKeeper_.increaseBatchReferenceCount(wire);
    }
  }

  void decreaseReferenceCount(
      IScheduler::WireId<IScheduler::WireType::Arithmetic> wire) override {
    if (!wire.isEmpty()) {
      wireKeeper_.decreaseBatchReferenceCount(wire);
    }
  }
};

} // namespace fbpcf::scheduler
