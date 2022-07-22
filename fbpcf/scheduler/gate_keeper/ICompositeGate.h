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

#include "fbpcf/engine/ISecretShareEngine.h"
#include "fbpcf/scheduler/IScheduler.h"
#include "fbpcf/scheduler/IWireKeeper.h"
#include "fbpcf/scheduler/gate_keeper/IGate.h"

namespace fbpcf::scheduler {

/**
 * This class executes composite gates in a circuit. i.e. where one value is
 * used in the same operation with multiple other values. There are subclasses
 * for batched and non-batched gates.
 */
class ICompositeGate : public IGate {
 public:
  enum class GateType {
    FreeAnd,
    NonFreeAnd,
  };

  ICompositeGate(
      GateType gateType,
      std::vector<IScheduler::WireId<IScheduler::Boolean>> outputWireIDs,
      IScheduler::WireId<IScheduler::Boolean> left,
      std::vector<IScheduler::WireId<IScheduler::Boolean>> rights,
      uint32_t numberOfResults,
      IWireKeeper& wireKeeper)
      : gateType_{gateType},
        outputWireIDs_{outputWireIDs},
        left_{left},
        rights_{rights},
        numberOfResults_{numberOfResults},
        wireKeeper_{wireKeeper} {
    if (outputWireIDs.size() != rights.size()) {
      throw std::runtime_error(
          "Number of input wires on rhs must equal number of output wires.");
    }
  }

  ICompositeGate(const ICompositeGate& g) : wireKeeper_(g.wireKeeper_) {
    copy(g);
  }

  ICompositeGate(ICompositeGate&& g) noexcept : wireKeeper_(g.wireKeeper_) {
    move(std::move(g));
  }

  ICompositeGate& operator=(const ICompositeGate& g) {
    copy(g);
    return *this;
  }

  ICompositeGate& operator=(ICompositeGate&& g) {
    move(std::move(g));
    return *this;
  }

  virtual ~ICompositeGate() override = default;

  static bool isFree(GateType gateType) {
    switch (gateType) {
      case GateType::FreeAnd:
        return true;

      case GateType::NonFreeAnd:
        return false;
    }
  }

  // Run or schedule the computation for this gate.
  virtual void compute(
      engine::ISecretShareEngine& engine,
      std::map<int64_t, IGate::Secrets>& secretSharesByParty) override = 0;

  // For non-free gates, get the result of the computation that was scheduled
  // and store it on the appropriate wire.
  virtual void collectScheduledResult(
      engine::ISecretShareEngine& engine,
      std::map<int64_t, IGate::Secrets>& revealedSecretsByParty) override = 0;

  uint32_t getNumberOfResults() const override {
    return numberOfResults_;
  }

  std::vector<IScheduler::WireId<IScheduler::Boolean>> getOutputWireIds()
      const {
    return outputWireIDs_;
  }

 protected:
  GateType gateType_;
  std::vector<IScheduler::WireId<IScheduler::Boolean>> outputWireIDs_;
  IScheduler::WireId<IScheduler::Boolean> left_;
  std::vector<IScheduler::WireId<IScheduler::Boolean>> rights_;
  uint32_t scheduledResultIndex_;
  uint32_t numberOfResults_;
  IWireKeeper& wireKeeper_;

  void copy(const ICompositeGate& src) {
    for (auto wireID : outputWireIDs_) {
      decreaseReferenceCount(wireID);
    }
    decreaseReferenceCount(left_);
    for (auto wireID : rights_) {
      decreaseReferenceCount(wireID);
    }

    gateType_ = src.gateType_;
    outputWireIDs_ = src.outputWireIDs_;
    left_ = src.left_;
    rights_ = src.rights_;
    scheduledResultIndex_ = src.scheduledResultIndex_;
    numberOfResults_ = src.numberOfResults_;

    for (auto wireID : outputWireIDs_) {
      increaseReferenceCount(wireID);
    }
    increaseReferenceCount(left_);
    for (auto wireID : rights_) {
      increaseReferenceCount(wireID);
    }
  }

  void move(ICompositeGate&& src) {
    for (auto wireID : outputWireIDs_) {
      decreaseReferenceCount(wireID);
    }
    decreaseReferenceCount(left_);
    for (auto wireID : rights_) {
      decreaseReferenceCount(wireID);
    }
    gateType_ = src.gateType_;
    outputWireIDs_ = src.outputWireIDs_;
    left_ = src.left_;
    rights_ = src.rights_;
    scheduledResultIndex_ = src.scheduledResultIndex_;
    numberOfResults_ = src.numberOfResults_;

    src.outputWireIDs_ = src.rights_ =
        std::vector<IScheduler::WireId<IScheduler::Boolean>>();
    src.left_ = IScheduler::WireId<IScheduler::Boolean>();
  }

  virtual void increaseReferenceCount(
      IScheduler::WireId<IScheduler::Boolean> wire) = 0;

  virtual void decreaseReferenceCount(
      IScheduler::WireId<IScheduler::Boolean> wire) = 0;
};

} // namespace fbpcf::scheduler
