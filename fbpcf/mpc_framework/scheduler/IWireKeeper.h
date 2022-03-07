/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <memory>

#include "fbpcf/mpc_framework/scheduler/IScheduler.h"

namespace fbpcf::mpc_framework::scheduler {
/**
 * A wire keeper is the object that stores all the live wires and free
 * upon request.
 */

class IWireKeeper {
 public:
  virtual ~IWireKeeper() {}
  // create a boolean wire with value v, return its wire id.
  virtual IScheduler::WireId<IScheduler::Boolean> allocateBooleanValue(
      bool v = false,
      uint32_t firstAvailableLevel = 0) = 0;

  // create an integer wire with value v, returns its wire id.
  virtual IScheduler::WireId<IScheduler::Arithmetic> allocateIntegerValue(
      uint64_t v = 0,
      uint32_t firstAvailableLevel = 0) = 0;

  // get the value associated with boolean wire with given id.
  virtual bool getBooleanValue(
      IScheduler::WireId<IScheduler::Boolean> id) const = 0;

  // get the value associated with integer wire with given id.
  virtual uint64_t getIntegerValue(
      IScheduler::WireId<IScheduler::Arithmetic> id) const = 0;

  // set the value associated with boolean wire with given id.
  virtual void setBooleanValue(
      IScheduler::WireId<IScheduler::Boolean> id,
      bool v) = 0;

  // set the value associated with integer wire with given id.
  virtual void setIntegerValue(
      IScheduler::WireId<IScheduler::Arithmetic> id,
      uint64_t v) = 0;

  // get the level when the wire with the given ID will have its value set.
  virtual uint32_t getFirstAvailableLevel(
      IScheduler::WireId<IScheduler::Boolean> id) const = 0;

  // get the level when the wire with the given ID will have its value set.
  virtual uint32_t getFirstAvailableLevel(
      IScheduler::WireId<IScheduler::Arithmetic> id) const = 0;

  // set the level associated with boolean wire with given id.
  virtual void setFirstAvailableLevel(
      IScheduler::WireId<IScheduler::Boolean> id,
      uint32_t level) = 0;

  // set the level associated with boolean wire with given id.
  virtual void setFirstAvailableLevel(
      IScheduler::WireId<IScheduler::Arithmetic> id,
      uint32_t level) = 0;

  // increase the reference count on the wire with given id
  virtual void increaseReferenceCount(
      IScheduler::WireId<IScheduler::Boolean> id) = 0;

  // increase the reference count on the wire with given id
  virtual void increaseReferenceCount(
      IScheduler::WireId<IScheduler::Arithmetic> id) = 0;

  // decrease the reference count on the wire with given id, if the count
  // decreases to 0, free this wire
  virtual void decreaseReferenceCount(
      IScheduler::WireId<IScheduler::Boolean> id) = 0;

  // decrease the reference count on the wire with given id, if the count
  // decreases to 0, free this wire
  virtual void decreaseReferenceCount(
      IScheduler::WireId<IScheduler::Arithmetic> id) = 0;

  // create a boolean wire with values v, return its wire id.
  virtual IScheduler::WireId<IScheduler::Boolean> allocateBatchBooleanValue(
      const std::vector<bool>& v,
      uint32_t firstAvailableLevel = 0) = 0;

  // create an integer wire with values v, return its wire id.
  virtual IScheduler::WireId<IScheduler::Arithmetic> allocateBatchIntegerValue(
      const std::vector<uint64_t>& v,
      uint32_t firstAvailableLevel = 0) = 0;

  // get the batch of value associated with boolean wire with given id.
  virtual const std::vector<bool>& getBatchBooleanValue(
      IScheduler::WireId<IScheduler::Boolean> id) const = 0;

  // get the batch of value associated with integer wire with given id.
  virtual const std::vector<uint64_t>& getBatchIntegerValue(
      IScheduler::WireId<IScheduler::Arithmetic> id) const = 0;

  // set the value associated with boolean wire with given id.
  virtual void setBatchBooleanValue(
      IScheduler::WireId<IScheduler::Boolean> id,
      const std::vector<bool>& v) = 0;

  // set the value associated with integer wire with given id.
  virtual void setBatchIntegerValue(
      IScheduler::WireId<IScheduler::Arithmetic> id,
      const std::vector<uint64_t>& v) = 0;

  // get the level when the wire with the given ID will have its value set.
  virtual uint32_t getBatchFirstAvailableLevel(
      IScheduler::WireId<IScheduler::Boolean> id) const = 0;

  // get the level when the wire with the given ID will have its value set.
  virtual uint32_t getBatchFirstAvailableLevel(
      IScheduler::WireId<IScheduler::Arithmetic> id) const = 0;

  // set the level associated with boolean wire with given id.
  virtual void setBatchFirstAvailableLevel(
      IScheduler::WireId<IScheduler::Boolean> id,
      uint32_t level) = 0;

  // set the level associated with boolean wire with given id.
  virtual void setBatchFirstAvailableLevel(
      IScheduler::WireId<IScheduler::Arithmetic> id,
      uint32_t level) = 0;

  // increase the reference count on the wire with given id
  virtual void increaseBatchReferenceCount(
      IScheduler::WireId<IScheduler::Boolean> id) = 0;

  // increase the reference count on the wire with given id
  virtual void increaseBatchReferenceCount(
      IScheduler::WireId<IScheduler::Arithmetic> id) = 0;

  // decrease the reference count on the wire with given id, if the count
  // decreases to 0, free this wire
  virtual void decreaseBatchReferenceCount(
      IScheduler::WireId<IScheduler::Boolean> id) = 0;

  // decrease the reference count on the wire with given id, if the count
  // decreases to 0, free this wire
  virtual void decreaseBatchReferenceCount(
      IScheduler::WireId<IScheduler::Arithmetic> id) = 0;

  // Return a pair of the number of wires (allocated, deallocated).
  std::pair<uint64_t, uint64_t> getWireStatistics() const {
    return {wiresAllocated_, wiresDeallocated_};
  }

 protected:
  uint64_t wiresAllocated_ = 0;
  uint64_t wiresDeallocated_ = 0;

  template <typename T>
  struct WireRecord {
    T v;
    uint32_t firstAvailableLevel;
    uint32_t referenceCount;
  };
};

} // namespace fbpcf::mpc_framework::scheduler
