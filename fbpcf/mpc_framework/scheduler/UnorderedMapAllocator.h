/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <unordered_map>

#include "fbpcf/mpc_framework/scheduler/IAllocator.h"

namespace fbpcf::scheduler {

/**
 * This class uses an std::unordered_map as its underlying storage.
 */
template <typename T>
class UnorderedMapAllocator final : public IAllocator<T> {
 public:
  /**
   * @inherit doc
   */
  uint64_t allocate(T&& value) override {
    map_.emplace(nextId_, value);
    return nextId_++;
  }

  /**
   * @inherit doc
   */
  void free(uint64_t id) override {
    map_.erase(id);
  }

  /**
   * @inherit doc
   */
  const T& get(uint64_t id) const override {
    auto item = map_.find(id);
    if (item == map_.end()) {
      throw std::runtime_error(IAllocator<T>::errorMessageCannotFindItem(id));
    }
    return item->second;
  }

  /**
   * @inherit doc
   */
  T& getWritableReference(uint64_t id) override {
    auto item = map_.find(id);
    if (item == map_.end()) {
      throw std::runtime_error(IAllocator<T>::errorMessageCannotFindItem(id));
    }
    return item->second;
  }

  bool isSafe() const override {
    return true;
  }

 private:
  std::unordered_map<uint64_t, T> map_;
  uint64_t nextId_ = 1;
};

} // namespace fbpcf::scheduler
