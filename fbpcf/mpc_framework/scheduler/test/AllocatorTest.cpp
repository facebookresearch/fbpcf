/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include "fbpcf/mpc_framework/scheduler/IAllocator.h"
#include "fbpcf/mpc_framework/scheduler/UnorderedMapAllocator.h"
#include "fbpcf/mpc_framework/scheduler/VectorArenaAllocator.h"

namespace fbpcf::mpc_framework::scheduler {

template <typename T>
void testAllocator(std::unique_ptr<IAllocator<T>> arena) {
  if (arena->isSafe()) {
    // Error: attempt to get an unallocated slot
    EXPECT_THROW(arena->get(0), std::runtime_error);
  }

  for (auto i = 0; i < 5000 /*do a ton of iterations*/; ++i) {
    auto v1 = arena->allocate(i);
    EXPECT_EQ(arena->get(v1), i);

    // Free some of the slots
    if (i % 5 == 0) {
      arena->free(v1);
    }
  }

  arena->getWritableReference(3000) = 1234;
  EXPECT_EQ(arena->get(3000), 1234);

  if (arena->isSafe()) {
    // Error: attempt to get a freed slot
    arena->free(3000);
    EXPECT_THROW(arena->get(3000), std::runtime_error);
  }
}

TEST(UnsafeVectorArenaAllocatorTest, testAllocator) {
  testAllocator<int64_t>(
      std::make_unique<VectorArenaAllocator<int64_t, /*unsafe*/ true>>());
}

TEST(SafeVectorArenaAllocatorTest, testAllocator) {
  testAllocator<int64_t>(
      std::make_unique<VectorArenaAllocator<int64_t, /*unsafe*/ false>>());
}

TEST(UnorderedMapAllocatorTest, testAllocator) {
  testAllocator<int64_t>(std::make_unique<UnorderedMapAllocator<int64_t>>());
}
} // namespace fbpcf::mpc_framework::scheduler
