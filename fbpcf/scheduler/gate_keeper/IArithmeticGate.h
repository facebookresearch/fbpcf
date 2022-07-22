/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <map>

#include "fbpcf/engine/ISecretShareEngine.h"
#include "fbpcf/scheduler/IScheduler.h"
#include "fbpcf/scheduler/IWireKeeper.h"
#include "fbpcf/scheduler/gate_keeper/IGate.h"

namespace fbpcf::scheduler {

/**
 * This class executes arithmetic gates in a circuit.
 * There are subclasses for batched and non-batched gates.
 */
class IArithmeticGate : public IGate {
 public:
  enum class GateType {
    Input,
    Output,
    FreeMult,
    NonFreeMult,
    AsymmetricPlus,
    SymmetricPlus,
    Neg,
  };

  IArithmeticGate(
      GateType gateType,
      IScheduler::WireId<IScheduler::WireType::Arithmetic> wireID,
      IScheduler::WireId<IScheduler::WireType::Arithmetic> left,
      IScheduler::WireId<IScheduler::WireType::Arithmetic> right,
      int partyID,
      uint32_t numberOfResults,
      IWireKeeper& wireKeeper)
      : gateType_{gateType},
        wireID_{wireID},
        left_{left},
        right_{right},
        partyID_{partyID},
        numberOfResults_{numberOfResults},
        wireKeeper_{wireKeeper} {}

  IArithmeticGate(const IArithmeticGate& g) : wireKeeper_(g.wireKeeper_) {
    copy(g);
  }

  IArithmeticGate(IArithmeticGate&& g) noexcept : wireKeeper_(g.wireKeeper_) {
    move(std::move(g));
  }

  IArithmeticGate& operator=(const IArithmeticGate& g) {
    copy(g);
    return *this;
  }

  IArithmeticGate& operator=(IArithmeticGate&& g) {
    move(std::move(g));
    return *this;
  }

  static bool isFree(GateType gateType) {
    switch (gateType) {
      case GateType::AsymmetricPlus:
      case GateType::FreeMult:
      case GateType::Input:
      case GateType::Neg:
      case GateType::SymmetricPlus:
        return true;

      case GateType::NonFreeMult:
      case GateType::Output:
        return false;
    }
  }

  IScheduler::WireId<IScheduler::WireType::Arithmetic> getWireId() const {
    return wireID_;
  }

  // The number of values in a batch gate (1 for non-batch case)
  uint32_t getNumberOfResults() const override {
    return numberOfResults_;
  }

 protected:
  GateType gateType_;
  IScheduler::WireId<IScheduler::WireType::Arithmetic> wireID_;
  IScheduler::WireId<IScheduler::WireType::Arithmetic> left_;
  IScheduler::WireId<IScheduler::WireType::Arithmetic> right_;
  int partyID_;
  uint32_t scheduledResultIndex_;
  uint32_t numberOfResults_;
  IWireKeeper& wireKeeper_;

  void copy(const IArithmeticGate& src) {
    decreaseReferenceCount(wireID_);
    decreaseReferenceCount(left_);
    decreaseReferenceCount(right_);
    gateType_ = src.gateType_;
    wireID_ = src.wireID_;
    left_ = src.left_;
    right_ = src.right_;
    partyID_ = src.partyID_;
    scheduledResultIndex_ = src.scheduledResultIndex_;
    numberOfResults_ = src.numberOfResults_;
    increaseReferenceCount(wireID_);
    increaseReferenceCount(left_);
    increaseReferenceCount(right_);
  }

  void move(IArithmeticGate&& src) {
    decreaseReferenceCount(wireID_);
    decreaseReferenceCount(left_);
    decreaseReferenceCount(right_);
    gateType_ = src.gateType_;
    wireID_ = src.wireID_;
    left_ = src.left_;
    right_ = src.right_;
    partyID_ = src.partyID_;
    scheduledResultIndex_ = src.scheduledResultIndex_;
    numberOfResults_ = src.numberOfResults_;

    src.wireID_ = src.left_ = src.right_ =
        IScheduler::WireId<IScheduler::WireType::Arithmetic>();
  }

  virtual void increaseReferenceCount(
      IScheduler::WireId<IScheduler::WireType::Arithmetic> wire) = 0;

  virtual void decreaseReferenceCount(
      IScheduler::WireId<IScheduler::WireType::Arithmetic> wire) = 0;
};

} // namespace fbpcf::scheduler
