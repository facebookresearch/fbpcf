/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/mpc_framework/engine/tuple_generator/TupleGenerator.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <future>
#include <memory>
#include <thread>

#include "fbpcf/mpc_framework/engine/tuple_generator/DummyProductShareGeneratorFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/ITupleGenerator.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/test/TupleGeneratorTestHelper.h"

namespace fbpcf::mpc_framework::engine::tuple_generator {

using TupleGeneratorFactoryCreator =
    std::unique_ptr<tuple_generator::ITupleGeneratorFactory>(
        int numberOfParty,
        int myId,
        communication::IPartyCommunicationAgentFactory& agentFactory);

void testTupleGenerator(
    int numberOfParty,
    TupleGeneratorFactoryCreator creator) {
  auto agentFactories = communication::getInMemoryAgentFactory(numberOfParty);

  auto task =
      [](TupleGeneratorFactoryCreator creator,
         int numberOfParty,
         int myId,
         std::reference_wrapper<communication::IPartyCommunicationAgentFactory>
             agentFactory,
         int size) {
        auto generator = creator(numberOfParty, myId, agentFactory)->create();
        return generator->getBooleanTuple(size);
      };

  // this size is larger than the AES and tuple generator buffer size so we can
  // test regeneration.
  int size = kTestBufferSize * 4;

  std::vector<std::future<std::vector<ITupleGenerator::BooleanTuple>>> futures;
  for (int i = 0; i < numberOfParty; i++) {
    futures.push_back(std::async(
        task,
        creator,
        numberOfParty,
        i,
        std::reference_wrapper<communication::IPartyCommunicationAgentFactory>(
            *agentFactories.at(i)),
        size));
  }

  std::vector<std::vector<ITupleGenerator::BooleanTuple>> results;
  for (int i = 0; i < numberOfParty; i++) {
    results.push_back(futures[i].get());
  }

  for (int i = 0; i < size; i++) {
    bool a = false;
    bool b = false;
    bool c = false;
    for (int j = 0; j < numberOfParty; j++) {
      a ^= results[j][i].getA();
      b ^= results[j][i].getB();
      c ^= results[j][i].getC();
    }
    EXPECT_EQ(c, a & b);
  }
}

TEST(TupleGeneratorTest, testDummyTupleGenerator) {
  int numberOfParty = 4;

  testTupleGenerator(numberOfParty, createDummyTupleGeneratorFactory);
}

TEST(TupleGeneratorTest, testWithDummyProductShareGenerator) {
  int numberOfParty = 4;

  testTupleGenerator(
      numberOfParty,

      createInMemoryTupleGeneratorFactoryWithDummyProductShareGenerator);
}

TEST(TupleGeneratorTest, testWithSecureProductShareGenerator) {
  int numberOfParty = 4;

  testTupleGenerator(
      numberOfParty,
      createInMemoryTupleGeneratorFactoryWithRealProductShareGenerator);
}

TEST(TupleGeneratorTest, testTwoPartyTupleGeneratorWithDummyRcot) {
  testTupleGenerator(2, createTwoPartyTupleGeneratorFactoryWithDummyRcot);
}

TEST(TupleGeneratorTest, testTwoPartyTupleGeneratorWithRealOt) {
  testTupleGenerator(2, createTwoPartyTupleGeneratorFactoryWithRealOt);
}

TEST(TupleGeneratorTest, testTwoPartyTupleGeneratorWithRcotExtender) {
  testTupleGenerator(2, createTwoPartyTupleGeneratorFactoryWithRcotExtender);
}

} // namespace fbpcf::mpc_framework::engine::tuple_generator
