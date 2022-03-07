/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdint.h>
#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "fbpcf/mpc_framework/scheduler/IAllocator.h"
#include "fbpcf/mpc_framework/scheduler/IWireKeeper.h"
#include "fbpcf/mpc_framework/scheduler/UnorderedMapAllocator.h"
#include "fbpcf/mpc_framework/scheduler/VectorArenaAllocator.h"

namespace fbpcf::mpc_framework::scheduler {

class WireKeeper final : public IWireKeeper {
 public:
  WireKeeper(
      std::unique_ptr<IAllocator<WireRecord<bool>>> boolAllocator,
      std::unique_ptr<IAllocator<WireRecord<std::vector<bool>>>>
          boolBatchAllocator,
      std::unique_ptr<IAllocator<WireRecord<uint64_t>>> intAllocator,
      std::unique_ptr<IAllocator<WireRecord<std::vector<uint64_t>>>>
          intBatchAllocator_)
      : boolAllocator_{std::move(boolAllocator)},
        boolBatchAllocator_{std::move(boolBatchAllocator)},
        intAllocator_{std::move(intAllocator)},
        intBatchAllocator_{std::move(intBatchAllocator_)} {}

  template <bool unsafe>
  static std::unique_ptr<IWireKeeper> createWithVectorArena() {
    return std::make_unique<WireKeeper>(
        std::make_unique<VectorArenaAllocator<WireRecord<bool>, unsafe>>(),
        std::make_unique<
            VectorArenaAllocator<WireRecord<std::vector<bool>>, unsafe>>(),
        std::make_unique<VectorArenaAllocator<WireRecord<uint64_t>, unsafe>>(),
        std::make_unique<
            VectorArenaAllocator<WireRecord<std::vector<uint64_t>>, unsafe>>());
  }

  static std::unique_ptr<IWireKeeper> createWithUnorderedMap() {
    return std::make_unique<WireKeeper>(
        std::make_unique<UnorderedMapAllocator<WireRecord<bool>>>(),
        std::make_unique<
            UnorderedMapAllocator<WireRecord<std::vector<bool>>>>(),
        std::make_unique<UnorderedMapAllocator<WireRecord<uint64_t>>>(),
        std::make_unique<
            UnorderedMapAllocator<WireRecord<std::vector<uint64_t>>>>());
  }

  /**
   * @inherit doc
   */
  IScheduler::WireId<IScheduler::Boolean> allocateBooleanValue(
      bool v = 0,
      uint32_t firstAvailableLevel = 0) override;

  /**
   * @inherit doc
   */
  IScheduler::WireId<IScheduler::Arithmetic> allocateIntegerValue(
      uint64_t v = 0,
      uint32_t firstAvailableLevel = 0) override;

  /**
   * @inherit doc
   */
  bool getBooleanValue(
      IScheduler::WireId<IScheduler::Boolean> id) const override;

  /**
   * @inherit doc
   */
  uint64_t getIntegerValue(
      IScheduler::WireId<IScheduler::Arithmetic> id) const override;

  /**
   * @inherit doc
   */
  void setBooleanValue(IScheduler::WireId<IScheduler::Boolean> id, bool v)
      override;

  /**
   * @inherit doc
   */
  void setIntegerValue(
      IScheduler::WireId<IScheduler::Arithmetic> id,
      uint64_t v) override;

  /**
   * @inherit doc
   */
  uint32_t getFirstAvailableLevel(
      IScheduler::WireId<IScheduler::Boolean> id) const override;

  /**
   * @inherit doc
   */
  uint32_t getFirstAvailableLevel(
      IScheduler::WireId<IScheduler::Arithmetic> id) const override;

  /**
   * @inherit doc
   */
  void setFirstAvailableLevel(
      IScheduler::WireId<IScheduler::Boolean> id,
      uint32_t level) override;

  /**
   * @inherit doc
   */
  void setFirstAvailableLevel(
      IScheduler::WireId<IScheduler::Arithmetic> id,
      uint32_t level) override;

  /**
   * @inherit doc
   */
  void increaseReferenceCount(
      IScheduler::WireId<IScheduler::Boolean> id) override;

  /**
   * @inherit doc
   */
  void increaseReferenceCount(
      IScheduler::WireId<IScheduler::Arithmetic> id) override;

  /**
   * @inherit doc
   */
  void decreaseReferenceCount(
      IScheduler::WireId<IScheduler::Boolean> id) override;

  /**
   * @inherit doc
   */
  void decreaseReferenceCount(
      IScheduler::WireId<IScheduler::Arithmetic> id) override;

  /**
   * @inherit doc
   */
  IScheduler::WireId<IScheduler::Boolean> allocateBatchBooleanValue(
      const std::vector<bool>& v,
      uint32_t firstAvailableLevel = 0) override;

  /**
   * @inherit doc
   */
  IScheduler::WireId<IScheduler::Arithmetic> allocateBatchIntegerValue(
      const std::vector<uint64_t>& v,
      uint32_t firstAvailableLevel = 0) override;

  /**
   * @inherit doc
   */
  const std::vector<bool>& getBatchBooleanValue(
      IScheduler::WireId<IScheduler::Boolean> id) const override;

  /**
   * @inherit doc
   */
  const std::vector<uint64_t>& getBatchIntegerValue(
      IScheduler::WireId<IScheduler::Arithmetic> id) const override;

  /**
   * @inherit doc
   */
  void setBatchBooleanValue(
      IScheduler::WireId<IScheduler::Boolean> id,
      const std::vector<bool>& v) override;

  /**
   * @inherit doc
   */
  void setBatchIntegerValue(
      IScheduler::WireId<IScheduler::Arithmetic> id,
      const std::vector<uint64_t>& v) override;

  /**
   * @inherit doc
   */
  uint32_t getBatchFirstAvailableLevel(
      IScheduler::WireId<IScheduler::Boolean> id) const override;

  /**
   * @inherit doc
   */
  uint32_t getBatchFirstAvailableLevel(
      IScheduler::WireId<IScheduler::Arithmetic> id) const override;

  /**
   * @inherit doc
   */
  void setBatchFirstAvailableLevel(
      IScheduler::WireId<IScheduler::Boolean> id,
      uint32_t level) override;

  /**
   * @inherit doc
   */
  void setBatchFirstAvailableLevel(
      IScheduler::WireId<IScheduler::Arithmetic> id,
      uint32_t level) override;

  /**
   * @inherit doc
   */
  void increaseBatchReferenceCount(
      IScheduler::WireId<IScheduler::Boolean> id) override;

  /**
   * @inherit doc
   */
  void increaseBatchReferenceCount(
      IScheduler::WireId<IScheduler::Arithmetic> id) override;

  /**
   * @inherit doc
   */
  void decreaseBatchReferenceCount(
      IScheduler::WireId<IScheduler::Boolean> id) override;

  /**
   * @inherit doc
   */
  void decreaseBatchReferenceCount(
      IScheduler::WireId<IScheduler::Arithmetic> id) override;

 private:
  std::unique_ptr<IAllocator<WireRecord<bool>>> boolAllocator_;
  std::unique_ptr<IAllocator<WireRecord<std::vector<bool>>>>
      boolBatchAllocator_;
  std::unique_ptr<IAllocator<WireRecord<uint64_t>>> intAllocator_;
  std::unique_ptr<IAllocator<WireRecord<std::vector<uint64_t>>>>
      intBatchAllocator_;
};

} // namespace fbpcf::mpc_framework::scheduler
