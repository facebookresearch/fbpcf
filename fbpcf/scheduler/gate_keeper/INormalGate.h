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
 * This class executes gates in a circuit. There are subclasses for batched and
 * non-batched gates.
 */
class INormalGate : public IGate {
 public:
  enum class GateType {
    FreeAnd,
    Input,
    Output,
    NonFreeAnd,
    AsymmetricNot,
    AsymmetricXOR,
    SymmetricNot,
    SymmetricXOR,
  };

  INormalGate(
      GateType gateType,
      IScheduler::WireId<IScheduler::Boolean> wireID,
      IScheduler::WireId<IScheduler::Boolean> left,
      IScheduler::WireId<IScheduler::Boolean> right,
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

  INormalGate(const INormalGate& g) : wireKeeper_(g.wireKeeper_) {
    copy(g);
  }

  INormalGate(INormalGate&& g) noexcept : wireKeeper_(g.wireKeeper_) {
    move(std::move(g));
  }

  INormalGate& operator=(const INormalGate& g) {
    copy(g);
    return *this;
  }

  INormalGate& operator=(INormalGate&& g) {
    move(std::move(g));
    return *this;
  }

  static bool isFree(GateType gateType) {
    switch (gateType) {
      case GateType::AsymmetricNot:
      case GateType::AsymmetricXOR:
      case GateType::FreeAnd:
      case GateType::Input:
      case GateType::SymmetricNot:
      case GateType::SymmetricXOR:
        return true;

      case GateType::NonFreeAnd:
      case GateType::Output:
        return false;
    }
  }

  IScheduler::WireId<IScheduler::Boolean> getWireId() const {
    return wireID_;
  }

  // The number of values in a batch gate (1 for non-batch case)
  uint32_t getNumberOfResults() const override {
    return numberOfResults_;
  }

 protected:
  GateType gateType_;
  IScheduler::WireId<IScheduler::Boolean> wireID_;
  IScheduler::WireId<IScheduler::Boolean> left_;
  IScheduler::WireId<IScheduler::Boolean> right_;
  int partyID_;
  uint32_t scheduledResultIndex_;
  uint32_t numberOfResults_;
  IWireKeeper& wireKeeper_;

  void copy(const INormalGate& src) {
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

  void move(INormalGate&& src) {
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
        IScheduler::WireId<IScheduler::Boolean>();
  }

  virtual void increaseReferenceCount(
      IScheduler::WireId<IScheduler::Boolean> wire) = 0;

  virtual void decreaseReferenceCount(
      IScheduler::WireId<IScheduler::Boolean> wire) = 0;
};

} // namespace fbpcf::scheduler
