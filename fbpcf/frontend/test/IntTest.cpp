/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <random>
#include <stdexcept>

#include "fbpcf/frontend/Int.h"
#include "fbpcf/scheduler/PlaintextScheduler.h"
#include "fbpcf/scheduler/WireKeeper.h"
#include "fbpcf/test/TestHelper.h"

namespace fbpcf::frontend {

TEST(IntTest, testInputAndOutput) {
  const int8_t width = 63;
  scheduler::SchedulerKeeper<0>::setScheduler(
      std::make_unique<scheduler::PlaintextScheduler>(
          scheduler::WireKeeper::createWithUnorderedMap()));
  using secSignedInt = Integer<Secret<Signed<width>>, 0>;
  using pubSignedInt = Integer<Public<Signed<width>>, 0>;
  using secUnsignedInt = Integer<Secret<Unsigned<width>>, 0>;
  using pubUnsignedInt = Integer<Public<Unsigned<width>>, 0>;

  int partyId = 2;

  int64_t v1 = (int64_t(1) << (width - 1)) - 1;
  int64_t v2 = -1 - v1;
  uint64_t v3 = (uint64_t(1) << width) - 1;

  secSignedInt int1(v1, partyId);
  secSignedInt int2(v2, partyId);
  secUnsignedInt int3(v3, partyId);

  EXPECT_EQ(int1.openToParty(partyId).getValue(), v1);
  EXPECT_EQ(int2.openToParty(partyId).getValue(), v2);
  EXPECT_EQ(int3.openToParty(partyId).getValue(), v3);

  pubSignedInt int4(v1);
  pubSignedInt int5(v2);
  pubUnsignedInt int6(v3);

  EXPECT_EQ(int4.getValue(), v1);
  EXPECT_EQ(int5.getValue(), v2);
  EXPECT_EQ(int6.getValue(), v3);

  auto share1 = int1.extractIntShare();
  auto share2 = int3.extractIntShare();
  secSignedInt::ExtractedInt share3(v1);
  secSignedInt::ExtractedInt share4(v2);
  secUnsignedInt::ExtractedInt share5(v3);

  EXPECT_EQ(share3.getValue(), v1);
  EXPECT_EQ(share4.getValue(), v2);
  EXPECT_EQ(share5.getValue(), v3);

  secSignedInt int7(std::move(share1));
  secUnsignedInt int8(std::move(share2));
  secSignedInt int9(std::move(share3));
  secSignedInt int10(std::move(share4));
  secUnsignedInt int11(std::move(share5));

  EXPECT_EQ(int7.openToParty(partyId).getValue(), v1);
  EXPECT_EQ(int8.openToParty(partyId).getValue(), v3);
  EXPECT_EQ(int9.openToParty(partyId).getValue(), v1);
  EXPECT_EQ(int10.openToParty(partyId).getValue(), v2);
  EXPECT_EQ(int11.openToParty(partyId).getValue(), v3);

  EXPECT_THROW(secSignedInt(v1 + 1, partyId), std::runtime_error);
  EXPECT_THROW(secSignedInt(v2 - 1, partyId), std::runtime_error);
  EXPECT_THROW(secUnsignedInt(v3 + 1, partyId), std::runtime_error);

  EXPECT_THROW(pubSignedInt(v1 + 1), std::runtime_error);
  EXPECT_THROW(pubSignedInt(v2 - 1), std::runtime_error);
  EXPECT_THROW(pubUnsignedInt(v3 + 1), std::runtime_error);
}

TEST(IntTest, testInputAndOutputBatch) {
  const int8_t width = 15;
  scheduler::SchedulerKeeper<0>::setScheduler(
      std::make_unique<scheduler::PlaintextScheduler>(
          scheduler::WireKeeper::createWithUnorderedMap()));
  using secSignedIntBatch = Integer<Secret<Batch<Signed<width>>>, 0>;
  using pubSignedIntBatch = Integer<Public<Batch<Signed<width>>>, 0>;
  using secUnsignedIntBatch = Integer<Secret<Batch<Unsigned<width>>>, 0>;
  using pubUnsignedIntBatch = Integer<Public<Batch<Unsigned<width>>>, 0>;

  int partyId = 2;

  int size = 5;
  std::vector<int64_t> v1(size, (int64_t(1) << (width - 1)) - 1);
  std::vector<int64_t> v2(size, -1 - v1[0]);
  std::vector<uint64_t> v3(size, (uint64_t(1) << width) - 1);

  std::vector<int16_t> shortV1(size, (int64_t(1) << (width - 1)) - 1);
  std::vector<int16_t> shortV2(size, -1 - v1[0]);
  std::vector<uint16_t> shortV3(size, (uint64_t(1) << width) - 1);

  secSignedIntBatch int1(v1, partyId);
  secSignedIntBatch int2(v2, partyId);
  secUnsignedIntBatch int3(v3, partyId);

  testVectorEq(int1.openToParty(partyId).getValue(), v1);
  testVectorEq(int2.openToParty(partyId).getValue(), v2);
  testVectorEq(int3.openToParty(partyId).getValue(), v3);

  secSignedIntBatch int1Short(shortV1, partyId);
  secSignedIntBatch int2Short(shortV2, partyId);
  secUnsignedIntBatch int3Short(shortV3, partyId);

  testVectorEq(int1Short.openToParty(partyId).getValue(), v1);
  testVectorEq(int2Short.openToParty(partyId).getValue(), v2);
  testVectorEq(int3Short.openToParty(partyId).getValue(), v3);

  pubSignedIntBatch int4(v1);
  pubSignedIntBatch int5(v2);
  pubUnsignedIntBatch int6(v3);

  testVectorEq(int4.getValue(), v1);
  testVectorEq(int5.getValue(), v2);
  testVectorEq(int6.getValue(), v3);

  pubSignedIntBatch int4Short(v1);
  pubSignedIntBatch int5Short(v2);
  pubUnsignedIntBatch int6Short(v3);

  testVectorEq(int4Short.getValue(), v1);
  testVectorEq(int5Short.getValue(), v2);
  testVectorEq(int6Short.getValue(), v3);

  auto share1 = int1.extractIntShare();
  auto share2 = int3.extractIntShare();

  auto share3 = int1Short.extractIntShare();
  auto share4 = int3Short.extractIntShare();

  secSignedIntBatch::ExtractedInt share5(v1);
  secSignedIntBatch::ExtractedInt share6(v2);
  secUnsignedIntBatch::ExtractedInt share7(v3);

  testVectorEq(share5.getValue(), v1);
  testVectorEq(share6.getValue(), v2);
  testVectorEq(share7.getValue(), v3);

  secSignedIntBatch int7(std::move(share1));
  secUnsignedIntBatch int8(std::move(share2));

  secSignedIntBatch int7Short(std::move(share3));
  secUnsignedIntBatch int8Short(std::move(share4));

  secSignedIntBatch int9(std::move(share5));
  secSignedIntBatch int10(std::move(share6));
  secUnsignedIntBatch int11(std::move(share7));

  testVectorEq(int7.openToParty(partyId).getValue(), v1);
  testVectorEq(int8.openToParty(partyId).getValue(), v3);

  testVectorEq(int7Short.openToParty(partyId).getValue(), v1);
  testVectorEq(int8Short.openToParty(partyId).getValue(), v3);

  testVectorEq(int9.openToParty(partyId).getValue(), v1);
  testVectorEq(int10.openToParty(partyId).getValue(), v2);
  testVectorEq(int11.openToParty(partyId).getValue(), v3);
}

bool testSignedAdditionOverflow(int64_t a, int64_t b) {
  if ((a > 0) ^ (b > 0)) {
    if (a > 0) {
      // b can't be too large
      return b > std::numeric_limits<int64_t>().max() - a;
    } else {
      // b can't be too small
      return b < std::numeric_limits<int64_t>().min() - a;
    }
  } else {
    // if a and b are different sign
    return true;
  }
}

bool testUnsignedAdditionOverflow(uint64_t a, uint64_t b) {
  return b > std::numeric_limits<uint64_t>().max() - a;
}

TEST(IntTest, testAdd) {
  const int8_t width = 64;

  int64_t largestSigned = std::numeric_limits<int64_t>().max();
  int64_t smallestSigned = std::numeric_limits<int64_t>().min();
  uint64_t largestUnsigned = std::numeric_limits<uint64_t>().max();

  scheduler::SchedulerKeeper<0>::setScheduler(
      std::make_unique<scheduler::PlaintextScheduler>(
          scheduler::WireKeeper::createWithUnorderedMap()));
  using secSignedInt = Integer<Secret<Signed<width>>, 0>;
  using pubSignedInt = Integer<Public<Signed<width>>, 0>;
  using secUnsignedInt = Integer<Secret<Unsigned<width>>, 0>;
  using pubUnsignedInt = Integer<Public<Unsigned<width>>, 0>;

  int partyId = 2;

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<int64_t> dist1(smallestSigned, largestSigned);

  std::uniform_int_distribution<uint64_t> dist2(0, largestUnsigned);

  for (int i = 0; i < 100; i++) {
    int64_t v1;

    int64_t v2;
    do {
      v1 = dist1(e);
      v2 = dist1(e);
    } while (testSignedAdditionOverflow(v1, v2));

    /**
     * The block is to prevent the secure types from outliving the backend.
     * The test will fail if the backend (owned by global unique pointer) is not
     * explicitly deleted before the end of the test.
     * This is only an issue within the unit tests. We don't need to do this in
     * production code.
     */
    {
      secSignedInt int1(v1, partyId);
      secSignedInt int2(v2, partyId);

      auto r1 = int1 + int2;
      EXPECT_EQ(r1.openToParty(partyId).getValue(), v1 + v2);

      pubSignedInt int3(v1);
      pubSignedInt int4(v2);

      auto r2 = int3 + int4;
      EXPECT_EQ(r2.getValue(), v1 + v2);

      auto r3 = int1 + int4;
      EXPECT_EQ(r3.openToParty(partyId).getValue(), v1 + v2);
    }
    uint64_t v3;
    uint64_t v4;

    do {
      v3 = dist2(e);
      v4 = dist2(e);
    } while (testUnsignedAdditionOverflow(v3, v4));

    /**
     * The block is to prevent the secure types from outliving the backend.
     * The test will fail if the backend (owned by global unique pointer) is not
     * explicitly deleted before the end of the test.
     * This is only an issue within the unit tests. We don't need to do this in
     * production code.
     */
    {
      secUnsignedInt int1(v3, partyId);
      secUnsignedInt int2(v4, partyId);
      auto r1 = int1 + int2;
      EXPECT_EQ(r1.openToParty(partyId).getValue(), v3 + v4);

      pubUnsignedInt int3(v3);
      pubUnsignedInt int4(v4);

      auto r2 = int3 + int4;
      EXPECT_EQ(r2.getValue(), v3 + v4);

      auto r3 = int1 + int4;
      EXPECT_EQ(r3.openToParty(partyId).getValue(), v3 + v4);
    }
  }
}

template <typename T>
std::vector<T> addVector(
    const std::vector<T>& src1,
    const std::vector<T>& src2) {
  if (src1.size() != src2.size()) {
    throw std::runtime_error("Uneven size!");
  }
  if (src1.size() == 0) {
    return std::vector<T>();
  } else {
    std::vector<T> rst(src1.size());
    for (size_t i = 0; i < src1.size(); i++) {
      rst[i] = src1.at(i) + src2.at(i);
    }
    return rst;
  }
}

TEST(IntTest, testAddBatch) {
  const int8_t width = 64;

  int64_t largestSigned = std::numeric_limits<int64_t>().max();
  int64_t smallestSigned = std::numeric_limits<int64_t>().min();
  uint64_t largestUnsigned = std::numeric_limits<uint64_t>().max();

  scheduler::SchedulerKeeper<0>::setScheduler(
      std::make_unique<scheduler::PlaintextScheduler>(
          scheduler::WireKeeper::createWithUnorderedMap()));
  using secSignedIntBatch = Integer<Secret<Batch<Signed<width>>>, 0>;
  using pubSignedIntBatch = Integer<Public<Batch<Signed<width>>>, 0>;
  using secUnsignedIntBatch = Integer<Secret<Batch<Unsigned<width>>>, 0>;
  using pubUnsignedIntBatch = Integer<Public<Batch<Unsigned<width>>>, 0>;

  size_t batchSize = 17;

  int partyId = 2;

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<int64_t> dist1(smallestSigned, largestSigned);

  std::uniform_int_distribution<uint64_t> dist2(0, largestUnsigned);

  for (int i = 0; i < 20; i++) {
    std::vector<int64_t> v1(batchSize);
    std::vector<int64_t> v2(batchSize);

    for (size_t j = 0; j < batchSize; j++) {
      int64_t tmp1;
      int64_t tmp2;
      do {
        tmp1 = dist1(e);
        tmp2 = dist1(e);
      } while (testSignedAdditionOverflow(tmp1, tmp2));
      v1[j] = tmp1;
      v2[j] = tmp2;
    }

    /**
     * The block is to prevent the secure types from outliving the backend.
     * The test will fail if the backend (owned by global unique pointer) is not
     * explicitly deleted before the end of the test.
     * This is only an issue within the unit tests. We don't need to do this in
     * production code.
     */
    {
      secSignedIntBatch int1(v1, partyId);
      secSignedIntBatch int2(v2, partyId);

      auto r1 = int1 + int2;
      testVectorEq(r1.openToParty(partyId).getValue(), addVector(v1, v2));

      pubSignedIntBatch int3(v1);
      pubSignedIntBatch int4(v2);

      auto r2 = int3 + int4;
      testVectorEq(r2.getValue(), addVector(v1, v2));

      auto r3 = int1 + int4;
      testVectorEq(r3.openToParty(partyId).getValue(), addVector(v1, v2));
    }

    std::vector<uint64_t> v3(batchSize);
    std::vector<uint64_t> v4(batchSize);

    for (size_t j = 0; j < batchSize; j++) {
      uint64_t tmp1;
      uint64_t tmp2;
      do {
        tmp1 = dist2(e);
        tmp2 = dist2(e);
      } while (testUnsignedAdditionOverflow(tmp1, tmp2));
      v3[j] = tmp1;
      v4[j] = tmp2;
    }

    /**
     * The block is to prevent the secure types from outliving the backend.
     * The test will fail if the backend (owned by global unique pointer) is not
     * explicitly deleted before the end of the test.
     * This is only an issue within the unit tests. We don't need to do this in
     * production code.
     */
    {
      secUnsignedIntBatch int1(v3, partyId);
      secUnsignedIntBatch int2(v4, partyId);
      auto r1 = int1 + int2;
      testVectorEq(r1.openToParty(partyId).getValue(), addVector(v3, v4));

      pubUnsignedIntBatch int3(v3);
      pubUnsignedIntBatch int4(v4);

      auto r2 = int3 + int4;
      testVectorEq(r2.getValue(), addVector(v3, v4));

      auto r3 = int1 + int4;
      testVectorEq(r3.openToParty(partyId).getValue(), addVector(v3, v4));
    }
  }
}

TEST(IntTest, testSubtract) {
  const int8_t width = 64;

  int64_t largestSigned = std::numeric_limits<int64_t>().max();
  int64_t smallestSigned = std::numeric_limits<int64_t>().min();
  uint64_t largestUnsigned = std::numeric_limits<uint64_t>().max();

  scheduler::SchedulerKeeper<0>::setScheduler(
      std::make_unique<scheduler::PlaintextScheduler>(
          scheduler::WireKeeper::createWithUnorderedMap()));
  using secSignedInt = Integer<Secret<Signed<width>>, 0>;
  using pubSignedInt = Integer<Public<Signed<width>>, 0>;
  using secUnsignedInt = Integer<Secret<Unsigned<width>>, 0>;
  using pubUnsignedInt = Integer<Public<Unsigned<width>>, 0>;

  int partyId = 2;

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<int64_t> dist1(smallestSigned, largestSigned);

  std::uniform_int_distribution<uint64_t> dist2(0, largestUnsigned);

  for (int i = 0; i < 100; i++) {
    int64_t v1;

    int64_t v2;
    do {
      v1 = dist1(e);
      v2 = dist1(e);
    } while (testSignedAdditionOverflow(v1, -v2));

    /**
     * The block is to prevent the secure types from outliving the backend.
     * The test will fail if the backend (owned by global unique pointer) is not
     * explicitly deleted before the end of the test.
     * This is only an issue within the unit tests. We don't need to do this in
     * production code.
     */
    {
      secSignedInt int1(v1, partyId);
      secSignedInt int2(v2, partyId);

      auto r1 = int1 - int2;
      EXPECT_EQ(r1.openToParty(partyId).getValue(), v1 - v2);

      pubSignedInt int3(v1);
      pubSignedInt int4(v2);

      auto r2 = int3 - int4;
      EXPECT_EQ(r2.getValue(), v1 - v2);

      auto r3 = int1 - int4;
      EXPECT_EQ(r3.openToParty(partyId).getValue(), v1 - v2);
    }
    uint64_t v3;
    uint64_t v4;

    do {
      v3 = dist2(e);
      v4 = dist2(e);
    } while (testUnsignedAdditionOverflow(v3, -v4));

    /**
     * The block is to prevent the secure types from outliving the backend.
     * The test will fail if the backend (owned by global unique pointer) is not
     * explicitly deleted before the end of the test.
     * This is only an issue within the unit tests. We don't need to do this in
     * production code.
     */
    {
      secUnsignedInt int1(v3, partyId);
      secUnsignedInt int2(v4, partyId);
      auto r1 = int1 - int2;
      EXPECT_EQ(r1.openToParty(partyId).getValue(), v3 - v4);

      pubUnsignedInt int3(v3);
      pubUnsignedInt int4(v4);

      auto r2 = int3 - int4;
      EXPECT_EQ(r2.getValue(), v3 - v4);

      auto r3 = int1 - int4;
      EXPECT_EQ(r3.openToParty(partyId).getValue(), v3 - v4);
    }
  }
}

template <typename T>
std::vector<T> subtractVector(
    const std::vector<T>& src1,
    const std::vector<T>& src2) {
  if (src1.size() != src2.size()) {
    throw std::runtime_error("Uneven size!");
  }
  if (src1.size() == 0) {
    return std::vector<T>();
  } else {
    std::vector<T> rst(src1.size());
    for (size_t i = 0; i < src1.size(); i++) {
      rst[i] = src1.at(i) - src2.at(i);
    }
    return rst;
  }
}

TEST(IntTest, testSubtractBatch) {
  const int8_t width = 64;

  int64_t largestSigned = std::numeric_limits<int64_t>().max();
  int64_t smallestSigned = std::numeric_limits<int64_t>().min();
  uint64_t largestUnsigned = std::numeric_limits<uint64_t>().max();

  scheduler::SchedulerKeeper<0>::setScheduler(
      std::make_unique<scheduler::PlaintextScheduler>(
          scheduler::WireKeeper::createWithUnorderedMap()));
  using secSignedIntBatch = Integer<Secret<Batch<Signed<width>>>, 0>;
  using pubSignedIntBatch = Integer<Public<Batch<Signed<width>>>, 0>;
  using secUnsignedIntBatch = Integer<Secret<Batch<Unsigned<width>>>, 0>;
  using pubUnsignedIntBatch = Integer<Public<Batch<Unsigned<width>>>, 0>;

  size_t batchSize = 17;

  int partyId = 2;

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<int64_t> dist1(smallestSigned, largestSigned);

  std::uniform_int_distribution<uint64_t> dist2(0, largestUnsigned);

  for (int i = 0; i < 20; i++) {
    std::vector<int64_t> v1(batchSize);
    std::vector<int64_t> v2(batchSize);

    for (size_t j = 0; j < batchSize; j++) {
      int64_t tmp1;
      int64_t tmp2;
      do {
        tmp1 = dist1(e);
        tmp2 = dist1(e);
      } while (testSignedAdditionOverflow(tmp1, -tmp2));
      v1[j] = tmp1;
      v2[j] = tmp2;
    }

    /**
     * The block is to prevent the secure types from outliving the backend.
     * The test will fail if the backend (owned by global unique pointer) is not
     * explicitly deleted before the end of the test.
     * This is only an issue within the unit tests. We don't need to do this in
     * production code.
     */
    {
      secSignedIntBatch int1(v1, partyId);
      secSignedIntBatch int2(v2, partyId);

      auto r1 = int1 - int2;
      testVectorEq(r1.openToParty(partyId).getValue(), subtractVector(v1, v2));

      pubSignedIntBatch int3(v1);
      pubSignedIntBatch int4(v2);

      auto r2 = int3 - int4;
      testVectorEq(r2.getValue(), subtractVector(v1, v2));

      auto r3 = int1 - int4;
      testVectorEq(r3.openToParty(partyId).getValue(), subtractVector(v1, v2));
    }

    std::vector<uint64_t> v3(batchSize);
    std::vector<uint64_t> v4(batchSize);

    for (size_t j = 0; j < batchSize; j++) {
      uint64_t tmp1;
      uint64_t tmp2;
      do {
        tmp1 = dist2(e);
        tmp2 = dist2(e);
      } while (testUnsignedAdditionOverflow(tmp1, -tmp2));
      v3[j] = tmp1;
      v4[j] = tmp2;
    }

    /**
     * The block is to prevent the secure types from outliving the backend.
     * The test will fail if the backend (owned by global unique pointer) is not
     * explicitly deleted before the end of the test.
     * This is only an issue within the unit tests. We don't need to do this in
     * production code.
     */
    {
      secUnsignedIntBatch int1(v3, partyId);
      secUnsignedIntBatch int2(v4, partyId);
      auto r1 = int1 - int2;
      testVectorEq(r1.openToParty(partyId).getValue(), subtractVector(v3, v4));

      pubUnsignedIntBatch int3(v3);
      pubUnsignedIntBatch int4(v4);

      auto r2 = int3 - int4;
      testVectorEq(r2.getValue(), subtractVector(v3, v4));

      auto r3 = int1 - int4;
      testVectorEq(r3.openToParty(partyId).getValue(), subtractVector(v3, v4));
    }
  }
}

TEST(IntTest, testComparison) {
  const int8_t width = 64;

  int64_t largestSigned = std::numeric_limits<int64_t>().max();
  int64_t smallestSigned = std::numeric_limits<int64_t>().min();
  uint64_t largestUnsigned = std::numeric_limits<uint64_t>().max();

  scheduler::SchedulerKeeper<0>::setScheduler(
      std::make_unique<scheduler::PlaintextScheduler>(
          scheduler::WireKeeper::createWithUnorderedMap()));
  using secSignedInt = Integer<Secret<Signed<width>>, 0>;
  using pubSignedInt = Integer<Public<Signed<width>>, 0>;
  using secUnsignedInt = Integer<Secret<Unsigned<width>>, 0>;
  using pubUnsignedInt = Integer<Public<Unsigned<width>>, 0>;

  int partyId = 2;

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<int64_t> dist1(smallestSigned, largestSigned);

  std::uniform_int_distribution<uint64_t> dist2(0, largestUnsigned);

  for (int i = 0; i < 100; i++) {
    int64_t v1 = dist1(e);
    int64_t v2 = dist1(e);

    secSignedInt int1(v1, partyId);
    secSignedInt int2(v2, partyId);
    pubSignedInt int3(v1);
    pubSignedInt int4(v2);

    auto r1 = int1 < int2;
    auto r2 = int1 < int4;
    auto r3 = int3 < int4;
    auto r4 = int1 <= int2;
    auto r5 = int1 <= int4;
    auto r6 = int3 <= int4;
    auto r7 = int1 <= int1;
    auto r8 = int3 <= int3;

    auto r9 = int1 > int2;
    auto r10 = int1 > int4;
    auto r11 = int3 > int4;
    auto r12 = int1 >= int2;
    auto r13 = int1 >= int4;
    auto r14 = int3 >= int4;
    auto r15 = int1 >= int1;
    auto r16 = int3 >= int3;

    EXPECT_EQ(r1.openToParty(partyId).getValue(), v1 < v2);
    EXPECT_EQ(r2.openToParty(partyId).getValue(), v1 < v2);
    EXPECT_EQ(r3.getValue(), v1 < v2);
    EXPECT_EQ(r4.openToParty(partyId).getValue(), v1 <= v2);
    EXPECT_EQ(r5.openToParty(partyId).getValue(), v1 <= v2);
    EXPECT_EQ(r6.getValue(), v1 <= v2);
    EXPECT_TRUE(r7.openToParty(partyId).getValue());
    EXPECT_TRUE(r8.getValue());

    EXPECT_EQ(r9.openToParty(partyId).getValue(), v1 > v2);
    EXPECT_EQ(r10.openToParty(partyId).getValue(), v1 > v2);
    EXPECT_EQ(r11.getValue(), v1 > v2);
    EXPECT_EQ(r12.openToParty(partyId).getValue(), v1 >= v2);
    EXPECT_EQ(r13.openToParty(partyId).getValue(), v1 >= v2);
    EXPECT_EQ(r14.getValue(), v1 >= v2);
    EXPECT_TRUE(r15.openToParty(partyId).getValue());
    EXPECT_TRUE(r16.getValue());
  }

  for (int i = 0; i < 100; i++) {
    uint64_t v3 = dist2(e);
    uint64_t v4 = dist2(e);

    secUnsignedInt int1(v3, partyId);
    secUnsignedInt int2(v4, partyId);
    pubUnsignedInt int3(v3);
    pubUnsignedInt int4(v4);

    auto r1 = int1 < int2;
    auto r2 = int1 < int4;
    auto r3 = int3 < int4;
    auto r4 = int1 <= int2;
    auto r5 = int1 <= int4;
    auto r6 = int3 <= int4;
    auto r7 = int1 <= int1;
    auto r8 = int3 <= int3;

    auto r9 = int1 > int2;
    auto r10 = int1 > int4;
    auto r11 = int3 > int4;
    auto r12 = int1 >= int2;
    auto r13 = int1 >= int4;
    auto r14 = int3 >= int4;
    auto r15 = int1 >= int1;
    auto r16 = int3 >= int3;

    EXPECT_EQ(r1.openToParty(partyId).getValue(), v3 < v4);
    EXPECT_EQ(r2.openToParty(partyId).getValue(), v3 < v4);
    EXPECT_EQ(r3.getValue(), v3 < v4);
    EXPECT_EQ(r4.openToParty(partyId).getValue(), v3 <= v4);
    EXPECT_EQ(r5.openToParty(partyId).getValue(), v3 <= v4);
    EXPECT_EQ(r6.getValue(), v3 <= v4);
    EXPECT_TRUE(r7.openToParty(partyId).getValue());
    EXPECT_TRUE(r8.getValue());

    EXPECT_EQ(r9.openToParty(partyId).getValue(), v3 > v4);
    EXPECT_EQ(r10.openToParty(partyId).getValue(), v3 > v4);
    EXPECT_EQ(r11.getValue(), v3 > v4);
    EXPECT_EQ(r12.openToParty(partyId).getValue(), v3 >= v4);
    EXPECT_EQ(r13.openToParty(partyId).getValue(), v3 >= v4);
    EXPECT_EQ(r14.getValue(), v3 >= v4);
    EXPECT_TRUE(r15.openToParty(partyId).getValue());
    EXPECT_TRUE(r16.getValue());
  }
}

template <typename T, typename F>
std::vector<bool> compareVector(
    const std::vector<T>& src1,
    const std::vector<T>& src2,
    F predicater) {
  if (src1.size() != src2.size()) {
    throw std::runtime_error("Uneven size!");
  }
  if (src1.size() == 0) {
    return std::vector<bool>();
  } else {
    std::vector<bool> rst(src1.size());
    for (size_t i = 0; i < src1.size(); i++) {
      rst[i] = predicater(src1.at(i), src2.at(i));
    }
    return rst;
  }
}

TEST(IntTest, testComparisonBatch) {
  const int8_t width = 64;

  int64_t largestSigned = std::numeric_limits<int64_t>().max();
  int64_t smallestSigned = std::numeric_limits<int64_t>().min();
  uint64_t largestUnsigned = std::numeric_limits<uint64_t>().max();

  scheduler::SchedulerKeeper<0>::setScheduler(
      std::make_unique<scheduler::PlaintextScheduler>(
          scheduler::WireKeeper::createWithUnorderedMap()));
  using secSignedIntBatch = Integer<Secret<Batch<Signed<width>>>, 0>;
  using pubSignedIntBatch = Integer<Public<Batch<Signed<width>>>, 0>;
  using secUnsignedIntBatch = Integer<Secret<Batch<Unsigned<width>>>, 0>;
  using pubUnsignedIntBatch = Integer<Public<Batch<Unsigned<width>>>, 0>;

  size_t batchSize = 21;

  int partyId = 2;

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<int64_t> dist1(smallestSigned, largestSigned);

  std::uniform_int_distribution<uint64_t> dist2(0, largestUnsigned);

  for (int i = 0; i < 100; i++) {
    std::vector<int64_t> v1(batchSize);
    std::vector<int64_t> v2(batchSize);
    for (size_t j = 0; j < batchSize; j++) {
      v1[j] = dist1(e);
      v2[j] = dist1(e);
    }

    secSignedIntBatch int1(v1, partyId);
    secSignedIntBatch int2(v2, partyId);
    pubSignedIntBatch int3(v1);
    pubSignedIntBatch int4(v2);

    auto r1 = int1 < int2;
    auto r2 = int1 < int4;
    auto r3 = int3 < int4;
    auto r4 = int1 <= int2;
    auto r5 = int1 <= int4;
    auto r6 = int3 <= int4;
    auto r7 = int1 <= int1;
    auto r8 = int3 <= int3;

    auto r9 = int1 > int2;
    auto r10 = int1 > int4;
    auto r11 = int3 > int4;
    auto r12 = int1 >= int2;
    auto r13 = int1 >= int4;
    auto r14 = int3 >= int4;
    auto r15 = int1 >= int1;
    auto r16 = int3 >= int3;

    auto lt = [](auto x, auto y) { return x < y; };
    auto le = [](auto x, auto y) { return x <= y; };
    testVectorEq(r1.openToParty(partyId).getValue(), compareVector(v1, v2, lt));
    testVectorEq(r2.openToParty(partyId).getValue(), compareVector(v1, v2, lt));
    testVectorEq(r3.getValue(), compareVector(v1, v2, lt));

    testVectorEq(r4.openToParty(partyId).getValue(), compareVector(v1, v2, le));
    testVectorEq(r5.openToParty(partyId).getValue(), compareVector(v1, v2, le));
    testVectorEq(r6.getValue(), compareVector(v1, v2, le));
    testVectorEq(
        r7.openToParty(partyId).getValue(), std::vector<bool>(batchSize, true));
    testVectorEq(r8.getValue(), std::vector<bool>(batchSize, true));

    auto gt = [](auto x, auto y) { return x > y; };
    auto ge = [](auto x, auto y) { return x >= y; };
    testVectorEq(r9.openToParty(partyId).getValue(), compareVector(v1, v2, gt));
    testVectorEq(
        r10.openToParty(partyId).getValue(), compareVector(v1, v2, gt));
    testVectorEq(r11.getValue(), compareVector(v1, v2, gt));

    testVectorEq(
        r12.openToParty(partyId).getValue(), compareVector(v1, v2, ge));
    testVectorEq(
        r13.openToParty(partyId).getValue(), compareVector(v1, v2, ge));
    testVectorEq(r14.getValue(), compareVector(v1, v2, ge));
    testVectorEq(
        r15.openToParty(partyId).getValue(),
        std::vector<bool>(batchSize, true));
    testVectorEq(r16.getValue(), std::vector<bool>(batchSize, true));
  }

  for (int i = 0; i < 20; i++) {
    std::vector<uint64_t> v3(batchSize);
    std::vector<uint64_t> v4(batchSize);
    for (size_t j = 0; j < batchSize; j++) {
      v3[j] = dist2(e);
      v4[j] = dist2(e);
    }

    secUnsignedIntBatch int1(v3, partyId);
    secUnsignedIntBatch int2(v4, partyId);
    pubUnsignedIntBatch int3(v3);
    pubUnsignedIntBatch int4(v4);

    auto r1 = int1 < int2;
    auto r2 = int1 < int4;
    auto r3 = int3 < int4;
    auto r4 = int1 <= int2;
    auto r5 = int1 <= int4;
    auto r6 = int3 <= int4;
    auto r7 = int1 <= int1;
    auto r8 = int3 <= int3;

    auto r9 = int1 > int2;
    auto r10 = int1 > int4;
    auto r11 = int3 > int4;
    auto r12 = int1 >= int2;
    auto r13 = int1 >= int4;
    auto r14 = int3 >= int4;
    auto r15 = int1 >= int1;
    auto r16 = int3 >= int3;

    auto lt = [](auto x, auto y) { return x < y; };
    auto le = [](auto x, auto y) { return x <= y; };

    testVectorEq(r1.openToParty(partyId).getValue(), compareVector(v3, v4, lt));
    testVectorEq(r2.openToParty(partyId).getValue(), compareVector(v3, v4, lt));
    testVectorEq(r3.getValue(), compareVector(v3, v4, [](auto x, auto y) {
                   return x < y;
                 }));
    testVectorEq(r4.openToParty(partyId).getValue(), compareVector(v3, v4, le));
    testVectorEq(r5.openToParty(partyId).getValue(), compareVector(v3, v4, le));
    testVectorEq(r6.getValue(), compareVector(v3, v4, [](auto x, auto y) {
                   return x <= y;
                 }));
    testVectorEq(
        r7.openToParty(partyId).getValue(), std::vector<bool>(batchSize, true));
    testVectorEq(r8.getValue(), std::vector<bool>(batchSize, true));

    auto gt = [](auto x, auto y) { return x > y; };
    auto ge = [](auto x, auto y) { return x >= y; };

    testVectorEq(r9.openToParty(partyId).getValue(), compareVector(v3, v4, gt));
    testVectorEq(
        r10.openToParty(partyId).getValue(), compareVector(v3, v4, gt));
    testVectorEq(r11.getValue(), compareVector(v3, v4, gt));

    testVectorEq(
        r12.openToParty(partyId).getValue(), compareVector(v3, v4, ge));
    testVectorEq(
        r13.openToParty(partyId).getValue(), compareVector(v3, v4, ge));
    testVectorEq(r14.getValue(), compareVector(v3, v4, ge));
    testVectorEq(
        r15.openToParty(partyId).getValue(),
        std::vector<bool>(batchSize, true));
    testVectorEq(r16.getValue(), std::vector<bool>(batchSize, true));
  }
}

TEST(IntTest, testEqual) {
  const int8_t width = 64;

  int64_t largestSigned = std::numeric_limits<int64_t>().max();
  int64_t smallestSigned = std::numeric_limits<int64_t>().min();
  uint64_t largestUnsigned = std::numeric_limits<uint64_t>().max();

  scheduler::SchedulerKeeper<0>::setScheduler(
      std::make_unique<scheduler::PlaintextScheduler>(
          scheduler::WireKeeper::createWithUnorderedMap()));
  using secSignedInt = Integer<Secret<Signed<width>>, 0>;
  using pubSignedInt = Integer<Public<Signed<width>>, 0>;
  using secUnsignedInt = Integer<Secret<Unsigned<width>>, 0>;
  using pubUnsignedInt = Integer<Public<Unsigned<width>>, 0>;

  int partyId = 2;

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<int64_t> dist1(smallestSigned, largestSigned);

  std::uniform_int_distribution<uint64_t> dist2(0, largestUnsigned);

  for (int i = 0; i < 100; i++) {
    int64_t v1 = dist1(e);
    int64_t v2 = dist1(e);

    secSignedInt int1(v1, partyId);
    secSignedInt int2(v2, partyId);
    pubSignedInt int3(v1);
    pubSignedInt int4(v2);

    auto r1 = int1 == int2;
    auto r2 = int1 == int4;
    auto r3 = int3 == int4;

    EXPECT_EQ(r1.openToParty(partyId).getValue(), v1 == v2);
    EXPECT_EQ(r2.openToParty(partyId).getValue(), v1 == v2);
    EXPECT_EQ(r3.getValue(), v1 == v2);
  }

  for (int i = 0; i < 100; i++) {
    uint64_t v3 = dist2(e);
    uint64_t v4 = dist2(e);

    secUnsignedInt int1(v3, partyId);
    secUnsignedInt int2(v4, partyId);
    pubUnsignedInt int3(v3);
    pubUnsignedInt int4(v4);

    auto r1 = int1 == int2;
    auto r2 = int1 == int4;
    auto r3 = int3 == int4;

    EXPECT_EQ(r1.openToParty(partyId).getValue(), v3 == v4);
    EXPECT_EQ(r2.openToParty(partyId).getValue(), v3 == v4);
    EXPECT_EQ(r3.getValue(), v3 == v4);
  }
}

template <typename T>
std::vector<bool> equalVector(
    const std::vector<T>& src1,
    const std::vector<T>& src2) {
  if (src1.size() != src2.size()) {
    throw std::runtime_error("Uneven size!");
  }
  if (src1.size() == 0) {
    return std::vector<bool>();
  } else {
    std::vector<bool> rst(src1.size());
    for (size_t i = 0; i < src1.size(); i++) {
      rst[i] = src1.at(i) == src2.at(i);
    }
    return rst;
  }
}

TEST(IntTest, testEqualBatch) {
  const int8_t width = 64;

  int64_t largestSigned = std::numeric_limits<int64_t>().max();
  int64_t smallestSigned = std::numeric_limits<int64_t>().min();
  uint64_t largestUnsigned = std::numeric_limits<uint64_t>().max();

  scheduler::SchedulerKeeper<0>::setScheduler(
      std::make_unique<scheduler::PlaintextScheduler>(
          scheduler::WireKeeper::createWithUnorderedMap()));
  using secSignedIntBatch = Integer<Secret<Batch<Signed<width>>>, 0>;
  using pubSignedIntBatch = Integer<Public<Batch<Signed<width>>>, 0>;
  using secUnsignedIntBatch = Integer<Secret<Batch<Unsigned<width>>>, 0>;
  using pubUnsignedIntBatch = Integer<Public<Batch<Unsigned<width>>>, 0>;

  size_t batchSize = 21;

  int partyId = 2;

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<int64_t> dist1(smallestSigned, largestSigned);

  std::uniform_int_distribution<uint64_t> dist2(0, largestUnsigned);

  for (int i = 0; i < 100; i++) {
    std::vector<int64_t> v1(batchSize);
    std::vector<int64_t> v2(batchSize);
    for (size_t j = 0; j < batchSize; j++) {
      v1[j] = dist1(e);
      v2[j] = dist1(e);
    }

    secSignedIntBatch int1(v1, partyId);
    secSignedIntBatch int2(v2, partyId);
    pubSignedIntBatch int3(v1);
    pubSignedIntBatch int4(v2);

    auto r1 = int1 == int2;
    auto r2 = int1 == int4;
    auto r3 = int3 == int4;

    testVectorEq(r1.openToParty(partyId).getValue(), equalVector(v1, v2));
    testVectorEq(r2.openToParty(partyId).getValue(), equalVector(v1, v2));
    testVectorEq(r3.getValue(), equalVector(v1, v2));
  }

  for (int i = 0; i < 20; i++) {
    std::vector<uint64_t> v3(batchSize);
    std::vector<uint64_t> v4(batchSize);
    for (size_t j = 0; j < batchSize; j++) {
      v3[j] = dist2(e);
      v4[j] = dist2(e);
    }

    secUnsignedIntBatch int1(v3, partyId);
    secUnsignedIntBatch int2(v4, partyId);
    pubUnsignedIntBatch int3(v3);
    pubUnsignedIntBatch int4(v4);

    auto r1 = int1 == int2;
    auto r2 = int1 == int4;
    auto r3 = int3 == int4;

    testVectorEq(r1.openToParty(partyId).getValue(), equalVector(v3, v4));
    testVectorEq(r2.openToParty(partyId).getValue(), equalVector(v3, v4));
    testVectorEq(r3.getValue(), equalVector(v3, v4));
  }
}

TEST(IntTest, testMux) {
  const int8_t width = 64;

  int64_t largestSigned = std::numeric_limits<int64_t>().max();
  int64_t smallestSigned = std::numeric_limits<int64_t>().min();
  uint64_t largestUnsigned = std::numeric_limits<uint64_t>().max();

  scheduler::SchedulerKeeper<0>::setScheduler(
      std::make_unique<scheduler::PlaintextScheduler>(
          scheduler::WireKeeper::createWithUnorderedMap()));
  using secSignedInt = Integer<Secret<Signed<width>>, 0>;
  using pubSignedInt = Integer<Public<Signed<width>>, 0>;
  using secUnsignedInt = Integer<Secret<Unsigned<width>>, 0>;
  using pubUnsignedInt = Integer<Public<Unsigned<width>>, 0>;

  int partyId = 2;

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<int64_t> dist1(smallestSigned, largestSigned);

  std::uniform_int_distribution<uint64_t> dist2(0, largestUnsigned);

  std::uniform_int_distribution<uint8_t> dist3(0, 1);

  for (int i = 0; i < 100; i++) {
    int64_t v1 = dist1(e);
    int64_t v2 = dist1(e);
    bool choice = dist3(e);

    secSignedInt int1(v1, partyId);
    secSignedInt int2(v2, partyId);
    pubSignedInt int3(v1);
    pubSignedInt int4(v2);
    Bit<true, 0> secChoice(choice, partyId);
    Bit<false, 0> pubChoice(choice);

    auto r1 = int1.mux(secChoice, int2);
    auto r2 = int1.mux(pubChoice, int2);
    auto r3 = int1.mux(secChoice, int4);
    auto r4 = int1.mux(pubChoice, int4);

    auto r5 = int3.mux(secChoice, int2);
    auto r6 = int3.mux(pubChoice, int2);
    auto r7 = int3.mux(secChoice, int4);
    auto r8 = int3.mux(pubChoice, int4);

    auto expectedValue = choice ? v2 : v1;

    EXPECT_EQ(r1.openToParty(partyId).getValue(), expectedValue);
    EXPECT_EQ(r2.openToParty(partyId).getValue(), expectedValue);
    EXPECT_EQ(r3.openToParty(partyId).getValue(), expectedValue);
    EXPECT_EQ(r4.openToParty(partyId).getValue(), expectedValue);
    EXPECT_EQ(r5.openToParty(partyId).getValue(), expectedValue);
    EXPECT_EQ(r6.openToParty(partyId).getValue(), expectedValue);
    EXPECT_EQ(r7.openToParty(partyId).getValue(), expectedValue);
    EXPECT_EQ(r8.getValue(), expectedValue);
  }

  for (int i = 0; i < 100; i++) {
    uint64_t v1 = dist2(e);
    uint64_t v2 = dist2(e);

    bool choice = dist3(e);

    secUnsignedInt int1(v1, partyId);
    secUnsignedInt int2(v2, partyId);
    pubUnsignedInt int3(v1);
    pubUnsignedInt int4(v2);
    Bit<true, 0> secChoice(choice, partyId);
    Bit<false, 0> pubChoice(choice);

    auto r1 = int1.mux(secChoice, int2);
    auto r2 = int1.mux(pubChoice, int2);
    auto r3 = int1.mux(secChoice, int4);
    auto r4 = int1.mux(pubChoice, int4);

    auto r5 = int3.mux(secChoice, int2);
    auto r6 = int3.mux(pubChoice, int2);
    auto r7 = int3.mux(secChoice, int4);
    auto r8 = int3.mux(pubChoice, int4);

    auto expectedValue = choice ? v2 : v1;

    EXPECT_EQ(r1.openToParty(partyId).getValue(), expectedValue);
    EXPECT_EQ(r2.openToParty(partyId).getValue(), expectedValue);
    EXPECT_EQ(r3.openToParty(partyId).getValue(), expectedValue);
    EXPECT_EQ(r4.openToParty(partyId).getValue(), expectedValue);
    EXPECT_EQ(r5.openToParty(partyId).getValue(), expectedValue);
    EXPECT_EQ(r6.openToParty(partyId).getValue(), expectedValue);
    EXPECT_EQ(r7.openToParty(partyId).getValue(), expectedValue);
    EXPECT_EQ(r8.getValue(), expectedValue);
  }
}

TEST(IntTest, testMuxBatch) {
  const int8_t width = 64;

  int64_t largestSigned = std::numeric_limits<int64_t>().max();
  int64_t smallestSigned = std::numeric_limits<int64_t>().min();
  uint64_t largestUnsigned = std::numeric_limits<uint64_t>().max();

  scheduler::SchedulerKeeper<0>::setScheduler(
      std::make_unique<scheduler::PlaintextScheduler>(
          scheduler::WireKeeper::createWithUnorderedMap()));
  using secSignedIntBatch = Integer<Secret<Batch<Signed<width>>>, 0>;
  using pubSignedIntBatch = Integer<Public<Batch<Signed<width>>>, 0>;
  using secUnsignedIntBatch = Integer<Secret<Batch<Unsigned<width>>>, 0>;
  using pubUnsignedIntBatch = Integer<Public<Batch<Unsigned<width>>>, 0>;

  size_t batchSize = 9;

  int partyId = 2;

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<int64_t> dist1(smallestSigned, largestSigned);

  std::uniform_int_distribution<uint64_t> dist2(0, largestUnsigned);

  std::uniform_int_distribution<uint8_t> dist3(0, 1);

  for (int i = 0; i < 100; i++) {
    std::vector<int64_t> v1(batchSize);
    std::vector<int64_t> v2(batchSize);
    std::vector<bool> choice(batchSize);
    for (size_t j = 0; j < batchSize; j++) {
      v1[j] = dist1(e);
      v2[j] = dist1(e);
      choice[j] = dist3(e);
    }

    secSignedIntBatch int1(v1, partyId);
    secSignedIntBatch int2(v2, partyId);
    pubSignedIntBatch int3(v1);
    pubSignedIntBatch int4(v2);
    Bit<true, 0, true> secChoice(choice, partyId);
    Bit<false, 0, true> pubChoice(choice);

    auto r1 = int1.mux(secChoice, int2);
    auto r2 = int1.mux(pubChoice, int2);
    auto r3 = int1.mux(secChoice, int4);
    auto r4 = int1.mux(pubChoice, int4);

    auto r5 = int3.mux(secChoice, int2);
    auto r6 = int3.mux(pubChoice, int2);
    auto r7 = int3.mux(secChoice, int4);
    auto r8 = int3.mux(pubChoice, int4);

    std::vector<int64_t> expectedValue(batchSize);
    for (size_t j = 0; j < batchSize; j++) {
      expectedValue[j] = choice[j] ? v2[j] : v1[j];
    }

    testVectorEq(r1.openToParty(partyId).getValue(), expectedValue);
    testVectorEq(r2.openToParty(partyId).getValue(), expectedValue);
    testVectorEq(r3.openToParty(partyId).getValue(), expectedValue);
    testVectorEq(r4.openToParty(partyId).getValue(), expectedValue);
    testVectorEq(r5.openToParty(partyId).getValue(), expectedValue);
    testVectorEq(r6.openToParty(partyId).getValue(), expectedValue);
    testVectorEq(r7.openToParty(partyId).getValue(), expectedValue);
    testVectorEq(r8.getValue(), expectedValue);
  }

  for (int i = 0; i < 100; i++) {
    std::vector<uint64_t> v1(batchSize);
    std::vector<uint64_t> v2(batchSize);
    std::vector<bool> choice(batchSize);
    for (size_t j = 0; j < batchSize; j++) {
      v1[j] = dist2(e);
      v2[j] = dist2(e);
      choice[j] = dist3(e);
    }

    secUnsignedIntBatch int1(v1, partyId);
    secUnsignedIntBatch int2(v2, partyId);
    pubUnsignedIntBatch int3(v1);
    pubUnsignedIntBatch int4(v2);
    Bit<true, 0, true> secChoice(choice, partyId);
    Bit<false, 0, true> pubChoice(choice);

    auto r1 = int1.mux(secChoice, int2);
    auto r2 = int1.mux(pubChoice, int2);
    auto r3 = int1.mux(secChoice, int4);
    auto r4 = int1.mux(pubChoice, int4);

    auto r5 = int3.mux(secChoice, int2);
    auto r6 = int3.mux(pubChoice, int2);
    auto r7 = int3.mux(secChoice, int4);
    auto r8 = int3.mux(pubChoice, int4);

    std::vector<uint64_t> expectedValue(batchSize);
    for (size_t j = 0; j < batchSize; j++) {
      expectedValue[j] = choice[j] ? v2[j] : v1[j];
    }

    testVectorEq(r1.openToParty(partyId).getValue(), expectedValue);
    testVectorEq(r2.openToParty(partyId).getValue(), expectedValue);
    testVectorEq(r3.openToParty(partyId).getValue(), expectedValue);
    testVectorEq(r4.openToParty(partyId).getValue(), expectedValue);
    testVectorEq(r5.openToParty(partyId).getValue(), expectedValue);
    testVectorEq(r6.openToParty(partyId).getValue(), expectedValue);
    testVectorEq(r7.openToParty(partyId).getValue(), expectedValue);
    testVectorEq(r8.getValue(), expectedValue);
  }
}

TEST(IntTest, testSubscript) {
  const int8_t width = 64;

  int64_t largestSigned = std::numeric_limits<int64_t>().max();
  int64_t smallestSigned = std::numeric_limits<int64_t>().min();
  uint64_t largestUnsigned = std::numeric_limits<uint64_t>().max();

  scheduler::SchedulerKeeper<0>::setScheduler(
      std::make_unique<scheduler::PlaintextScheduler>(
          scheduler::WireKeeper::createWithUnorderedMap()));
  using secSignedInt = Integer<Secret<Signed<width>>, 0>;
  using pubSignedInt = Integer<Public<Signed<width>>, 0>;
  using secUnsignedInt = Integer<Secret<Unsigned<width>>, 0>;
  using pubUnsignedInt = Integer<Public<Unsigned<width>>, 0>;

  int partyId = 2;

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<int64_t> dist1(smallestSigned, largestSigned);

  std::uniform_int_distribution<uint64_t> dist2(0, largestUnsigned);

  for (int i = 0; i < 100; i++) {
    int64_t v = dist1(e);

    secSignedInt int1(v, partyId);
    pubSignedInt int2(v);
    EXPECT_THROW(int1[width], std::invalid_argument);
    EXPECT_THROW(int2[width], std::invalid_argument);

    EXPECT_EQ(int1[width - 1].openToParty(partyId).getValue(), v < 0);
    EXPECT_EQ(int2[width - 1].getValue(), v < 0);

    for (size_t j = 0; j < width - 1; j--) {
      EXPECT_EQ(int1[j].openToParty(partyId).getValue(), v & 1);
      EXPECT_EQ(int2[j].getValue(), v & 1);
      v >>= 1;
    }
  }

  for (int i = 0; i < 100; i++) {
    uint64_t v = dist2(e);

    secUnsignedInt int1(v, partyId);
    pubUnsignedInt int2(v);
    EXPECT_THROW(int1[width], std::invalid_argument);
    EXPECT_THROW(int2[width], std::invalid_argument);

    for (size_t j = 0; j < width - 1; j--) {
      EXPECT_EQ(int1[j].openToParty(partyId).getValue(), v & 1);
      EXPECT_EQ(int2[j].getValue(), v & 1);
      v >>= 1;
    }
  }
}

TEST(IntTest, testSubscriptBatch) {
  const int8_t width = 64;

  int64_t largestSigned = std::numeric_limits<int64_t>().max();
  int64_t smallestSigned = std::numeric_limits<int64_t>().min();
  uint64_t largestUnsigned = std::numeric_limits<uint64_t>().max();

  scheduler::SchedulerKeeper<0>::setScheduler(
      std::make_unique<scheduler::PlaintextScheduler>(
          scheduler::WireKeeper::createWithUnorderedMap()));
  using secSignedIntBatch = Integer<Secret<Batch<Signed<width>>>, 0>;
  using pubSignedIntBatch = Integer<Public<Batch<Signed<width>>>, 0>;
  using secUnsignedIntBatch = Integer<Secret<Batch<Unsigned<width>>>, 0>;
  using pubUnsignedIntBatch = Integer<Public<Batch<Unsigned<width>>>, 0>;

  size_t batchSize = 9;

  int partyId = 2;

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<int64_t> dist1(smallestSigned, largestSigned);

  std::uniform_int_distribution<uint64_t> dist2(0, largestUnsigned);

  for (int i = 0; i < 100; i++) {
    std::vector<int64_t> v(batchSize);
    for (size_t j = 0; j < batchSize; j++) {
      v[j] = dist1(e);
    }

    secSignedIntBatch int1(v, partyId);
    pubSignedIntBatch int2(v);
    EXPECT_THROW(int1[width], std::invalid_argument);
    EXPECT_THROW(int2[width], std::invalid_argument);

    std::vector<bool> expectedResults(batchSize);
    std::transform(v.begin(), v.end(), expectedResults.begin(), [](int64_t v) {
      return v < 0;
    });

    testVectorEq(
        int1[width - 1].openToParty(partyId).getValue(), expectedResults);
    testVectorEq(int2[width - 1].getValue(), expectedResults);

    for (size_t j = 0; j < width - 1; j--) {
      std::transform(
          v.begin(), v.end(), expectedResults.begin(), [](int64_t v) {
            return v & 1;
          });
      testVectorEq(int1[j].openToParty(partyId).getValue(), expectedResults);
      testVectorEq(int2[j].getValue(), expectedResults);
      std::transform(
          v.begin(), v.end(), v.begin(), [](int64_t v) { return v >> 1; });
    }
  }

  for (int i = 0; i < 100; i++) {
    std::vector<uint64_t> v(batchSize);

    secUnsignedIntBatch int1(v, partyId);
    pubUnsignedIntBatch int2(v);
    EXPECT_THROW(int1[width], std::invalid_argument);
    EXPECT_THROW(int2[width], std::invalid_argument);
    std::vector<bool> expectedResults(batchSize);

    for (size_t j = 0; j < width - 1; j--) {
      std::transform(
          v.begin(), v.end(), expectedResults.begin(), [](uint64_t v) {
            return v & 1;
          });
      testVectorEq(int1[j].openToParty(partyId).getValue(), expectedResults);
      testVectorEq(int2[j].getValue(), expectedResults);
      std::transform(
          v.begin(), v.end(), v.begin(), [](uint64_t v) { return v >> 1; });
    }
  }
}

TEST(IntTest, testRebatch) {
  const int8_t width = 15;
  scheduler::SchedulerKeeper<0>::setScheduler(
      std::make_unique<scheduler::PlaintextScheduler>(
          scheduler::WireKeeper::createWithUnorderedMap()));
  using secSignedIntBatch = Integer<Secret<Batch<Signed<width>>>, 0>;

  int partyId = 2;

  int size1 = 5;
  int size2 = 2;
  int size3 = 3;

  std::vector<int64_t> v1(size1, (int64_t(1) << (width - 1)) - 1);
  std::vector<int64_t> v2(size2, -1 - v1[0]);
  std::vector<int64_t> v3(size3, 3);

  secSignedIntBatch int1(v1, partyId);
  secSignedIntBatch int2(v2, partyId);
  secSignedIntBatch int3(v3, partyId);

  std::vector<int64_t> expectedV(size1 + size2 + size3);
  for (size_t i = 0; i < size1; i++) {
    expectedV[i] = v1.at(i);
  }
  for (size_t i = 0; i < size2; i++) {
    expectedV[size1 + i] = v2.at(i);
  }

  for (size_t i = 0; i < size3; i++) {
    expectedV[size1 + size2 + i] = v3.at(i);
  }

  auto int4 = int1.batchingWith({int2, int3});

  testVectorEq(int4.openToParty(partyId).getValue(), expectedV);

  auto int123 = int4.unbatching(
      std::make_shared<std::vector<uint32_t>>(std::vector<uint32_t>(
          {static_cast<unsigned int>(size1),
           static_cast<unsigned int>(size2),
           static_cast<unsigned int>(size3)})));

  testVectorEq(int123.at(0).openToParty(partyId).getValue(), v1);
  testVectorEq(int123.at(1).openToParty(partyId).getValue(), v2);
  testVectorEq(int123.at(2).openToParty(partyId).getValue(), v3);
}

} // namespace fbpcf::frontend
