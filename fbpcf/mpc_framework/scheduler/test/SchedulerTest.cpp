/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/mpc_framework/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/mpc_framework/scheduler/EagerScheduler.h"
#include "fbpcf/mpc_framework/scheduler/IScheduler.h"
#include "fbpcf/mpc_framework/scheduler/LazyScheduler.h"
#include "fbpcf/mpc_framework/scheduler/NetworkPlaintextScheduler.h"
#include "fbpcf/mpc_framework/scheduler/PlaintextScheduler.h"
#include "fbpcf/mpc_framework/scheduler/WireKeeper.h"
#include "fbpcf/mpc_framework/scheduler/gate_keeper/GateKeeper.h"
#include "fbpcf/mpc_framework/test/TestHelper.h"

namespace fbpcf::mpc_framework::scheduler {

const bool unsafe = false;
const int numberOfParties = 2;

void runWithScheduler(
    SchedulerType schedulerType,
    std::function<void(std::unique_ptr<IScheduler> scheduler, int8_t myID)>
        testBody) {
  auto schedulerCreator = getSchedulerCreator<unsafe>(schedulerType);
  auto agentFactories =
      engine::communication::getInMemoryAgentFactory(numberOfParties);

  std::vector<std::future<void>> futures;

  for (auto i = 0; i < numberOfParties; ++i) {
    futures.push_back(std::async(
        [i, schedulerCreator, testBody](
            std::reference_wrapper<
                engine::communication::IPartyCommunicationAgentFactory>
                agentFactory) {
          testBody(schedulerCreator(i, agentFactory), i);
        },
        std::reference_wrapper<
            engine::communication::IPartyCommunicationAgentFactory>(
            *agentFactories.at(i))));
  }

  for (auto i = 0; i < numberOfParties; ++i) {
    futures.at(i).get();
  }
}

class SchedulerTestFixture : public ::testing::TestWithParam<SchedulerType> {};

INSTANTIATE_TEST_SUITE_P(
    SchedulerTest,
    SchedulerTestFixture,
    ::testing::Values(
        SchedulerType::Plaintext,
        SchedulerType::NetworkPlaintext,
        SchedulerType::Eager,
        SchedulerType::Lazy),
    [](const testing::TestParamInfo<SchedulerTestFixture::ParamType>& info) {
      return getSchedulerName(info.param);
    });

void testInputAndOutput(std::unique_ptr<IScheduler> scheduler, int8_t myID) {
  // Public inputs
  auto wire1 = scheduler->publicBooleanInput(true);
  EXPECT_TRUE(scheduler->getBooleanValue(wire1));

  auto wire2 = scheduler->publicBooleanInput(false);
  EXPECT_FALSE(scheduler->getBooleanValue(wire2));

  // Private inputs
  auto wire3 = scheduler->privateBooleanInput(false, 0);
  auto wire4 = scheduler->privateBooleanInput(true, 1);

  // Reveal 0's input to 1
  auto wire5 =
      scheduler->getBooleanValue(scheduler->openBooleanValueToParty(wire3, 1));
  if (myID == 1) {
    EXPECT_FALSE(wire5);
  }

  auto share = scheduler->extractBooleanSecretShare(wire4);
  auto wire6 = scheduler->recoverBooleanWire(share);

  // Reveal 1's input to 0
  auto wire7 =
      scheduler->getBooleanValue(scheduler->openBooleanValueToParty(wire6, 0));
  if (myID == 0) {
    EXPECT_TRUE(wire7);
  }
  auto gateCount = scheduler->getGateStatistics();
  EXPECT_EQ(gateCount.first, 2);
  EXPECT_EQ(gateCount.second, 5);
}

TEST_P(SchedulerTestFixture, testInputAndOutput) {
  runWithScheduler(GetParam(), testInputAndOutput);
}

void testInputAndOutputBatch(
    std::unique_ptr<IScheduler> scheduler,
    int8_t myID) {
  // Public inputs
  auto wire1 = scheduler->publicBooleanInputBatch({true, false});
  testVectorEq(scheduler->getBooleanValueBatch(wire1), {true, false});

  auto wire2 = scheduler->publicBooleanInputBatch({false, true});
  testVectorEq(scheduler->getBooleanValueBatch(wire2), {false, true});

  // Private inputs
  auto wire3 = scheduler->privateBooleanInputBatch({false, true}, 0);
  auto wire4 = scheduler->privateBooleanInputBatch({true, false}, 1);

  // Reveal 0's input to 1
  auto wire5 = scheduler->getBooleanValueBatch(
      scheduler->openBooleanValueToPartyBatch(wire3, 1));
  if (myID == 1) {
    testVectorEq(wire5, {false, true});
  }

  auto share = scheduler->extractBooleanSecretShareBatch(wire4);
  auto wire6 = scheduler->recoverBooleanWireBatch(share);

  // Reveal 1's input to 0
  auto wire7 = scheduler->getBooleanValueBatch(
      scheduler->openBooleanValueToPartyBatch(wire6, 0));
  if (myID == 0) {
    testVectorEq(wire7, {true, false});
  }

  auto gateCount = scheduler->getGateStatistics();
  EXPECT_EQ(gateCount.first, 4);
  EXPECT_EQ(gateCount.second, 10);
}

TEST_P(SchedulerTestFixture, testInputAndOutputBatch) {
  runWithScheduler(GetParam(), testInputAndOutputBatch);
}

void testAnd(std::unique_ptr<IScheduler> scheduler, int8_t myID) {
  for (auto v1 : {true, false}) {
    for (auto v2 : {true, false}) {
      // Private and private
      auto wire1 =
          scheduler->getBooleanValue(scheduler->openBooleanValueToParty(
              scheduler->privateAndPrivate(
                  scheduler->privateBooleanInput(v1, 0),
                  scheduler->privateBooleanInput(v2, 1)),
              0));
      if (myID == 0) {
        EXPECT_EQ(wire1, v1 & v2);
      }

      // Public and public
      auto wire2 = scheduler->publicAndPublic(
          scheduler->publicBooleanInput(v1), scheduler->publicBooleanInput(v2));
      EXPECT_EQ(scheduler->getBooleanValue(wire2), v1 & v2);

      // Private and public
      auto wire3 =
          scheduler->getBooleanValue(scheduler->openBooleanValueToParty(
              scheduler->privateAndPublic(
                  scheduler->privateBooleanInput(v1, 0),
                  scheduler->publicBooleanInput(v2)),
              1));
      if (myID == 1) {
        EXPECT_EQ(wire3, v1 & v2);
      }
    }
  }
  auto gateCount = scheduler->getGateStatistics();
  EXPECT_EQ(gateCount.first, 12);
  EXPECT_EQ(gateCount.second, 32);
}

TEST_P(SchedulerTestFixture, testAnd) {
  runWithScheduler(GetParam(), testAnd);
}

void testAndBatch(std::unique_ptr<IScheduler> scheduler, int8_t myID) {
  for (auto v1 : {true, false}) {
    for (auto v2 : {true, false}) {
      // Private and private
      auto wire1 = scheduler->getBooleanValueBatch(
          scheduler->openBooleanValueToPartyBatch(
              scheduler->privateAndPrivateBatch(
                  scheduler->privateBooleanInputBatch({v1, v1}, 0),
                  scheduler->privateBooleanInputBatch({v2, v1}, 1)),
              0));
      if (myID == 0) {
        testVectorEq(wire1, {v1 && v2, v1 && v1});
      }

      // Public and public
      auto wire2 =
          scheduler->getBooleanValueBatch(scheduler->publicAndPublicBatch(
              scheduler->publicBooleanInputBatch({v1, v1}),
              scheduler->publicBooleanInputBatch({v2, v1})));
      testVectorEq(wire2, {v1 && v2, v1 && v1});

      // Private and public
      auto wire3 = scheduler->getBooleanValueBatch(
          scheduler->openBooleanValueToPartyBatch(
              scheduler->privateAndPublicBatch(
                  scheduler->privateBooleanInputBatch({v1, v1}, 0),
                  scheduler->publicBooleanInputBatch({v2, v1})),
              1));
      if (myID == 1) {
        testVectorEq(wire3, {v1 && v2, v1 && v1});
      }
    }
  }

  auto gateCount = scheduler->getGateStatistics();
  EXPECT_EQ(gateCount.first, 24);
  EXPECT_EQ(gateCount.second, 64);
}

TEST_P(SchedulerTestFixture, testAndBatch) {
  runWithScheduler(GetParam(), testAndBatch);
}

void testXor(std::unique_ptr<IScheduler> scheduler, int8_t myID) {
  for (auto v1 : {true, false}) {
    for (auto v2 : {true, false}) {
      // Private xor private
      auto wire1 =
          scheduler->getBooleanValue(scheduler->openBooleanValueToParty(
              scheduler->privateXorPrivate(
                  scheduler->privateBooleanInput(v1, 0),
                  scheduler->privateBooleanInput(v2, 1)),
              0));
      if (myID == 0) {
        EXPECT_EQ(wire1, v1 ^ v2);
      }

      // Public xor public
      auto wire2 = scheduler->publicXorPublic(
          scheduler->publicBooleanInput(v1), scheduler->publicBooleanInput(v2));
      EXPECT_EQ(scheduler->getBooleanValue(wire2), v1 ^ v2);

      // Private xor public
      auto wire3 =
          scheduler->getBooleanValue(scheduler->openBooleanValueToParty(
              scheduler->privateXorPublic(
                  scheduler->privateBooleanInput(v1, 0),
                  scheduler->publicBooleanInput(v2)),
              1));
      if (myID == 1) {
        EXPECT_EQ(wire3, v1 ^ v2);
      }
    }
  }
  auto gateCount = scheduler->getGateStatistics();
  EXPECT_EQ(gateCount.first, 8);
  EXPECT_EQ(gateCount.second, 36);
}

TEST_P(SchedulerTestFixture, testXor) {
  runWithScheduler(GetParam(), testXor);
}

void testXorBatch(std::unique_ptr<IScheduler> scheduler, int8_t myID) {
  for (auto v1 : {true, false}) {
    for (auto v2 : {true, false}) {
      // Private xor private
      auto wire1 = scheduler->getBooleanValueBatch(
          scheduler->openBooleanValueToPartyBatch(
              scheduler->privateXorPrivateBatch(
                  scheduler->privateBooleanInputBatch({v1, v1}, 0),
                  scheduler->privateBooleanInputBatch({v2, v1}, 1)),
              0));
      if (myID == 0) {
        testVectorEq(wire1, {v1 != v2, v1 != v1});
      }

      // Public xor public
      auto wire2 = scheduler->publicXorPublicBatch(
          scheduler->publicBooleanInputBatch({v1, v1}),
          scheduler->publicBooleanInputBatch({v2, v1}));
      testVectorEq(
          scheduler->getBooleanValueBatch(wire2), {v1 != v2, v1 != v1});

      // Private xor public
      auto wire3 = scheduler->getBooleanValueBatch(
          scheduler->openBooleanValueToPartyBatch(
              scheduler->privateXorPublicBatch(
                  scheduler->privateBooleanInputBatch({v1, v1}, 0),
                  scheduler->publicBooleanInputBatch({v2, v1})),
              1));
      if (myID == 1) {
        testVectorEq(wire3, {v1 != v2, v1 != v1});
      }
    }
  }

  auto gateCount = scheduler->getGateStatistics();
  EXPECT_EQ(gateCount.first, 16);
  EXPECT_EQ(gateCount.second, 72);
}

TEST_P(SchedulerTestFixture, testXorBatch) {
  runWithScheduler(GetParam(), testXorBatch);
}

void testNot(std::unique_ptr<IScheduler> scheduler, int8_t myID) {
  for (auto v1 : {true, false}) {
    // Not private
    auto wire1 = scheduler->getBooleanValue(scheduler->openBooleanValueToParty(
        scheduler->notPrivate(scheduler->privateBooleanInput(v1, 1)), 0));
    if (myID == 0) {
      EXPECT_EQ(wire1, !v1);
    }

    // Not public
    auto wire2 = scheduler->notPublic(scheduler->publicBooleanInput(v1));
    EXPECT_EQ(scheduler->getBooleanValue(wire2), !v1);
  }
  auto gateCount = scheduler->getGateStatistics();
  EXPECT_EQ(gateCount.first, 2);
  EXPECT_EQ(gateCount.second, 8);
}

TEST_P(SchedulerTestFixture, testNot) {
  runWithScheduler(GetParam(), testNot);
}

void testNotBatch(std::unique_ptr<IScheduler> scheduler, int8_t myID) {
  for (auto v1 : {true, false}) {
    // Not private
    auto wire1 =
        scheduler->getBooleanValueBatch(scheduler->openBooleanValueToPartyBatch(
            scheduler->notPrivateBatch(
                scheduler->privateBooleanInputBatch({v1, !v1}, 1)),
            0));
    if (myID == 0) {
      testVectorEq(wire1, {!v1, v1});
    }

    // Not public
    auto wire2 = scheduler->notPublicBatch(
        scheduler->publicBooleanInputBatch({v1, !v1}));
    testVectorEq(scheduler->getBooleanValueBatch(wire2), {!v1, v1});
  }

  auto gateCount = scheduler->getGateStatistics();
  EXPECT_EQ(gateCount.first, 4);
  EXPECT_EQ(gateCount.second, 16);
}

TEST_P(SchedulerTestFixture, testNotBatch) {
  runWithScheduler(GetParam(), testNotBatch);
}

void testMultipleOperations(
    std::unique_ptr<IScheduler> scheduler,
    int8_t myID) {
  auto v1 = scheduler->privateBooleanInput(true, 0);
  auto v2 = scheduler->privateBooleanInput(false, 0);

  auto v3 = scheduler->privateXorPrivate(v1, v2); // true
  auto v4 = scheduler->privateAndPrivate(v3, v1); // true
  auto v5 = scheduler->notPrivate(v4); // false

  auto revealed1 =
      scheduler->getBooleanValue(scheduler->openBooleanValueToParty(v5, 0));
  if (myID == 0) {
    EXPECT_FALSE(revealed1);
  }

  auto v6 = scheduler->publicBooleanInput(false);

  auto v7 = scheduler->notPublic(v6); // true
  auto v8 = scheduler->privateAndPublic(v1, v7); // true
  auto v9 = scheduler->privateXorPublic(v2, v7); // true

  auto revealed2 =
      scheduler->getBooleanValue(scheduler->openBooleanValueToParty(v8, 1));
  auto revealed3 =
      scheduler->getBooleanValue(scheduler->openBooleanValueToParty(v9, 1));
  if (myID == 1) {
    EXPECT_TRUE(revealed2);
    EXPECT_TRUE(revealed3);
  }

  auto v10 = scheduler->publicBooleanInput(true);

  auto v11 = scheduler->publicAndPublic(v10, v6); // false
  auto v12 = scheduler->publicXorPublic(v11, v10); // true
  auto v13 = scheduler->publicAndPublic(v12, v7); // true

  EXPECT_TRUE(scheduler->getBooleanValue(v13));

  auto v14 = scheduler->privateBooleanInput(true, 1);
  auto v15 = scheduler->privateAndPrivate(v14, v1); // true
  auto v16 = scheduler->notPrivate(v15); // false
  auto v17 = scheduler->privateXorPrivate(v1, v14); // false
  auto v18 = scheduler->privateAndPrivate(v16, v17); // false
  auto v19 = scheduler->privateXorPrivate(v18, v15); // true

  auto revealed4 =
      scheduler->getBooleanValue(scheduler->openBooleanValueToParty(v19, 0));
  auto revealed5 =
      scheduler->getBooleanValue(scheduler->openBooleanValueToParty(v18, 0));
  if (myID == 0) {
    EXPECT_TRUE(revealed4);
    EXPECT_FALSE(revealed5);
  }
}

TEST_P(SchedulerTestFixture, testMultipleOperations) {
  runWithScheduler(GetParam(), testMultipleOperations);
}

void testReferenceCount(
    std::unique_ptr<IScheduler> scheduler,
    int8_t /*myId*/) {
  auto wire = scheduler->privateBooleanInput(true, 0);

  for (int i = 0; i < 10; i++) {
    if (i % 3 == 2) {
      scheduler->decreaseReferenceCount(wire);
    } else {
      scheduler->increaseReferenceCount(wire);
    }
    EXPECT_NO_THROW(scheduler->getBooleanValue(wire));
  }

  for (int i = 0; i < 10; i++) {
    EXPECT_NO_THROW(scheduler->getBooleanValue(wire));
    if (i % 3 != 2) {
      scheduler->decreaseReferenceCount(wire);
    } else {
      scheduler->increaseReferenceCount(wire);
    }
  }

  testPairEq(scheduler->getWireStatistics(), {1, 0});

  scheduler->decreaseReferenceCount(wire);
  EXPECT_THROW(scheduler->getBooleanValue(wire), std::runtime_error);

  testPairEq(scheduler->getWireStatistics(), {1, 1});
}

TEST_P(SchedulerTestFixture, testReferenceCount) {
  runWithScheduler(GetParam(), testReferenceCount);
}

void testReferenceCountBatch(
    std::unique_ptr<IScheduler> scheduler,
    int8_t /*myId*/) {
  auto wire = scheduler->privateBooleanInputBatch({true, false}, 0);

  for (int i = 0; i < 10; i++) {
    if (i % 3 == 2) {
      scheduler->decreaseReferenceCountBatch(wire);
    } else {
      scheduler->increaseReferenceCountBatch(wire);
    }
    EXPECT_NO_THROW(scheduler->getBooleanValueBatch(wire));
  }

  for (int i = 0; i < 10; i++) {
    EXPECT_NO_THROW(scheduler->getBooleanValueBatch(wire));
    if (i % 3 != 2) {
      scheduler->decreaseReferenceCountBatch(wire);
    } else {
      scheduler->increaseReferenceCountBatch(wire);
    }
  }

  testPairEq(scheduler->getWireStatistics(), {1, 0});

  scheduler->decreaseReferenceCountBatch(wire);
  EXPECT_THROW(scheduler->getBooleanValueBatch(wire), std::runtime_error);

  testPairEq(scheduler->getWireStatistics(), {1, 1});
}

TEST_P(SchedulerTestFixture, testReferenceCountBatch) {
  runWithScheduler(GetParam(), testReferenceCountBatch);
}

class CompositeSchedulerTestFixture
    : public ::testing::TestWithParam<std::tuple<SchedulerType, size_t>> {};

INSTANTIATE_TEST_SUITE_P(
    SchedulerTest,
    CompositeSchedulerTestFixture,
    ::testing::Combine(
        ::testing::Values(
            SchedulerType::Plaintext,
            SchedulerType::NetworkPlaintext),
        ::testing::Values(16, 256, 1024)),
    [](const testing::TestParamInfo<CompositeSchedulerTestFixture::ParamType>&
           info) {
      return getSchedulerName(std::get<0>(info.param)) + "_" +
          std::to_string(std::get<1>(info.param));
    });

void testCompositeAND(
    std::unique_ptr<IScheduler> scheduler,
    int8_t myID,
    size_t compositeSize) {
  for (auto sharedBit : {true, false}) {
    // set-up values
    std::vector<bool> rightBits(compositeSize, false);
    for (int i = 0; i * i < rightBits.size(); ++i) {
      rightBits[i * i] = true;
    }
    for (int i = 0; i * i * i < rightBits.size(); ++i) {
      rightBits[i * i * i] = true;
    }

    // set-up input wires
    auto publicSharedWire = scheduler->publicBooleanInput(sharedBit);
    auto privateSharedWire = scheduler->privateBooleanInput(sharedBit, 0);

    std::vector<IScheduler::WireId<IScheduler::Boolean>> publicRightWires(
        compositeSize);
    std::vector<IScheduler::WireId<IScheduler::Boolean>> privateRightWires(
        compositeSize);
    for (int i = 0; i < compositeSize; ++i) {
      publicRightWires[i] = scheduler->publicBooleanInput(rightBits[i]);
      privateRightWires[i] =
          scheduler->privateBooleanInput(rightBits[i], i % numberOfParties);
    }

    // Private and private
    {
      auto resultWires = scheduler->privateAndPrivateComposite(
          privateSharedWire, privateRightWires);
      EXPECT_EQ(resultWires.size(), compositeSize);
      for (int i = 0; i < compositeSize; ++i) {
        auto val = scheduler->getBooleanValue(
            scheduler->openBooleanValueToParty(resultWires[i], 0));
        if (myID == 0) {
          EXPECT_EQ(val, sharedBit & rightBits[i])
              << "Result different in index " + std::to_string(i);
        }
      }
    }

    // Public And Private
    {
      auto resultWires1 = scheduler->privateAndPublicComposite(
          privateSharedWire, publicRightWires);
      auto resultWires2 = scheduler->privateAndPublicComposite(
          publicSharedWire, privateRightWires);
      EXPECT_EQ(resultWires1.size(), compositeSize);
      EXPECT_EQ(resultWires2.size(), compositeSize);
      for (int i = 0; i < compositeSize; ++i) {
        auto val1 = scheduler->getBooleanValue(
            scheduler->openBooleanValueToParty(resultWires1[i], 0));
        auto val2 = scheduler->getBooleanValue(
            scheduler->openBooleanValueToParty(resultWires2[i], 1));
        if (myID == 0) {
          EXPECT_EQ(val1, sharedBit & rightBits[i])
              << "Result different in index " + std::to_string(i);
        } else if (myID == 1) {
          EXPECT_EQ(val2, sharedBit & rightBits[i])
              << "Result different in index " + std::to_string(i);
        }
      }
    }

    // Public and Public
    {
      auto resultWires = scheduler->publicAndPublicComposite(
          publicSharedWire, publicRightWires);
      EXPECT_EQ(resultWires.size(), compositeSize);
      for (int i = 0; i < compositeSize; ++i) {
        EXPECT_EQ(
            scheduler->getBooleanValue(resultWires[i]),
            sharedBit & rightBits[i]);
      }
    }
  }
  auto gateCount = scheduler->getGateStatistics();
  EXPECT_EQ(gateCount.first, 8 * compositeSize);
  EXPECT_EQ(gateCount.second, 4 + 10 * compositeSize);
}

TEST_P(CompositeSchedulerTestFixture, testCompositeAND) {
  auto schedulerType = std::get<0>(GetParam());
  auto andSize = std::get<1>(GetParam());
  runWithScheduler(
      schedulerType,
      [andSize](std::unique_ptr<IScheduler> scheduler, int8_t myID) {
        testCompositeAND(std::move(scheduler), myID, andSize);
      });
}

void testCompositeANDBatch(
    std::unique_ptr<IScheduler> scheduler,
    int8_t myID,
    size_t compositeSize) {
  for (auto sharedBit : {true, false}) {
    // set-up values
    std::vector<bool> leftBits = {sharedBit, !sharedBit, sharedBit};
    std::vector<std::vector<bool>> rightBits;
    for (int i = 0; i < compositeSize; i++) {
      rightBits.push_back({false, true, false});
    }
    for (int i = 0; i * i < rightBits.size(); ++i) {
      rightBits[i * i] = {true, true, false};
    }
    for (int i = 0; i * i * i < rightBits.size(); ++i) {
      rightBits[i * i * i] = {false, false, true};
    }

    // set-up input wires
    auto publicSharedWire = scheduler->publicBooleanInputBatch(leftBits);
    auto privateSharedWire = scheduler->privateBooleanInputBatch(leftBits, 0);

    std::vector<IScheduler::WireId<IScheduler::Boolean>> publicRightWires(
        compositeSize);
    std::vector<IScheduler::WireId<IScheduler::Boolean>> privateRightWires(
        compositeSize);
    for (int i = 0; i < compositeSize; ++i) {
      publicRightWires[i] = scheduler->publicBooleanInputBatch(rightBits[i]);
      privateRightWires[i] = scheduler->privateBooleanInputBatch(
          rightBits[i], i % numberOfParties);
    }

    // Private and private
    {
      auto resultWires = scheduler->privateAndPrivateCompositeBatch(
          privateSharedWire, privateRightWires);
      EXPECT_EQ(resultWires.size(), compositeSize);
      for (int i = 0; i < compositeSize; ++i) {
        auto val = scheduler->getBooleanValueBatch(
            scheduler->openBooleanValueToPartyBatch(resultWires[i], 0));
        if (myID == 0) {
          testVectorEq(
              val,
              {leftBits[0] && rightBits[i][0],
               leftBits[1] && rightBits[i][1],
               leftBits[2] && rightBits[i][2]});
        }
      }
    }

    // Public And Private
    {
      auto resultWires1 = scheduler->privateAndPublicCompositeBatch(
          privateSharedWire, publicRightWires);
      auto resultWires2 = scheduler->privateAndPublicCompositeBatch(
          publicSharedWire, privateRightWires);
      EXPECT_EQ(resultWires1.size(), compositeSize);
      EXPECT_EQ(resultWires2.size(), compositeSize);
      for (int i = 0; i < compositeSize; ++i) {
        auto val1 = scheduler->getBooleanValueBatch(
            scheduler->openBooleanValueToPartyBatch(resultWires1[i], 0));
        auto val2 = scheduler->getBooleanValueBatch(
            scheduler->openBooleanValueToPartyBatch(resultWires2[i], 1));
        if (myID == 0) {
          testVectorEq(
              val1,
              {leftBits[0] && rightBits[i][0],
               leftBits[1] && rightBits[i][1],
               leftBits[2] && rightBits[i][2]});
        } else if (myID == 1) {
          testVectorEq(
              val2,
              {leftBits[0] && rightBits[i][0],
               leftBits[1] && rightBits[i][1],
               leftBits[2] && rightBits[i][2]});
        }
      }
    }

    // Public and Public
    {
      auto resultWires = scheduler->publicAndPublicCompositeBatch(
          publicSharedWire, publicRightWires);
      EXPECT_EQ(resultWires.size(), compositeSize);
      for (int i = 0; i < compositeSize; ++i) {
        testVectorEq(
            scheduler->getBooleanValueBatch(resultWires[i]),
            {leftBits[0] && rightBits[i][0],
             leftBits[1] && rightBits[i][1],
             leftBits[2] && rightBits[i][2]});
      }
    }
  }
  auto gateCount = scheduler->getGateStatistics();
  EXPECT_EQ(gateCount.first, 24 * compositeSize);
  EXPECT_EQ(gateCount.second, 12 + 30 * compositeSize);
}

TEST_P(CompositeSchedulerTestFixture, testCompositeANDBatch) {
  auto schedulerType = std::get<0>(GetParam());
  auto andSize = std::get<1>(GetParam());
  runWithScheduler(
      schedulerType,
      [andSize](std::unique_ptr<IScheduler> scheduler, int8_t myID) {
        testCompositeANDBatch(std::move(scheduler), myID, andSize);
      });
}

} // namespace fbpcf::mpc_framework::scheduler
