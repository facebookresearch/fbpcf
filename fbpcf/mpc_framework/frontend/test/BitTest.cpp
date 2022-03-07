/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "fbpcf/mpc_framework/frontend/Bit.h"
#include "fbpcf/mpc_framework/frontend/test/schedulerMock.h"
#include "fbpcf/mpc_framework/scheduler/IScheduler.h"
#include "fbpcf/mpc_framework/scheduler/PlaintextScheduler.h"
#include "fbpcf/mpc_framework/scheduler/WireKeeper.h"
#include "fbpcf/mpc_framework/test/TestHelper.h"

namespace fbpcf::mpc_framework::frontend {

using namespace ::testing;

TEST(BitTest, testInputAndOutput) {
  auto mock = std::make_unique<schedulerMock>();

  bool v = true;
  int partyId = 3;

  EXPECT_CALL(*mock, privateBooleanInput(v, partyId)).Times(2);

  EXPECT_CALL(*mock, publicBooleanInput(v)).Times(1);

  EXPECT_CALL(*mock, recoverBooleanWire(v)).Times(1);

  EXPECT_CALL(*mock, openBooleanValueToParty(_, partyId)).Times(1);

  EXPECT_CALL(*mock, extractBooleanSecretShare(_)).Times(1);

  EXPECT_CALL(*mock, getBooleanValue(_)).Times(1);

  scheduler::SchedulerKeeper<0>::setScheduler(std::move(mock));

  using SecBit = Bit<true, 0>;
  using PubBit = Bit<false, 0>;
  {
    SecBit b1(v, partyId);
    SecBit b2;
    b2.privateInput(v, partyId);

    PubBit b3(v);

    SecBit::ExtractedBit extractedBit(v);

    SecBit b4(std::move(extractedBit));

    b1.openToParty(partyId);

    b2.extractBit();

    b3.getValue();
  }
  scheduler::SchedulerKeeper<0>::freeScheduler();
}

TEST(BitTest, testInputAndOutputBatch) {
  auto mock = std::make_unique<schedulerMock>();

  std::vector<bool> v(5, true);
  int partyId = 3;

  EXPECT_CALL(*mock, privateBooleanInputBatch(v, partyId)).Times(2);

  EXPECT_CALL(*mock, publicBooleanInputBatch(v)).Times(1);

  EXPECT_CALL(*mock, recoverBooleanWireBatch(v)).Times(1);

  EXPECT_CALL(*mock, openBooleanValueToPartyBatch(_, partyId)).Times(1);

  EXPECT_CALL(*mock, extractBooleanSecretShareBatch(_)).Times(1);

  EXPECT_CALL(*mock, getBooleanValueBatch(_)).Times(1);

  scheduler::SchedulerKeeper<0>::setScheduler(std::move(mock));

  using SecBitBatch = Bit<true, 0, true>;
  using PubBitBatch = Bit<false, 0, true>;
  {
    SecBitBatch b1(v, partyId);
    SecBitBatch b2;
    b2.privateInput(v, partyId);

    PubBitBatch b3(v);

    SecBitBatch::ExtractedBit extractedBit(v);

    SecBitBatch b4(std::move(extractedBit));

    b1.openToParty(partyId);

    b2.extractBit();

    b3.getValue();
  }
  scheduler::SchedulerKeeper<0>::freeScheduler();
}

TEST(BitTest, testAnd) {
  auto mock = std::make_unique<schedulerMock>();

  bool v = true;
  int partyId = 3;

  EXPECT_CALL(*mock, privateAndPrivate(_, _)).Times(1);

  EXPECT_CALL(*mock, privateAndPublic(_, _)).Times(1);

  EXPECT_CALL(*mock, publicAndPublic(_, _)).Times(1);

  scheduler::SchedulerKeeper<0>::setScheduler(std::move(mock));

  using SecBit = Bit<true, 0>;
  using PubBit = Bit<false, 0>;

  {
    SecBit b1(v, partyId);
    SecBit b2;
    b2.privateInput(v, partyId);

    PubBit b3(v);

    PubBit b4(v);

    auto b5 = b1 & b2;
    auto b6 = b1 & b3;
    auto b7 = b3 & b4;
  }
  // everything must be out of scope before free the scheduler.
  scheduler::SchedulerKeeper<0>::freeScheduler();
}

TEST(BitTest, testAndBatch) {
  auto mock = std::make_unique<schedulerMock>();

  std::vector<bool> v(5, true);
  int partyId = 3;

  EXPECT_CALL(*mock, privateAndPrivateBatch(_, _)).Times(1);

  EXPECT_CALL(*mock, privateAndPublicBatch(_, _)).Times(1);

  EXPECT_CALL(*mock, publicAndPublicBatch(_, _)).Times(1);

  scheduler::SchedulerKeeper<0>::setScheduler(std::move(mock));

  using SecBitBatch = Bit<true, 0, true>;
  using PubBitBatch = Bit<false, 0, true>;

  {
    SecBitBatch b1(v, partyId);
    SecBitBatch b2;
    b2.privateInput(v, partyId);

    PubBitBatch b3(v);

    PubBitBatch b4(v);

    auto b5 = b1 & b2;
    auto b6 = b1 & b3;
    auto b7 = b3 & b4;
  }
  // everything must be out of scope before free the scheduler.
  scheduler::SchedulerKeeper<0>::freeScheduler();
}

TEST(BitTest, testXor) {
  auto mock = std::make_unique<schedulerMock>();

  bool v = true;
  int partyId = 3;

  EXPECT_CALL(*mock, privateXorPrivate(_, _)).Times(1);

  EXPECT_CALL(*mock, privateXorPublic(_, _)).Times(1);

  EXPECT_CALL(*mock, publicXorPublic(_, _)).Times(1);

  scheduler::SchedulerKeeper<0>::setScheduler(std::move(mock));

  using SecBit = Bit<true, 0>;
  using PubBit = Bit<false, 0>;
  {
    SecBit b1(v, partyId);
    SecBit b2;
    b2.privateInput(v, partyId);

    PubBit b3(v);

    PubBit b4(v);

    auto b5 = b1 ^ b2;
    auto b6 = b1 ^ b3;
    auto b7 = b3 ^ b4;
  }
  scheduler::SchedulerKeeper<0>::freeScheduler();
}

TEST(BitTest, testXorBatch) {
  auto mock = std::make_unique<schedulerMock>();

  std::vector<bool> v(5, true);
  int partyId = 3;

  EXPECT_CALL(*mock, privateXorPrivateBatch(_, _)).Times(1);

  EXPECT_CALL(*mock, privateXorPublicBatch(_, _)).Times(1);

  EXPECT_CALL(*mock, publicXorPublicBatch(_, _)).Times(1);

  scheduler::SchedulerKeeper<0>::setScheduler(std::move(mock));

  using SecBitBatch = Bit<true, 0, true>;
  using PubBitBatch = Bit<false, 0, true>;
  {
    SecBitBatch b1(v, partyId);
    SecBitBatch b2;
    b2.privateInput(v, partyId);

    PubBitBatch b3(v);

    PubBitBatch b4(v);

    auto b5 = b1 ^ b2;
    auto b6 = b1 ^ b3;
    auto b7 = b3 ^ b4;
  }
  scheduler::SchedulerKeeper<0>::freeScheduler();
}

TEST(BitTest, testNot) {
  auto mock = std::make_unique<schedulerMock>();

  bool v = true;
  int partyId = 3;

  EXPECT_CALL(*mock, notPrivate(_)).Times(1);

  EXPECT_CALL(*mock, notPublic(_)).Times(1);

  scheduler::SchedulerKeeper<0>::setScheduler(std::move(mock));

  using SecBit = Bit<true, 0>;
  using PubBit = Bit<false, 0>;
  {
    SecBit b1(v, partyId);

    PubBit b2(v);

    auto b3 = !b1;
    auto b4 = !b2;
  }
  scheduler::SchedulerKeeper<0>::freeScheduler();
}

TEST(BitTest, testNotBatch) {
  auto mock = std::make_unique<schedulerMock>();

  std::vector<bool> v(5, true);
  int partyId = 3;

  EXPECT_CALL(*mock, notPrivateBatch(_)).Times(1);

  EXPECT_CALL(*mock, notPublicBatch(_)).Times(1);

  scheduler::SchedulerKeeper<0>::setScheduler(std::move(mock));

  using SecBitBatch = Bit<true, 0, true>;
  using PubBitBatch = Bit<false, 0, true>;
  {
    SecBitBatch b1(v, partyId);

    PubBitBatch b2(v);

    auto b3 = !b1;
    auto b4 = !b2;
  }
  scheduler::SchedulerKeeper<0>::freeScheduler();
}

TEST(BitTest, testOr) {
  auto mock = std::make_unique<schedulerMock>();

  bool v = true;
  int partyId = 3;

  EXPECT_CALL(*mock, privateXorPrivate(_, _)).Times(3);

  EXPECT_CALL(*mock, privateXorPublic(_, _)).Times(1);

  EXPECT_CALL(*mock, publicXorPublic(_, _)).Times(2);

  EXPECT_CALL(*mock, privateAndPrivate(_, _)).Times(1);

  EXPECT_CALL(*mock, privateAndPublic(_, _)).Times(1);

  EXPECT_CALL(*mock, publicAndPublic(_, _)).Times(1);

  scheduler::SchedulerKeeper<0>::setScheduler(std::move(mock));

  using SecBit = Bit<true, 0>;
  using PubBit = Bit<false, 0>;
  {
    SecBit b1(v, partyId);
    SecBit b2;
    b2.privateInput(v, partyId);

    PubBit b3(v);

    PubBit b4(v);

    auto b5 = b1 || b2;
    auto b6 = b1 || b3;
    auto b7 = b3 || b4;
  }
  scheduler::SchedulerKeeper<0>::freeScheduler();
}

TEST(BitTest, testOrBatch) {
  auto mock = std::make_unique<schedulerMock>();

  std::vector<bool> v(5, true);
  int partyId = 3;

  EXPECT_CALL(*mock, privateXorPrivateBatch(_, _)).Times(3);

  EXPECT_CALL(*mock, privateXorPublicBatch(_, _)).Times(1);

  EXPECT_CALL(*mock, publicXorPublicBatch(_, _)).Times(2);

  EXPECT_CALL(*mock, privateAndPrivateBatch(_, _)).Times(1);

  EXPECT_CALL(*mock, privateAndPublicBatch(_, _)).Times(1);

  EXPECT_CALL(*mock, publicAndPublicBatch(_, _)).Times(1);

  scheduler::SchedulerKeeper<0>::setScheduler(std::move(mock));

  using SecBitBatch = Bit<true, 0, true>;
  using PubBitBatch = Bit<false, 0, true>;
  {
    SecBitBatch b1(v, partyId);
    SecBitBatch b2;
    b2.privateInput(v, partyId);

    PubBitBatch b3(v);

    PubBitBatch b4(v);

    auto b5 = b1 || b2;
    auto b6 = b1 || b3;
    auto b7 = b3 || b4;
  }
  scheduler::SchedulerKeeper<0>::freeScheduler();
}

TEST(BitTest, testOrPlaintextScheduler) {
  scheduler::SchedulerKeeper<0>::setScheduler(
      std::make_unique<scheduler::PlaintextScheduler>(
          scheduler::WireKeeper::createWithUnorderedMap()));

  int partyId = 3;

  using SecBit = Bit<true, 0>;
  using PubBit = Bit<false, 0>;
  {
    for (auto v1 : {true, false}) {
      for (auto v3 : {true, false}) {
        bool v2 = v1;
        bool v4 = v3;

        SecBit b1(v1, partyId);
        PubBit b2(v2);

        SecBit b3(v3, partyId);
        PubBit b4(v4);

        auto b5 = b1 || b3;
        auto b6 = b2 || b4;
        auto b7 = b1 || b4;

        EXPECT_EQ(b5.openToParty(partyId).getValue(), v1 || v3);
        EXPECT_EQ(b6.getValue(), v2 || v4);
        EXPECT_EQ(b7.openToParty(partyId).getValue(), v1 || v4);
      }
    }
  }
  scheduler::SchedulerKeeper<0>::freeScheduler();
}

TEST(BitTest, testOrBatchPlaintextScheduler) {
  scheduler::SchedulerKeeper<0>::setScheduler(
      std::make_unique<scheduler::PlaintextScheduler>(
          scheduler::WireKeeper::createWithUnorderedMap()));

  int partyId = 3;

  using SecBitBatch = Bit<true, 0, true>;
  using PubBitBatch = Bit<false, 0, true>;
  {
    for (auto v1 : {true, false}) {
      for (auto v3 : {true, false}) {
        bool v2 = v1;
        bool v4 = v3;

        SecBitBatch b1({v1, v3}, partyId);
        PubBitBatch b2({v2, v4});

        SecBitBatch b3({v3, v1}, partyId);
        PubBitBatch b4({v4, v2});

        auto b5 = b1 || b3;
        auto b6 = b2 || b4;
        auto b7 = b1 || b4;

        auto output1 = b5.openToParty(partyId).getValue();
        auto output2 = b6.getValue();
        auto output3 = b7.openToParty(partyId).getValue();

        EXPECT_EQ(output1.size(), 2);
        EXPECT_EQ(output2.size(), 2);
        EXPECT_EQ(output3.size(), 2);

        EXPECT_EQ(output1[0], v1 || v3);
        EXPECT_EQ(output1[1], v1 || v3);
        EXPECT_EQ(output2[0], v2 || v4);
        EXPECT_EQ(output2[1], v2 || v4);
        EXPECT_EQ(output3[0], v1 || v4);
        EXPECT_EQ(output3[1], v3 || v2);
      }
    }
  }
  scheduler::SchedulerKeeper<0>::freeScheduler();
}

TEST(BitTest, testDeallocateWires) {
  scheduler::SchedulerKeeper<0>::setScheduler(
      std::make_unique<scheduler::PlaintextScheduler>(
          scheduler::WireKeeper::createWithUnorderedMap()));

  using PubBit = Bit<false, 0>;
  using SecBit = Bit<true, 0>;

  {
    SecBit b1{true, 0};
    SecBit b2{false, 0};

    // b1 and b2 are allocated
    testPairEq(scheduler::SchedulerKeeper<0>::getWireStatistics(), {2, 0});

    // Do some random operations
    for (auto v : {true, false}) {
      b1 = b1 ^ b2;
      b2 = b1 & PubBit{v};
      b2 = b2 & b1;
      b1 = !b2;
    }

    b1.openToParty(0).getValue();

    // b1 and b2 are still in scope, but everything else has been deallocated
    testPairEq(scheduler::SchedulerKeeper<0>::getWireStatistics(), {13, 11});
  }

  // All wires are now deallocated
  testPairEq(scheduler::SchedulerKeeper<0>::getWireStatistics(), {13, 13});

  scheduler::SchedulerKeeper<0>::freeScheduler();
}

TEST(BitTest, testReferenceCount) {
  auto mock = std::make_unique<schedulerMock>();

  bool v = true;
  int partyId = 3;

  EXPECT_CALL(*mock, increaseReferenceCount(_)).Times(4);

  EXPECT_CALL(*mock, decreaseReferenceCount(_)).Times(6);

  scheduler::SchedulerKeeper<0>::setScheduler(std::move(mock));

  using SecBit = Bit<true, 0>;

  {
    SecBit b1(v, partyId);

    // increase reference
    auto b2(b1);

    // no change
    auto b3(std::move(b2));

    std::cout << "here executed" << std::endl;
    // increase reference
    // decrease reference
    b1 = b3;

    // no change
    b2 = std::move(b1);

    // when out of scope, b2, b3 cause 2 decrease reference count
  }

  using PubBit = Bit<false, 0>;
  {
    PubBit b1(v);

    // increase reference
    auto b2(b1);

    // no change
    auto b3(std::move(b2));

    std::cout << "here executed" << std::endl;
    // increase reference
    // decrease reference
    b1 = b3;

    // no change
    b2 = std::move(b1);

    // when out of scope, b2, b3 cause 2 decrease reference count
  }

  scheduler::SchedulerKeeper<0>::freeScheduler();
}

TEST(BitTest, testReferenceCountBatch) {
  auto mock = std::make_unique<schedulerMock>();

  std::vector<bool> v(5, true);
  int partyId = 3;

  EXPECT_CALL(*mock, increaseReferenceCountBatch(_)).Times(4);

  EXPECT_CALL(*mock, decreaseReferenceCountBatch(_)).Times(6);

  scheduler::SchedulerKeeper<0>::setScheduler(std::move(mock));

  using SecBitBatch = Bit<true, 0, true>;

  {
    SecBitBatch b1(v, partyId);

    // increase reference
    auto b2(b1);

    // no change
    auto b3(std::move(b2));

    std::cout << "here executed" << std::endl;
    // increase reference
    // decrease reference
    b1 = b3;

    // no change
    b2 = std::move(b1);

    // when out of scope, b2, b3 cause 2 decrease reference count
  }

  using PubBitBatch = Bit<false, 0, true>;
  {
    PubBitBatch b1(v);

    // increase reference
    auto b2(b1);

    // no change
    auto b3(std::move(b2));

    std::cout << "here executed" << std::endl;
    // increase reference
    // decrease reference
    b1 = b3;

    // no change
    b2 = std::move(b1);

    // when out of scope, b2, b3 cause 2 decrease reference count
  }

  scheduler::SchedulerKeeper<0>::freeScheduler();
}

} // namespace fbpcf::mpc_framework::frontend
