/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/mpc_framework/scheduler/WireKeeper.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <cstdint>
#include <stdexcept>
#include "fbpcf/mpc_framework/test/TestHelper.h"

namespace fbpcf::scheduler {

void wireKeeperTestAllocateSetAndGet(std::unique_ptr<IWireKeeper> wireKeeper) {
  // Non batch API: Bool
  auto wire1 = wireKeeper->allocateBooleanValue();
  wireKeeper->setBooleanValue(wire1, true);
  EXPECT_TRUE(wireKeeper->getBooleanValue(wire1));

  auto wire2 = wireKeeper->allocateBooleanValue(true);
  EXPECT_TRUE(wireKeeper->getBooleanValue(wire2));

  auto wire3 = wireKeeper->allocateBooleanValue(false);
  EXPECT_FALSE(wireKeeper->getBooleanValue(wire3));

  // Non batch API: Int
  auto wire4 = wireKeeper->allocateIntegerValue();
  wireKeeper->setIntegerValue(wire4, UINT64_MAX);
  EXPECT_EQ(wireKeeper->getIntegerValue(wire4), UINT64_MAX);

  auto wire5 = wireKeeper->allocateIntegerValue();
  wireKeeper->setIntegerValue(wire5, UINT64_MAX / 2);
  EXPECT_EQ(wireKeeper->getIntegerValue(wire5), UINT64_MAX / 2);

  auto wire6 = wireKeeper->allocateIntegerValue();
  wireKeeper->setIntegerValue(wire6, 0);
  EXPECT_EQ(wireKeeper->getIntegerValue(wire6), 0);

  // Batch API: Bool
  std::vector<bool> testValue1(true, 3);
  auto wire7 = wireKeeper->allocateBatchBooleanValue(testValue1);
  testVectorEq(wireKeeper->getBatchBooleanValue(wire7), testValue1);
  std::vector<bool> testValue2(false, 4);
  wireKeeper->setBatchBooleanValue(wire7, testValue2);
  testVectorEq(wireKeeper->getBatchBooleanValue(wire7), testValue2);

  // Batch API: Int

  std::vector<uint64_t> testValue3({UINT64_MAX, UINT64_MAX, 0});
  auto wire8 = wireKeeper->allocateBatchIntegerValue(testValue3);
  testVectorEq(wireKeeper->getBatchIntegerValue(wire8), testValue3);

  std::vector<uint64_t> testValue4({10, 11, 12});
  wireKeeper->setBatchIntegerValue(wire8, testValue4);
  testVectorEq(wireKeeper->getBatchIntegerValue(wire8), testValue4);
}

TEST(UnorderedMapWireKeeperTest, testAllocateSetAndGet) {
  wireKeeperTestAllocateSetAndGet(WireKeeper::createWithUnorderedMap());
}

TEST(UnsafeVectorArenaWireKeeperTest, testAllocateSetAndGet) {
  wireKeeperTestAllocateSetAndGet(
      WireKeeper::createWithVectorArena</*unsafe*/ true>());
}

TEST(SafeVectorArenaWireKeeperTest, testAllocateSetAndGet) {
  wireKeeperTestAllocateSetAndGet(
      WireKeeper::createWithVectorArena</*unsafe*/ false>());
}

void wireKeeperTestAvailableLevel(std::unique_ptr<IWireKeeper> wireKeeper) {
  // Non batch API: Bool
  auto wire1 =
      wireKeeper->allocateBooleanValue(true, /*firstAvailableLevel*/ 3);
  EXPECT_EQ(wireKeeper->getFirstAvailableLevel(wire1), 3);

  wireKeeper->setFirstAvailableLevel(wire1, 6);
  EXPECT_EQ(wireKeeper->getFirstAvailableLevel(wire1), 6);

  // Non batch API: Int
  auto wire2 = wireKeeper->allocateIntegerValue(5, /*firstAvailableLevel*/ 8);
  EXPECT_EQ(wireKeeper->getFirstAvailableLevel(wire2), 8);
  wireKeeper->setFirstAvailableLevel(wire2, 12);
  EXPECT_EQ(wireKeeper->getFirstAvailableLevel(wire2), 12);

  // Batch API: Bool
  auto wire3 = wireKeeper->allocateBatchBooleanValue(
      {true, false}, /*firstAvailableLevel*/ 9);
  EXPECT_EQ(wireKeeper->getBatchFirstAvailableLevel(wire3), 9);

  wireKeeper->setBatchFirstAvailableLevel(wire3, 2);
  EXPECT_EQ(wireKeeper->getBatchFirstAvailableLevel(wire3), 2);

  // Batch API: Int
  auto wire4 = wireKeeper->allocateBatchIntegerValue(
      {12, 24}, /* firstAvaialbleLevel*/ 25);
  EXPECT_EQ(wireKeeper->getBatchFirstAvailableLevel(wire4), 25);

  wireKeeper->setBatchFirstAvailableLevel(wire4, 1337);
  EXPECT_EQ(wireKeeper->getBatchFirstAvailableLevel(wire4), 1337);
}

TEST(UnorderedMapWireKeeperTest, testAvailableLevel) {
  wireKeeperTestAvailableLevel(WireKeeper::createWithUnorderedMap());
}

TEST(UnsafeVectorArenaWireKeeperTest, testAvailableLevel) {
  wireKeeperTestAvailableLevel(
      WireKeeper::createWithVectorArena</*unsafe*/ true>());
}

TEST(SafeVectorArenaWireKeeperTest, testAvailableLevel) {
  wireKeeperTestAvailableLevel(
      WireKeeper::createWithVectorArena</*unsafe*/ false>());
}

void wireKeeperTestReferenceCount(std::unique_ptr<IWireKeeper> wireKeeper) {
  // Non batch API: Bool
  auto boolWire = wireKeeper->allocateBooleanValue(true);

  for (int i = 0; i < 10; i++) {
    if (i % 3 == 2) {
      wireKeeper->decreaseReferenceCount(boolWire);
    } else {
      wireKeeper->increaseReferenceCount(boolWire);
    }
    EXPECT_TRUE(wireKeeper->getBooleanValue(boolWire));
  }

  for (int i = 0; i < 10; i++) {
    EXPECT_TRUE(wireKeeper->getBooleanValue(boolWire));
    if (i % 3 != 2) {
      wireKeeper->decreaseReferenceCount(boolWire);
    } else {
      wireKeeper->increaseReferenceCount(boolWire);
    }
  }

  testPairEq(wireKeeper->getWireStatistics(), {1, 0});

  wireKeeper->decreaseReferenceCount(boolWire);
  EXPECT_THROW(wireKeeper->getBooleanValue(boolWire), std::runtime_error);

  testPairEq(wireKeeper->getWireStatistics(), {1, 1});

  // Non batch API: Int
  auto intWire = wireKeeper->allocateIntegerValue(10);

  for (int i = 0; i < 10; i++) {
    if (i % 3 == 2) {
      wireKeeper->decreaseReferenceCount(intWire);
    } else {
      wireKeeper->increaseReferenceCount(intWire);
    }
    EXPECT_EQ(wireKeeper->getIntegerValue(intWire), 10);
  }

  for (int i = 0; i < 10; i++) {
    EXPECT_EQ(wireKeeper->getIntegerValue(intWire), 10);
    if (i % 3 != 2) {
      wireKeeper->decreaseReferenceCount(intWire);
    } else {
      wireKeeper->increaseReferenceCount(intWire);
    }
  }

  testPairEq(wireKeeper->getWireStatistics(), {2, 1});

  wireKeeper->decreaseReferenceCount(intWire);
  EXPECT_THROW(wireKeeper->getIntegerValue(intWire), std::runtime_error);

  testPairEq(wireKeeper->getWireStatistics(), {2, 2});

  // Batch API: Bool
  std::vector<bool> testBoolValue(true, 3);
  auto batchBoolWire = wireKeeper->allocateBatchBooleanValue(testBoolValue);

  for (int i = 0; i < 10; i++) {
    if (i % 3 == 2) {
      wireKeeper->decreaseBatchReferenceCount(batchBoolWire);
    } else {
      wireKeeper->increaseBatchReferenceCount(batchBoolWire);
    }
    testVectorEq(
        wireKeeper->getBatchBooleanValue(batchBoolWire), testBoolValue);
  }

  for (int i = 0; i < 10; i++) {
    testVectorEq(
        wireKeeper->getBatchBooleanValue(batchBoolWire), testBoolValue);
    if (i % 3 != 2) {
      wireKeeper->decreaseBatchReferenceCount(batchBoolWire);
    } else {
      wireKeeper->increaseBatchReferenceCount(batchBoolWire);
    }
  }

  testPairEq(wireKeeper->getWireStatistics(), {3, 2});

  wireKeeper->decreaseBatchReferenceCount(batchBoolWire);
  EXPECT_THROW(
      wireKeeper->getBatchBooleanValue(batchBoolWire), std::runtime_error);

  testPairEq(wireKeeper->getWireStatistics(), {3, 3});

  // Batch API: Int
  std::vector<uint64_t> testIntValue({3, 4, 5});
  auto batchIntWire = wireKeeper->allocateBatchIntegerValue(testIntValue);

  for (int i = 0; i < 10; i++) {
    if (i % 3 == 2) {
      wireKeeper->decreaseBatchReferenceCount(batchIntWire);
    } else {
      wireKeeper->increaseBatchReferenceCount(batchIntWire);
    }
    testVectorEq(wireKeeper->getBatchIntegerValue(batchIntWire), testIntValue);
  }

  for (int i = 0; i < 10; i++) {
    testVectorEq(wireKeeper->getBatchIntegerValue(batchIntWire), testIntValue);
    if (i % 3 != 2) {
      wireKeeper->decreaseBatchReferenceCount(batchIntWire);
    } else {
      wireKeeper->increaseBatchReferenceCount(batchIntWire);
    }
  }

  testPairEq(wireKeeper->getWireStatistics(), {4, 3});

  wireKeeper->decreaseBatchReferenceCount(batchIntWire);
  EXPECT_THROW(
      wireKeeper->getBatchIntegerValue(batchIntWire), std::runtime_error);

  testPairEq(wireKeeper->getWireStatistics(), {4, 4});
}

TEST(UnorderedMapWireKeeperTest, testReferenceCount) {
  wireKeeperTestReferenceCount(WireKeeper::createWithUnorderedMap());
}

// The unsafe vector arena can't be tested here since it doesn't throw errors
// when accessing a freed wire.

TEST(SafeVectorArenaWireKeeperTest, testReferenceCount) {
  wireKeeperTestReferenceCount(
      WireKeeper::createWithVectorArena</*unsafe*/ false>());
}
} // namespace fbpcf::scheduler
