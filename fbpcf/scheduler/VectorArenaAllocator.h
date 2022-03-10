/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdint.h>
#include <array>
#include <optional>
#include <queue>
#include <stdexcept>

#include "fbpcf/scheduler/IAllocator.h"

namespace fbpcf::scheduler {

/**
 * This class provides access to a large, contiguous block of memory that is
 * allocated once and increased periodically. This can significantly reduce the
 * number of system calls to allocate/deallocate memory.
 *
 * The "unsafe" version is more performant, but does not guard against accessing
 * unallocated or freed memory locations.
 */
template <typename T, bool unsafe>
class VectorArenaAllocator final : public IAllocator<T> {
 public:
  VectorArenaAllocator() : nextAvailableBlockIndex_{0} {
    increaseAllocation();
  }

  /**
   * @inherit doc
   */
  uint64_t allocate(T&& value) override {
    if (nextAvailableBlockIndex_ >= blocks_.size()) {
      increaseAllocation();
    }
    auto id = freeBlocks_[nextAvailableBlockIndex_++];
    blocks_[id] = value;
    return id;
  }

  /**
   * @inherit doc
   */
  void free(uint64_t id) override {
    freeBlocks_[--nextAvailableBlockIndex_] = id;
    if constexpr (!unsafe) {
      blocks_[id] = std::nullopt;
    }
  }

  /**
   * @inherit doc
   */
  const T& get(uint64_t id) const override {
    if constexpr (unsafe) {
      return blocks_.at(id);
    } else {
      if (blocks_.at(id) == std::nullopt) {
        throw std::runtime_error(IAllocator<T>::errorMessageCannotFindItem(id));
      }
      return *blocks_.at(id);
    }
  }

  /**
   * @inherit doc
   */
  T& getWritableReference(uint64_t id) override {
    if constexpr (unsafe) {
      return blocks_.at(id);
    } else {
      if (blocks_.at(id) == std::nullopt) {
        throw std::runtime_error(IAllocator<T>::errorMessageCannotFindItem(id));
      }
      return *blocks_.at(id);
    }
  }

  bool isSafe() const override {
    return !unsafe;
  }

 private:
  static const uint16_t kChunkSize = 1024;

  using BlockType =
      typename std::conditional<unsafe, T, std::optional<T>>::type;

  std::vector<BlockType> blocks_;
  std::vector<uint64_t> freeBlocks_;
  uint64_t nextAvailableBlockIndex_;

  void increaseAllocation() {
    auto allocatedSize = blocks_.size();
    std::vector<uint64_t> newFreeBlocks(kChunkSize);
    for (auto i = 0; i < kChunkSize; ++i) {
      newFreeBlocks[i] = allocatedSize + i;
    }

    freeBlocks_.insert(
        freeBlocks_.end(), newFreeBlocks.begin(), newFreeBlocks.end());

    std::vector<BlockType> newBlocks;
    if constexpr (unsafe) {
      newBlocks = std::vector<BlockType>(kChunkSize);
    } else {
      newBlocks = std::vector<BlockType>(kChunkSize, std::nullopt);
    }
    blocks_.insert(blocks_.end(), newBlocks.begin(), newBlocks.end());
  }
};

} // namespace fbpcf::scheduler
