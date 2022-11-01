/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdint.h>
#include <string>

/**
 * An instance of this class provides operations for allocating and freeing
 * memory.
 */
namespace fbpcf::scheduler {

template <typename T>
class IAllocator {
 public:
  virtual ~IAllocator() = default;

  /* Allocate a value of type T and return an ID reference to it.
   * This method invalidates any references returned by get or
   * getWritableReference
   */
  virtual uint64_t allocate(T&& value) = 0;

  // Free the object with the given ID reference.
  virtual void free(uint64_t id) = 0;

  // Get the value with given ID reference.
  virtual const T& get(uint64_t id) const = 0;

  // Same as above, but returns a non-const T.
  virtual T& getWritableReference(uint64_t id) = 0;

  // Does this allocator throw errors when accessing unallocated/freed memory
  // locations?
  virtual bool isSafe() const = 0;

 protected:
  std::string errorMessageCannotFindItem(uint64_t id) const {
    return std::string("Can't find item with ID " + std::to_string(id) + "!");
  }
};

} // namespace fbpcf::scheduler
