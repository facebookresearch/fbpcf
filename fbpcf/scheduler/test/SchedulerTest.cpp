/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>
#include <cstdint>

#include "fbpcf/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/scheduler/EagerScheduler.h"
#include "fbpcf/scheduler/IScheduler.h"
#include "fbpcf/scheduler/LazyScheduler.h"
#include "fbpcf/scheduler/NetworkPlaintextScheduler.h"
#include "fbpcf/scheduler/PlaintextScheduler.h"
#include "fbpcf/scheduler/WireKeeper.h"
#include "fbpcf/scheduler/gate_keeper/GateKeeper.h"
#include "fbpcf/test/TestHelper.h"

namespace fbpcf::scheduler {

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

void runWithArithmeticScheduler(
    SchedulerType schedulerType,
    std::function<
        void(std::unique_ptr<IArithmeticScheduler> scheduler, int8_t myID)>
        testBody) {
  auto schedulerCreator = getArithmeticSchedulerCreator<unsafe>(schedulerType);
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
class ArithmeticSchedulerTestFixture
    : public ::testing::TestWithParam<SchedulerType> {};

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

INSTANTIATE_TEST_SUITE_P(
    SchedulerTest,
    ArithmeticSchedulerTestFixture,
    ::testing::Values(
        SchedulerType::Plaintext,
        SchedulerType::NetworkPlaintext,
        SchedulerType::Eager),
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

void testIntegerInputAndOutput(
    std::unique_ptr<IArithmeticScheduler> scheduler,
    int8_t myID) {
  // Public inputs
  auto wire1 = scheduler->publicIntegerInput(12);
  EXPECT_EQ(scheduler->getIntegerValue(wire1), 12);

  auto wire2 = scheduler->publicIntegerInput(((uint64_t)1 << 63) * 2);
  EXPECT_EQ(scheduler->getIntegerValue(wire2), 0);

  // Private inputs
  auto wire3 = scheduler->privateIntegerInput(23, 0);
  auto wire4 = scheduler->privateIntegerInput(79, 1);

  // Reveal 0's input to 1
  auto wire5 =
      scheduler->getIntegerValue(scheduler->openIntegerValueToParty(wire3, 1));
  if (myID == 1) {
    EXPECT_EQ(wire5, 23);
  }

  auto share = scheduler->extractIntegerSecretShare(wire4);
  auto wire6 = scheduler->recoverIntegerWire(share);

  // Reveal 1's input to 0
  auto wire7 =
      scheduler->getIntegerValue(scheduler->openIntegerValueToParty(wire6, 0));
  if (myID == 0) {
    EXPECT_EQ(wire7, 79);
  }
  auto gateCount = scheduler->getGateStatistics();
  EXPECT_EQ(gateCount.first, 2);
  EXPECT_EQ(gateCount.second, 5);
}

TEST_P(ArithmeticSchedulerTestFixture, testInputAndOutput) {
  runWithArithmeticScheduler(GetParam(), testIntegerInputAndOutput);
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

void testIntegerInputAndOutputBatch(
    std::unique_ptr<IArithmeticScheduler> scheduler,
    int8_t myID) {
  // Public inputs
  auto wire1 = scheduler->publicIntegerInputBatch({12, 21});
  testVectorEq(scheduler->getIntegerValueBatch(wire1), {12, 21});

  auto wire2 = scheduler->publicIntegerInputBatch({13, 31});
  testVectorEq(scheduler->getIntegerValueBatch(wire2), {13, 31});

  // Private inputs
  auto wire3 = scheduler->privateIntegerInputBatch({14, 41}, 0);
  auto wire4 =
      scheduler->privateIntegerInputBatch({((uint64_t)1 << 63) * 2 + 1, 51}, 1);

  // Reveal 0's input to 1
  auto wire5 = scheduler->getIntegerValueBatch(
      scheduler->openIntegerValueToPartyBatch(wire3, 1));
  if (myID == 1) {
    testVectorEq(wire5, {14, 41});
  }

  auto share = scheduler->extractIntegerSecretShareBatch(wire4);
  auto wire6 = scheduler->recoverIntegerWireBatch(share);

  // Reveal 1's input to 0
  auto wire7 = scheduler->getIntegerValueBatch(
      scheduler->openIntegerValueToPartyBatch(wire6, 0));
  if (myID == 0) {
    testVectorEq(wire7, {1, 51});
  }

  auto gateCount = scheduler->getGateStatistics();
  EXPECT_EQ(gateCount.first, 4);
  EXPECT_EQ(gateCount.second, 10);
}

TEST_P(ArithmeticSchedulerTestFixture, testInputAndOutputBatch) {
  runWithArithmeticScheduler(GetParam(), testIntegerInputAndOutputBatch);
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

void testMult(std::unique_ptr<IArithmeticScheduler> scheduler, int8_t myID) {
  for (auto v1 : {(uint64_t)1 << 63, (uint64_t)332}) {
    for (auto v2 : {(uint64_t)1 << 63, (uint64_t)13}) {
      // Private Mult private
      auto wire1 =
          scheduler->getIntegerValue(scheduler->openIntegerValueToParty(
              scheduler->privateMultPrivate(
                  scheduler->privateIntegerInput(v1, 0),
                  scheduler->privateIntegerInput(v2, 1)),
              0));
      if (myID == 0) {
        EXPECT_EQ(wire1, v1 * v2);
      }

      // Public Mult public
      auto wire2 = scheduler->publicMultPublic(
          scheduler->publicIntegerInput(v1), scheduler->publicIntegerInput(v2));
      EXPECT_EQ(scheduler->getIntegerValue(wire2), v1 * v2);

      // Private Mult public
      auto wire3 =
          scheduler->getIntegerValue(scheduler->openIntegerValueToParty(
              scheduler->privateMultPublic(
                  scheduler->privateIntegerInput(v1, 0),
                  scheduler->publicIntegerInput(v2)),
              1));
      if (myID == 1) {
        EXPECT_EQ(wire3, v1 * v2);
      }
    }
  }
  auto gateCount = scheduler->getGateStatistics();
  EXPECT_EQ(gateCount.first, 12);
  EXPECT_EQ(gateCount.second, 32);
}

TEST_P(ArithmeticSchedulerTestFixture, testMult) {
  runWithArithmeticScheduler(GetParam(), testMult);
}

void testMultBatch(
    std::unique_ptr<IArithmeticScheduler> scheduler,
    int8_t myID) {
  for (auto v1 : {(uint64_t)1 << 63, (uint64_t)332}) {
    for (auto v2 : {(uint64_t)1 << 63, (uint64_t)13}) {
      // Private Mult private
      auto wire1 = scheduler->getIntegerValueBatch(
          scheduler->openIntegerValueToPartyBatch(
              scheduler->privateMultPrivateBatch(
                  scheduler->privateIntegerInputBatch({v1, v1}, 0),
                  scheduler->privateIntegerInputBatch({v2, v1}, 1)),
              0));
      if (myID == 0) {
        testVectorEq(wire1, {v1 * v2, v1 * v1});
      }

      // Public Mult public
      auto wire2 =
          scheduler->getIntegerValueBatch(scheduler->publicMultPublicBatch(
              scheduler->publicIntegerInputBatch({v1, v1}),
              scheduler->publicIntegerInputBatch({v2, v1})));
      testVectorEq(wire2, {v1 * v2, v1 * v1});

      // Private Mult public
      auto wire3 = scheduler->getIntegerValueBatch(
          scheduler->openIntegerValueToPartyBatch(
              scheduler->privateMultPublicBatch(
                  scheduler->privateIntegerInputBatch({v1, v1}, 0),
                  scheduler->publicIntegerInputBatch({v2, v1})),
              1));
      if (myID == 1) {
        testVectorEq(wire3, {v1 * v2, v1 * v1});
      }
    }
  }

  auto gateCount = scheduler->getGateStatistics();
  EXPECT_EQ(gateCount.first, 24);
  EXPECT_EQ(gateCount.second, 64);
}

TEST_P(ArithmeticSchedulerTestFixture, testMultBatch) {
  runWithArithmeticScheduler(GetParam(), testMultBatch);
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

void testPlus(std::unique_ptr<IArithmeticScheduler> scheduler, int8_t myID) {
  for (auto v1 : {(uint64_t)1 << 63, (uint64_t)332}) {
    for (auto v2 : {(uint64_t)1 << 63, (uint64_t)89}) {
      // Private and private
      auto wire1 =
          scheduler->getIntegerValue(scheduler->openIntegerValueToParty(
              scheduler->privatePlusPrivate(
                  scheduler->privateIntegerInput(v1, 0),
                  scheduler->privateIntegerInput(v2, 1)),
              0));
      if (myID == 0) {
        EXPECT_EQ(wire1, (uint64_t)v1 + v2);
      }

      // Public and public
      auto wire2 = scheduler->publicPlusPublic(
          scheduler->publicIntegerInput(v1), scheduler->publicIntegerInput(v2));
      EXPECT_EQ(scheduler->getIntegerValue(wire2), (uint64_t)v1 + v2);

      // Private and public
      auto wire3 =
          scheduler->getIntegerValue(scheduler->openIntegerValueToParty(
              scheduler->privatePlusPublic(
                  scheduler->privateIntegerInput(v1, 0),
                  scheduler->publicIntegerInput(v2)),
              1));
      if (myID == 1) {
        EXPECT_EQ(wire3, (uint64_t)v1 + v2);
      }
    }
  }
  auto gateCount = scheduler->getGateStatistics();
  EXPECT_EQ(gateCount.first, 8);
  EXPECT_EQ(gateCount.second, 36);
}

TEST_P(ArithmeticSchedulerTestFixture, testPlus) {
  runWithArithmeticScheduler(GetParam(), testPlus);
}

void testPlusBatch(
    std::unique_ptr<IArithmeticScheduler> scheduler,
    int8_t myID) {
  for (uint64_t v1 : {(uint64_t)1 << 63, (uint64_t)332}) {
    for (uint64_t v2 : {(uint64_t)1 << 63, (uint64_t)89}) {
      // Private and private
      auto wire1 = scheduler->getIntegerValueBatch(
          scheduler->openIntegerValueToPartyBatch(
              scheduler->privatePlusPrivateBatch(
                  scheduler->privateIntegerInputBatch({v1, v1}, 0),
                  scheduler->privateIntegerInputBatch({v2, v1}, 1)),
              0));
      if (myID == 0) {
        testVectorEq(wire1, {v1 + v2, v1 + v1});
      }

      // Public and public
      auto wire2 =
          scheduler->getIntegerValueBatch(scheduler->publicPlusPublicBatch(
              scheduler->publicIntegerInputBatch({v1, v1}),
              scheduler->publicIntegerInputBatch({v2, v1})));
      testVectorEq(wire2, {v1 + v2, v1 + v1});

      // Private and public
      auto wire3 = scheduler->getIntegerValueBatch(
          scheduler->openIntegerValueToPartyBatch(
              scheduler->privatePlusPublicBatch(
                  scheduler->privateIntegerInputBatch({v1, v1}, 0),
                  scheduler->publicIntegerInputBatch({v2, v1})),
              1));
      if (myID == 1) {
        testVectorEq(wire3, {v1 + v2, v1 + v1});
      }
    }
  }

  auto gateCount = scheduler->getGateStatistics();
  EXPECT_EQ(gateCount.first, 16);
  EXPECT_EQ(gateCount.second, 72);
}

TEST_P(ArithmeticSchedulerTestFixture, testPlusBatch) {
  runWithArithmeticScheduler(GetParam(), testPlusBatch);
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

void testNeg(std::unique_ptr<IArithmeticScheduler> scheduler, int8_t myID) {
  for (auto v1 : {(uint64_t)-1231, (uint64_t)5765}) {
    // Neg private
    auto wire1 = scheduler->getIntegerValue(scheduler->openIntegerValueToParty(
        scheduler->negPrivate(scheduler->privateIntegerInput(v1, 1)), 0));
    if (myID == 0) {
      EXPECT_EQ(wire1, -v1);
    }

    // Neg public
    auto wire2 = scheduler->negPublic(scheduler->publicIntegerInput(v1));
    EXPECT_EQ(scheduler->getIntegerValue(wire2), -v1);
  }
  auto gateCount = scheduler->getGateStatistics();
  EXPECT_EQ(gateCount.first, 2);
  EXPECT_EQ(gateCount.second, 8);
}

TEST_P(ArithmeticSchedulerTestFixture, testNeg) {
  runWithArithmeticScheduler(GetParam(), testNeg);
}

void testNegBatch(
    std::unique_ptr<IArithmeticScheduler> scheduler,
    int8_t myID) {
  for (auto v1 : {(uint64_t)-1231, (uint64_t)5765}) {
    // Neg private
    auto wire1 =
        scheduler->getIntegerValueBatch(scheduler->openIntegerValueToPartyBatch(
            scheduler->negPrivateBatch(
                scheduler->privateIntegerInputBatch({v1, -v1}, 1)),
            0));
    if (myID == 0) {
      testVectorEq(wire1, {-v1, v1});
    }

    // Neg public
    auto wire2 = scheduler->negPublicBatch(
        scheduler->publicIntegerInputBatch({v1, -v1}));
    testVectorEq(scheduler->getIntegerValueBatch(wire2), {-v1, v1});
  }

  auto gateCount = scheduler->getGateStatistics();
  EXPECT_EQ(gateCount.first, 4);
  EXPECT_EQ(gateCount.second, 16);
}

TEST_P(ArithmeticSchedulerTestFixture, testNegBatch) {
  runWithArithmeticScheduler(GetParam(), testNegBatch);
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

void testMultipleArithmeticOperations(
    std::unique_ptr<IArithmeticScheduler> scheduler,
    int8_t myID) {
  auto v1 = scheduler->privateIntegerInput(321, 0);
  auto v2 = scheduler->privateIntegerInput(123, 0);

  auto v3 = scheduler->privatePlusPrivate(v1, v2); // 444
  auto v4 = scheduler->privateMultPrivate(v3, v1); // 142524
  auto v5 = scheduler->negPrivate(v4); // -142524

  auto revealed1 =
      scheduler->getIntegerValue(scheduler->openIntegerValueToParty(v5, 0));
  if (myID == 0) {
    EXPECT_EQ(revealed1, (uint64_t)-142524);
  }

  auto v6 = scheduler->publicIntegerInput(235);

  auto v7 = scheduler->negPublic(v6); // -235
  auto v8 = scheduler->privateMultPublic(v1, v7); // -75435
  auto v9 = scheduler->privatePlusPublic(v2, v7); // -112

  auto revealed2 =
      scheduler->getIntegerValue(scheduler->openIntegerValueToParty(v8, 1));
  auto revealed3 =
      scheduler->getIntegerValue(scheduler->openIntegerValueToParty(v9, 1));
  if (myID == 1) {
    EXPECT_EQ(revealed2, (uint64_t)-75435);
    EXPECT_EQ(revealed3, (uint64_t)-112);
  }

  auto v10 = scheduler->publicIntegerInput(596854);

  auto v11 = scheduler->publicMultPublic(v10, v6); // 140260690
  auto v12 = scheduler->publicPlusPublic(v11, v10); // 140857544
  auto v13 = scheduler->publicMultPublic(v12, v7); // -235 * 140857544

  EXPECT_EQ(scheduler->getIntegerValue(v13), (uint64_t)(-235) * 140857544);

  auto v14 = scheduler->privateIntegerInput(888, 1);
  auto v15 = scheduler->privateMultPrivate(v14, v1); // 285048
  auto v16 = scheduler->negPrivate(v15); // -285048
  auto v17 = scheduler->privatePlusPrivate(v1, v14); // 1209
  auto v18 = scheduler->privateMultPrivate(v16, v17); // -344623032
  auto v19 = scheduler->privatePlusPrivate(v18, v15); // -344337984

  auto revealed4 =
      scheduler->getIntegerValue(scheduler->openIntegerValueToParty(v19, 0));
  auto revealed5 =
      scheduler->getIntegerValue(scheduler->openIntegerValueToParty(v18, 0));
  if (myID == 0) {
    EXPECT_EQ(revealed4, (uint64_t)-344337984);
    EXPECT_EQ(revealed5, (uint64_t)-344623032);
  }
}

TEST_P(ArithmeticSchedulerTestFixture, testMultipleOperations) {
  runWithArithmeticScheduler(GetParam(), testMultipleArithmeticOperations);
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

void testArithmeticReferenceCount(
    std::unique_ptr<IArithmeticScheduler> scheduler,
    int8_t /*myId*/) {
  auto wire = scheduler->privateIntegerInput(21321, 0);

  for (int i = 0; i < 10; i++) {
    if (i % 3 == 2) {
      scheduler->decreaseReferenceCount(wire);
    } else {
      scheduler->increaseReferenceCount(wire);
    }
    EXPECT_NO_THROW(scheduler->getIntegerValue(wire));
  }

  for (int i = 0; i < 10; i++) {
    EXPECT_NO_THROW(scheduler->getIntegerValue(wire));
    if (i % 3 != 2) {
      scheduler->decreaseReferenceCount(wire);
    } else {
      scheduler->increaseReferenceCount(wire);
    }
  }

  testPairEq(scheduler->getWireStatistics(), {1, 0});

  scheduler->decreaseReferenceCount(wire);
  EXPECT_THROW(scheduler->getIntegerValue(wire), std::runtime_error);

  testPairEq(scheduler->getWireStatistics(), {1, 1});
}

TEST_P(ArithmeticSchedulerTestFixture, testReferenceCount) {
  runWithArithmeticScheduler(GetParam(), testArithmeticReferenceCount);
}

void testArithmeticReferenceCountBatch(
    std::unique_ptr<IArithmeticScheduler> scheduler,
    int8_t /*myId*/) {
  auto wire = scheduler->privateIntegerInputBatch({123, (uint64_t)-324}, 0);

  for (int i = 0; i < 10; i++) {
    if (i % 3 == 2) {
      scheduler->decreaseReferenceCountBatch(wire);
    } else {
      scheduler->increaseReferenceCountBatch(wire);
    }
    EXPECT_NO_THROW(scheduler->getIntegerValueBatch(wire));
  }

  for (int i = 0; i < 10; i++) {
    EXPECT_NO_THROW(scheduler->getIntegerValueBatch(wire));
    if (i % 3 != 2) {
      scheduler->decreaseReferenceCountBatch(wire);
    } else {
      scheduler->increaseReferenceCountBatch(wire);
    }
  }

  testPairEq(scheduler->getWireStatistics(), {1, 0});

  scheduler->decreaseReferenceCountBatch(wire);
  EXPECT_THROW(scheduler->getIntegerValueBatch(wire), std::runtime_error);

  testPairEq(scheduler->getWireStatistics(), {1, 1});
}

TEST_P(ArithmeticSchedulerTestFixture, testReferenceCountBatch) {
  runWithArithmeticScheduler(GetParam(), testArithmeticReferenceCountBatch);
}

void testBatchingAndUnbatching(
    std::unique_ptr<IScheduler> scheduler,
    int8_t myId) {
  auto wire1 = scheduler->privateBooleanInputBatch({false, true}, 0);
  auto wire2 = scheduler->privateBooleanInputBatch({true, false, false}, 1);
  auto wire3 = scheduler->batchingUp({wire1, wire2});
  auto revealed3 = scheduler->getBooleanValueBatch(
      scheduler->openBooleanValueToPartyBatch(wire3, 0));
  if (myId == 0) {
    testVectorEq(revealed3, {false, true, true, false, false});
  }
  auto wire45 = scheduler->unbatching(
      wire3,
      std::make_shared<std::vector<uint32_t>>(std::vector<uint32_t>({3, 1})));
  auto revealed4 = scheduler->getBooleanValueBatch(
      scheduler->openBooleanValueToPartyBatch(wire45.at(0), 0));
  auto revealed5 = scheduler->getBooleanValueBatch(
      scheduler->openBooleanValueToPartyBatch(wire45.at(1), 0));
  if (myId == 0) {
    testVectorEq(revealed4, {false, true, true});
    testVectorEq(revealed5, {false});
  }
}

TEST_P(SchedulerTestFixture, testBatchingAndUnbatching) {
  runWithScheduler(GetParam(), testBatchingAndUnbatching);
}

class CompositeSchedulerTestFixture
    : public ::testing::TestWithParam<std::tuple<SchedulerType, size_t>> {};

INSTANTIATE_TEST_SUITE_P(
    SchedulerTest,
    CompositeSchedulerTestFixture,
    ::testing::Combine(
        ::testing::Values(
            SchedulerType::Plaintext,
            SchedulerType::NetworkPlaintext,
            SchedulerType::Lazy,
            SchedulerType::Eager),
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

} // namespace fbpcf::scheduler
