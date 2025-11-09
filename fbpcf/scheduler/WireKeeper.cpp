/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/scheduler/WireKeeper.h"
#include <folly/logging/xlog.h>
#include <cstdint>
#include <string>
#include "fbpcf/scheduler/IScheduler.h"

namespace fbpcf::scheduler {

IScheduler::WireId<IScheduler::Boolean> WireKeeper::allocateBooleanValue(
    bool v,
    uint32_t firstAvailableLevel) {
  wiresAllocated_++;
  auto wireID = boolAllocator_->allocate(WireRecord<bool, false>{
      .v = v,
      .firstAvailableLevel = firstAvailableLevel,
      .referenceCount = 1,
  });
  return IScheduler::WireId<IScheduler::Boolean>(wireID);
}

IScheduler::WireId<IScheduler::Arithmetic> WireKeeper::allocateIntegerValue(
    uint64_t v,
    uint32_t firstAvailableLevel) {
  wiresAllocated_++;
  auto wireID = intAllocator_->allocate(WireRecord<uint64_t, false>{
      .v = v,
      .firstAvailableLevel = firstAvailableLevel,
      .referenceCount = 1,
  });
  return IScheduler::WireId<IScheduler::Arithmetic>(wireID);
}

bool WireKeeper::getBooleanValue(
    IScheduler::WireId<IScheduler::Boolean> id) const {
  return boolAllocator_->get(id.getId()).v;
}

uint64_t WireKeeper::getIntegerValue(
    IScheduler::WireId<IScheduler::Arithmetic> id) const {
  return intAllocator_->get(id.getId()).v;
}

void WireKeeper::setBooleanValue(
    IScheduler::WireId<IScheduler::Boolean> id,
    bool v) {
  boolAllocator_->getWritableReference(id.getId()).v = v;
}

void WireKeeper::setIntegerValue(
    IScheduler::WireId<IScheduler::Arithmetic> id,
    uint64_t v) {
  intAllocator_->getWritableReference(id.getId()).v = v;
}

uint32_t WireKeeper::getFirstAvailableLevel(
    IScheduler::WireId<IScheduler::Boolean> id) const {
  return boolAllocator_->get(id.getId()).firstAvailableLevel;
}

uint32_t WireKeeper::getFirstAvailableLevel(
    IScheduler::WireId<IScheduler::Arithmetic> id) const {
  return intAllocator_->get(id.getId()).firstAvailableLevel;
}

void WireKeeper::setFirstAvailableLevel(
    IScheduler::WireId<IScheduler::Boolean> id,
    uint32_t level) {
  boolAllocator_->getWritableReference(id.getId()).firstAvailableLevel = level;
}

void WireKeeper::setFirstAvailableLevel(
    IScheduler::WireId<IScheduler::Arithmetic> id,
    uint32_t level) {
  intAllocator_->getWritableReference(id.getId()).firstAvailableLevel = level;
}

void WireKeeper::increaseReferenceCount(
    IScheduler::WireId<IScheduler::Boolean> id) {
  boolAllocator_->getWritableReference(id.getId()).referenceCount++;
}

void WireKeeper::increaseReferenceCount(
    IScheduler::WireId<IScheduler::Arithmetic> id) {
  intAllocator_->getWritableReference(id.getId()).referenceCount++;
}

void WireKeeper::decreaseReferenceCount(
    IScheduler::WireId<IScheduler::Boolean> id) {
  if (--boolAllocator_->getWritableReference(id.getId()).referenceCount == 0) {
    wiresDeallocated_++;
    boolAllocator_->free(id.getId());
  }
}

void WireKeeper::decreaseReferenceCount(
    IScheduler::WireId<IScheduler::Arithmetic> id) {
  if (--intAllocator_->getWritableReference(id.getId()).referenceCount == 0) {
    wiresDeallocated_++;
    intAllocator_->free(id.getId());
  }
}

size_t WireKeeper::getBatchSize(
    IScheduler::WireId<IScheduler::Boolean> id) const {
  return boolBatchAllocator_->get(id.getId()).expectedBatchSize;
}

size_t WireKeeper::getBatchSize(
    IScheduler::WireId<IScheduler::Arithmetic> id) const {
  return intBatchAllocator_->get(id.getId()).expectedBatchSize;
}

IScheduler::WireId<IScheduler::Boolean> WireKeeper::allocateBatchBooleanValue(
    const std::vector<bool>& v,
    size_t expectedBatchSize,
    uint32_t firstAvailableLevel) {
  wiresAllocated_++;
  auto wireID =
      boolBatchAllocator_->allocate(WireRecord<std::vector<bool>, true>{
          .v = v,
          .firstAvailableLevel = firstAvailableLevel,
          .referenceCount = 1,
          .expectedBatchSize = expectedBatchSize,
      });
  return IScheduler::WireId<IScheduler::Boolean>(wireID);
}

IScheduler::WireId<IScheduler::Arithmetic>
WireKeeper::allocateBatchIntegerValue(
    const std::vector<uint64_t>& v,
    size_t expectedBatchSize,
    uint32_t firstAvailableLevel) {
  wiresAllocated_++;
  auto wireID =
      intBatchAllocator_->allocate(WireRecord<std::vector<uint64_t>, true>{
          .v = v,
          .firstAvailableLevel = firstAvailableLevel,
          .referenceCount = 1,
          .expectedBatchSize = expectedBatchSize,
      });
  return IScheduler::WireId<IScheduler::Arithmetic>(wireID);
}

const std::vector<bool>& WireKeeper::getBatchBooleanValue(
    IScheduler::WireId<IScheduler::Boolean> id) const {
  return boolBatchAllocator_->get(id.getId()).v;
}

const std::vector<uint64_t>& WireKeeper::getBatchIntegerValue(
    IScheduler::WireId<IScheduler::Arithmetic> id) const {
  return intBatchAllocator_->get(id.getId()).v;
}

std::vector<bool>& WireKeeper::getWritableBatchBooleanValue(
    IScheduler::WireId<IScheduler::Boolean> id) const {
  return boolBatchAllocator_->getWritableReference(id.getId()).v;
}

std::vector<uint64_t>& WireKeeper::getWritableBatchIntegerValue(
    IScheduler::WireId<IScheduler::Arithmetic> id) const {
  return intBatchAllocator_->getWritableReference(id.getId()).v;
}

void WireKeeper::setBatchBooleanValue(
    IScheduler::WireId<IScheduler::Boolean> id,
    const std::vector<bool>& v) {
  //  The following code will be used later once we set up a correct batch size
  //  for the wires during allocation.
  //  if
  //  (boolBatchAllocator_->getWritableReference(id.getId()).expectedBatchSize
  //  !=
  //      v.size()) {
  //    XLOG(ERR) << "Wire batch size is different from the size of Boolean
  //    vector";
  //  }
  boolBatchAllocator_->getWritableReference(id.getId()).v = v;
}

void WireKeeper::setBatchIntegerValue(
    IScheduler::WireId<IScheduler::Arithmetic> id,
    const std::vector<uint64_t>& v) {
  if (intBatchAllocator_->getWritableReference(id.getId()).expectedBatchSize !=
      v.size()) {
    XLOG(ERR) << "Wire batch size is different from the size of Integer vector";
    ;
  }
  intBatchAllocator_->getWritableReference(id.getId()).v = v;
}

uint32_t WireKeeper::getBatchFirstAvailableLevel(
    IScheduler::WireId<IScheduler::Boolean> id) const {
  return boolBatchAllocator_->get(id.getId()).firstAvailableLevel;
}

uint32_t WireKeeper::getBatchFirstAvailableLevel(
    IScheduler::WireId<IScheduler::Arithmetic> id) const {
  return intBatchAllocator_->get(id.getId()).firstAvailableLevel;
}

void WireKeeper::setBatchFirstAvailableLevel(
    IScheduler::WireId<IScheduler::Boolean> id,
    uint32_t level) {
  boolBatchAllocator_->getWritableReference(id.getId()).firstAvailableLevel =
      level;
}

void WireKeeper::setBatchFirstAvailableLevel(
    IScheduler::WireId<IScheduler::Arithmetic> id,
    uint32_t level) {
  intBatchAllocator_->getWritableReference(id.getId()).firstAvailableLevel =
      level;
}

void WireKeeper::increaseBatchReferenceCount(
    IScheduler::WireId<IScheduler::Boolean> id) {
  boolBatchAllocator_->getWritableReference(id.getId()).referenceCount++;
}

void WireKeeper::increaseBatchReferenceCount(
    IScheduler::WireId<IScheduler::Arithmetic> id) {
  intBatchAllocator_->getWritableReference(id.getId()).referenceCount++;
}

void WireKeeper::decreaseBatchReferenceCount(
    IScheduler::WireId<IScheduler::Boolean> id) {
  if (--boolBatchAllocator_->getWritableReference(id.getId()).referenceCount ==
      0) {
    wiresDeallocated_++;
    boolBatchAllocator_->free(id.getId());
  }
}

void WireKeeper::decreaseBatchReferenceCount(
    IScheduler::WireId<IScheduler::Arithmetic> id) {
  if (--intBatchAllocator_->getWritableReference(id.getId()).referenceCount ==
      0) {
    wiresDeallocated_++;
    intBatchAllocator_->free(id.getId());
  }
}
} // namespace fbpcf::scheduler
