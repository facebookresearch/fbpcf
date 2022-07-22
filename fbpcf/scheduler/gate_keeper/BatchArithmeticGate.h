/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <map>

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
      case GateType::AsymmetricPlus:
        throw std::runtime_error("Unimplemented");
        break;

      case GateType::FreeMult:
        throw std::runtime_error("Unimplemented");
        break;

      case GateType::Input:
        throw std::runtime_error("Unimplemented");
        break;

      case GateType::Neg:
        throw std::runtime_error("Unimplemented");
        break;

      case GateType::SymmetricPlus:
        throw std::runtime_error("Unimplemented");
        break;

      // Non-free gates
      case GateType::Output:
        throw std::runtime_error("Unimplemented");
        break;

      case GateType::NonFreeMult:
        throw std::runtime_error("Unimplemented");
        break;
    }
  }

  void collectScheduledResult(
      engine::ISecretShareEngine& engine,
      std::map<int64_t, IGate::Secrets>& revealedSecretsByParty) override {
    switch (gateType_) {
      case GateType::NonFreeMult:
        throw std::runtime_error("Unimplemented");
        break;

      case GateType::Output:
        throw std::runtime_error("Unimplemented");
        break;

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
