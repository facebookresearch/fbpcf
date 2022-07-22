/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>
#include "fbpcf/scheduler/gate_keeper/INormalGate.h"

namespace fbpcf::scheduler {
/**
 * Rebatching gate doesn't really do any computation. Instead, its only goal is
 *to rebatch some batched values to ease future computation. Note that because
 *all other computation happens in an async manner, this kind of rebatching will
 *also need to be async as well.
 **/

class RebatchingBooleanGate : public IGate {
 public:
  enum class GateType {
    Batching, // Batch a number of batches of values into one batch of
              // values.
    Unbatching, // decompose a batch into several batches.
  };

  // this contructor will create a "Batching" gate. The caller is responsible
  // to make sure the batching is doable - e.g. you can't batch 3 vectors of 2
  // elements to a vector of 3 elements.
  RebatchingBooleanGate(
      const std::vector<IScheduler::WireId<IScheduler::Boolean>>&
          individualWireIDs,
      const IScheduler::WireId<IScheduler::Boolean>& batchWireID,
      IWireKeeper& wireKeeper)
      : gateType_(GateType::Batching),
        individualWireIDs_(individualWireIDs),
        batchWireID_(batchWireID),
        wireKeeper_(wireKeeper) {
    increaseBatchReferenceCount(batchWireID_);
    for (auto& wire : individualWireIDs_) {
      increaseBatchReferenceCount(wire);
    }
  }

  // this contructor will create a "unbatching" gate. The caller is responsible
  // to make sure the unbatching is doable - e.g. you can't unbatch a vector of
  // 3 elements to 3 vectors of 2 elements.
  RebatchingBooleanGate(
      const IScheduler::WireId<IScheduler::Boolean>& batchWireID,
      const std::vector<IScheduler::WireId<IScheduler::Boolean>>&
          individualWireIDs,
      IWireKeeper& wireKeeper,
      std::shared_ptr<std::vector<uint32_t>> unbatchingStrategy)
      : gateType_(GateType::Unbatching),
        individualWireIDs_(individualWireIDs),
        batchWireID_(batchWireID),
        wireKeeper_(wireKeeper),
        unbatchingStrategy_(unbatchingStrategy) {
    increaseBatchReferenceCount(batchWireID_);
    for (auto& wire : individualWireIDs_) {
      increaseBatchReferenceCount(wire);
    }
  }

  ~RebatchingBooleanGate() override {
    decreaseBatchReferenceCount(batchWireID_);
    for (auto& i : individualWireIDs_) {
      decreaseBatchReferenceCount(i);
    }
  }

  void compute(engine::ISecretShareEngine&, std::map<int64_t, IGate::Secrets>&)
      override {
    switch (gateType_) {
      case GateType::Batching:
        executeBatchingGate();
        break;
      case GateType::Unbatching:
        executeUnbatchingGate();
        break;
    }
  }

  void collectScheduledResult(
      engine::ISecretShareEngine&,
      std::map<int64_t, IGate::Secrets>&) override {}

  uint32_t getNumberOfResults() const override {
    return 0;
  }

  std::vector<IScheduler::WireId<IScheduler::Boolean>> getIndividualWireIDs()
      const {
    return individualWireIDs_;
  }

  IScheduler::WireId<IScheduler::Boolean> getBatchWireID() const {
    return batchWireID_;
  }

  bool isBatching() const {
    return gateType_ == GateType::Batching;
  }

 private:
  void increaseBatchReferenceCount(
      IScheduler::WireId<IScheduler::Boolean> wire) {
    if (!wire.isEmpty()) {
      wireKeeper_.increaseBatchReferenceCount(wire);
    }
  }

  void decreaseBatchReferenceCount(
      IScheduler::WireId<IScheduler::Boolean> wire) {
    if (!wire.isEmpty()) {
      wireKeeper_.decreaseBatchReferenceCount(wire);
    }
  }

  void executeBatchingGate() const {
    size_t batchIndex = 0;
    size_t totalSize = 0;
    std::vector<std::reference_wrapper<const std::vector<bool>>> batchValues(
        individualWireIDs_.size(),
        wireKeeper_.getBatchBooleanValue(individualWireIDs_.at(0)));

    for (size_t i = 0; i < individualWireIDs_.size(); i++) {
      batchValues[i] =
          wireKeeper_.getBatchBooleanValue(individualWireIDs_.at(i));
      totalSize += batchValues.at(i).get().size();
    }

    std::vector<bool> dst(totalSize);

    for (size_t i = 0; i < individualWireIDs_.size(); i++) {
      for (size_t j = 0; j < batchValues.at(i).get().size(); j++) {
        dst[batchIndex++] = batchValues.at(i).get().at(j);
      }
    }
    wireKeeper_.setBatchBooleanValue(batchWireID_, dst);
  }

  void executeUnbatchingGate() const {
    size_t batchIndex = 0;
    auto& src = wireKeeper_.getBatchBooleanValue(batchWireID_);
    for (size_t i = 0; i < unbatchingStrategy_->size(); i++) {
      std::vector<bool> dst(unbatchingStrategy_->at(i));
      for (size_t j = 0; j < dst.size(); j++) {
        dst[j] = src.at(batchIndex++);
      }
      wireKeeper_.setBatchBooleanValue(individualWireIDs_.at(i), dst);
    }
  }

  GateType gateType_;
  std::vector<IScheduler::WireId<IScheduler::Boolean>> individualWireIDs_;
  IScheduler::WireId<IScheduler::Boolean> batchWireID_;
  IWireKeeper& wireKeeper_;
  std::shared_ptr<std::vector<uint32_t>> unbatchingStrategy_;
};

} // namespace fbpcf::scheduler
