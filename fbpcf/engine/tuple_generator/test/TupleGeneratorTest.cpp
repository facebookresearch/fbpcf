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
         std::unordered_map<size_t, uint32_t> compositeTupleSizes) {
        auto generator = creator(numberOfParty, myId, agentFactory)->create();
        if constexpr (testCompositeTuples) {
          return generator->getNormalAndCompositeBooleanTuples(
              tupleSize, compositeTupleSizes);
        } else {
          auto tuples = generator->getBooleanTuple(tupleSize);
          return std::make_pair(
              std::move(tuples),
              std::unordered_map<
                  size_t,
                  std::vector<ITupleGenerator::CompositeBooleanTuple>>());
        }
      };

  // this size is larger than the AES and tuple generator buffer size so we can
  // test regeneration.
  uint32_t tupleSize = kTestBufferSize * 4;
  std::unordered_map<size_t, uint32_t> compositeTuplesSizes(
      {{8, tupleSize},
       {32, tupleSize},
       {64, tupleSize},
       {128, tupleSize},
       {50, tupleSize},
       {31, tupleSize}});

  std::vector<std::future<std::pair<
      std::vector<ITupleGenerator::BooleanTuple>,
      std::unordered_map<
          size_t,
          std::vector<ITupleGenerator::CompositeBooleanTuple>>>>>
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

  std::vector<std::pair<
      std::vector<ITupleGenerator::BooleanTuple>,
      std::unordered_map<
          size_t,
          std::vector<ITupleGenerator::CompositeBooleanTuple>>>>
      results;
  for (int i = 0; i < numberOfParty; i++) {
    results.push_back(futures[i].get());
  }

  for (int i = 0; i < tupleSize; i++) {
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
    for (auto& compositeSizeToCount : compositeTuplesSizes) {
      size_t compositeSize = compositeSizeToCount.first;
      uint32_t tupleCount = compositeSizeToCount.second;

      for (int j = 0; j < numberOfParty; j++) {
        ASSERT_EQ(std::get<1>(results[j]).at(compositeSize).size(), tupleCount);
      }

      for (int i = 0; i < tupleCount; i++) {
        std::vector<bool> a(compositeSize);
        bool b = false;
        std::vector<bool> c(compositeSize);

        for (int j = 0; j < numberOfParty; j++) {
          b ^= std::get<1>(results[j]).at(compositeSize)[i].getB();
          auto aShares = std::get<1>(results[j]).at(compositeSize)[i].getA();
          auto cShares = std::get<1>(results[j]).at(compositeSize)[i].getC();

          ASSERT_EQ(aShares.size(), compositeSize);
          ASSERT_EQ(cShares.size(), compositeSize);
          for (int k = 0; k < compositeSize; k++) {
            a.at(k) = a.at(k) ^ aShares[k];
            c.at(k) = c.at(k) ^ aShares[k];
          }
        }

        for (int k = 0; k < compositeSize; k++) {
          ASSERT_EQ(c[k], a[k] & b);
        }
      }
    }
  }
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
  testTupleGenerator<false>(
      2, createTwoPartyTupleGeneratorFactoryWithDummyRcot);
}

TEST(TupleGeneratorTest, testTwoPartyTupleGeneratorWithRealOt) {
  testTupleGenerator<false>(2, createTwoPartyTupleGeneratorFactoryWithRealOt);
}

TEST(TupleGeneratorTest, testTwoPartyTupleGeneratorWithRcotExtender) {
  testTupleGenerator<false>(
      2, createTwoPartyTupleGeneratorFactoryWithRcotExtender);
}

} // namespace fbpcf::engine::tuple_generator
