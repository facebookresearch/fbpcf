/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/engine/tuple_generator/TupleGenerator.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <future>
#include <map>
#include <memory>
#include <thread>

#include "fbpcf/engine/tuple_generator/DummyProductShareGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/ITupleGenerator.h"
#include "fbpcf/engine/tuple_generator/test/TupleGeneratorTestHelper.h"

namespace fbpcf::engine::tuple_generator {

using TupleGeneratorFactoryCreator =
    std::unique_ptr<tuple_generator::ITupleGeneratorFactory>(
        int numberOfParty,
        int myId,
        communication::IPartyCommunicationAgentFactory& agentFactory);

template <bool testCompositeTuples>
void assertResults(
    int numberOfParty,
    std::vector<std::future<std::pair<
        std::vector<ITupleGenerator::BooleanTuple>,
        std::
            map<size_t, std::vector<ITupleGenerator::CompositeBooleanTuple>>>>>&
        futures,
    int expectedTupleCount,
    std::map<size_t, uint32_t> expectedCompositeTuplesSizes) {
  std::vector<std::pair<
      std::vector<ITupleGenerator::BooleanTuple>,
      std::map<size_t, std::vector<ITupleGenerator::CompositeBooleanTuple>>>>
      results;
  for (int i = 0; i < numberOfParty; i++) {
    results.push_back(futures[i].get());
  }

  for (int i = 0; i < expectedTupleCount; i++) {
    bool a = false;
    bool b = false;
    bool c = false;
    for (int j = 0; j < numberOfParty; j++) {
      a ^= std::get<0>(results[j])[i].getA();
      b ^= std::get<0>(results[j])[i].getB();
      c ^= std::get<0>(results[j])[i].getC();
    }
    EXPECT_EQ(c, a & b);
  }

  if constexpr (testCompositeTuples) {
    for (auto& compositeSizeToCount : expectedCompositeTuplesSizes) {
      size_t compositeSize = compositeSizeToCount.first;
      uint32_t tupleCount = compositeSizeToCount.second;

      for (int j = 0; j < numberOfParty; j++) {
        ASSERT_EQ(std::get<1>(results[j]).at(compositeSize).size(), tupleCount);
      }

      for (int i = 0; i < tupleCount; i++) {
        bool a = false;
        std::vector<bool> b(compositeSize);
        std::vector<bool> c(compositeSize);

        for (int j = 0; j < numberOfParty; j++) {
          a ^= std::get<1>(results[j]).at(compositeSize)[i].getA();
          auto bShares = std::get<1>(results[j]).at(compositeSize)[i].getB();
          auto cShares = std::get<1>(results[j]).at(compositeSize)[i].getC();

          ASSERT_EQ(bShares.size(), compositeSize);
          ASSERT_EQ(cShares.size(), compositeSize);
          for (int k = 0; k < compositeSize; k++) {
            b.at(k) = b.at(k) ^ bShares[k];
            c.at(k) = c.at(k) ^ cShares[k];
          }
        }

        for (int k = 0; k < compositeSize; k++) {
          ASSERT_EQ(c[k], a & b[k]);
        }
      }
    }
  }
}

template <bool testCompositeTuples>
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
         uint32_t tupleSize,
         std::map<size_t, uint32_t> compositeTupleSizes) {
        auto generator = creator(numberOfParty, myId, agentFactory)->create();
        if constexpr (testCompositeTuples) {
          return generator->getNormalAndCompositeBooleanTuples(
              tupleSize, compositeTupleSizes);
        } else {
          auto tuples = generator->getBooleanTuple(tupleSize);
          return std::make_pair(
              std::move(tuples),
              std::map<
                  size_t,
                  std::vector<ITupleGenerator::CompositeBooleanTuple>>());
        }
      };

  // this size is larger than the AES and tuple generator buffer size so we can
  // test regeneration.
  uint32_t tupleSize = kTestBufferSize * 4;
  std::map<size_t, uint32_t> compositeTuplesSizes(
      {{8, tupleSize},
       {32, tupleSize},
       {64, tupleSize},
       {128, tupleSize},
       {50, tupleSize},
       {31, tupleSize},
       {256, tupleSize}});

  std::vector<std::future<std::pair<
      std::vector<ITupleGenerator::BooleanTuple>,
      std::map<size_t, std::vector<ITupleGenerator::CompositeBooleanTuple>>>>>
      futures;
  for (int i = 0; i < numberOfParty; i++) {
    futures.push_back(std::async(
        task,
        creator,
        numberOfParty,
        i,
        std::reference_wrapper<communication::IPartyCommunicationAgentFactory>(
            *agentFactories.at(i)),
        tupleSize,
        compositeTuplesSizes));
  }
  assertResults<testCompositeTuples>(
      numberOfParty, futures, tupleSize, compositeTuplesSizes);
}

TEST(TupleGeneratorTest, testDummyTupleGenerator) {
  int numberOfParty = 4;

  testTupleGenerator<true>(numberOfParty, createDummyTupleGeneratorFactory);
}

TEST(TupleGeneratorTest, testWithDummyProductShareGenerator) {
  int numberOfParty = 4;

  testTupleGenerator<false>(
      numberOfParty, createTupleGeneratorFactoryWithDummyProductShareGenerator);
}

TEST(TupleGeneratorTest, testWithSecureProductShareGenerator) {
  int numberOfParty = 4;

  testTupleGenerator<false>(
      numberOfParty, createTupleGeneratorFactoryWithRealProductShareGenerator);
}

TEST(TupleGeneratorTest, testTwoPartyTupleGeneratorWithDummyRcot) {
  testTupleGenerator<true>(2, createTwoPartyTupleGeneratorFactoryWithDummyRcot);
}

TEST(TupleGeneratorTest, testTwoPartyTupleGeneratorWithRealOt) {
  testTupleGenerator<true>(2, createTwoPartyTupleGeneratorFactoryWithRealOt);
}

TEST(TupleGeneratorTest, testTwoPartyTupleGeneratorWithRcotExtender) {
  testTupleGenerator<true>(
      2, createTwoPartyTupleGeneratorFactoryWithRcotExtender);
}

void testTupleGeneratorThreadSynchronization(
    int numberOfParty,
    TupleGeneratorFactoryCreator creator) {
  auto agentFactories = communication::getInMemoryAgentFactory(numberOfParty);

  auto task = [](TupleGeneratorFactoryCreator creator,
                 int numberOfParty,
                 int myId,
                 std::reference_wrapper<
                     communication::IPartyCommunicationAgentFactory>
                     agentFactory,
                 uint32_t tupleSize,
                 std::map<size_t, uint32_t> compositeTupleSizes) {
    auto generator = creator(numberOfParty, myId, agentFactory)->create();
    std::vector<ITupleGenerator::BooleanTuple> normalResults;
    std::vector<ITupleGenerator::CompositeBooleanTuple> compositeResults;
    for (size_t i = 0; i < kTestBufferSize; i++) {
      auto tuples = generator->getNormalAndCompositeBooleanTuples(
          tupleSize, compositeTupleSizes);

      normalResults.insert(
          normalResults.end(),
          std::get<0>(tuples).begin(),
          std::get<0>(tuples).end());
      compositeResults.insert(
          compositeResults.end(),
          std::get<1>(tuples).at(8).begin(),
          std::get<1>(tuples).at(8).end());
    }

    return std::make_pair<
        std::vector<ITupleGenerator::BooleanTuple>,
        std::map<size_t, std::vector<ITupleGenerator::CompositeBooleanTuple>>>(
        std::move(normalResults), {{8, compositeResults}});
  };

  // this size is larger than the AES and tuple generator buffer size so we can
  // test regeneration.
  uint32_t tupleSize = 1;
  std::map<size_t, uint32_t> compositeTupleSizes({{8, tupleSize}});

  std::vector<std::future<std::pair<
      std::vector<ITupleGenerator::BooleanTuple>,
      std::map<size_t, std::vector<ITupleGenerator::CompositeBooleanTuple>>>>>
      futures;
  for (int i = 0; i < numberOfParty; i++) {
    futures.push_back(std::async(
        task,
        creator,
        numberOfParty,
        i,
        std::reference_wrapper<communication::IPartyCommunicationAgentFactory>(
            *agentFactories.at(i)),
        tupleSize,
        compositeTupleSizes));
  }

  assertResults<true>(
      numberOfParty, futures, kTestBufferSize, {{8, kTestBufferSize}});
}

TEST(
    TupleGeneratorTest,
    testTwoPartyTupleGeneratorSynchronizationWithRcotExtender) {
  testTupleGeneratorThreadSynchronization(
      2, createTwoPartyTupleGeneratorFactoryWithRcotExtenderAndSmallBuffer);
}

} // namespace fbpcf::engine::tuple_generator
