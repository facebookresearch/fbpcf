/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <cstdint>
#include <functional>
#include <future>
#include <memory>
#include <random>
#include <thread>
#include <utility>
#include <vector>

#include "fbpcf/engine/DummySecretShareEngine.h"
#include "fbpcf/engine/DummySecretShareEngineFactory.h"
#include "fbpcf/engine/ISecretShareEngine.h"
#include "fbpcf/engine/SecretShareEngineFactory.h"
#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/util/MetricCollector.h"

namespace fbpcf::engine {

template <typename T>
T testHelper(
    int numberOfParty,
    std::function<
        T(std::unique_ptr<ISecretShareEngine> engine,
          int myId,
          int numberOfParty)> test,
    std::function<std::unique_ptr<ISecretShareEngineFactory>(
        int myId,
        int numberOfParty,
        communication::IPartyCommunicationAgentFactory& agentFactory,
        std::shared_ptr<fbpcf::util::MetricCollector> metricCollector)>
        engineFactory,
    void (*assertPartyResultsConsistent)(T base, T comparison)) {
  auto agentFactories = communication::getInMemoryAgentFactory(numberOfParty);

  std::vector<std::future<T>> futures;
  for (auto i = 0; i < numberOfParty; ++i) {
    auto partyMetricCollector = std::make_shared<fbpcf::util::MetricCollector>(
        "metric_collector_party_i");
    futures.push_back(std::async(
        [i, numberOfParty, test, engineFactory](
            std::reference_wrapper<
                communication::IPartyCommunicationAgentFactory> agentFactory,
            std::shared_ptr<fbpcf::util::MetricCollector> metricCollector) {
          auto engine =
              engineFactory(i, numberOfParty, agentFactory, metricCollector)
                  ->create();
          return test(std::move(engine), i, numberOfParty);
        },
        std::reference_wrapper<communication::IPartyCommunicationAgentFactory>(
            *agentFactories.at(i)),
        partyMetricCollector));
  }
  auto rst = futures.at(0).get();
  for (auto i = 1; i < numberOfParty; ++i) {
    auto tmp = futures.at(i).get();
    assertPartyResultsConsistent(rst, tmp);
  }

  return rst;
}

void assertPartyResultsConsistent(
    std::vector<bool> base,
    std::vector<bool> comparison) {
  EXPECT_EQ(comparison.size(), base.size());
}

void assertPartyResultsConsistent(
    std::vector<uint64_t> base,
    std::vector<uint64_t> comparison) {
  EXPECT_EQ(comparison.size(), base.size());
}

void assertPartyResultsConsistent(
    std::pair<std::vector<bool>, std::vector<std::vector<bool>>> base,
    std::pair<std::vector<bool>, std::vector<std::vector<bool>>> comparison) {
  auto expectedAndResult = std::get<0>(base);
  auto expectedCompositeAndResult = std::get<1>(base);

  auto andResult = std::get<0>(comparison);
  EXPECT_EQ(andResult.size(), expectedAndResult.size());

  auto compositeResult = std::get<1>(comparison);
  EXPECT_EQ(compositeResult.size(), expectedCompositeAndResult.size());
  for (int j = 0; j < compositeResult.size(); j++) {
    EXPECT_EQ(compositeResult[j].size(), expectedCompositeAndResult[j].size());
  }
}

void assertPartyResultsConsistent(
    std::tuple<
        std::vector<bool>,
        std::vector<std::vector<bool>>,
        std::vector<uint64_t>> base,
    std::tuple<
        std::vector<bool>,
        std::vector<std::vector<bool>>,
        std::vector<uint64_t>> comparison) {
  auto expectedAndResult = std::get<0>(base);
  auto expectedCompositeAndResult = std::get<1>(base);
  auto expectedMultResult = std::get<2>(base);

  auto andResult = std::get<0>(comparison);
  EXPECT_EQ(andResult.size(), expectedAndResult.size());

  auto multResult = std::get<2>(comparison);
  EXPECT_EQ(multResult.size(), expectedMultResult.size());

  auto compositeResult = std::get<1>(comparison);
  EXPECT_EQ(compositeResult.size(), expectedCompositeAndResult.size());
  for (int j = 0; j < compositeResult.size(); j++) {
    EXPECT_EQ(compositeResult[j].size(), expectedCompositeAndResult[j].size());
  }
}
std::function<std::vector<bool>(
    std::unique_ptr<ISecretShareEngine> engine,
    int myId,
    int numberOfParty)>
testTemplate(
    const std::vector<std::pair<bool, int>>& inputsArrangement,
    std::vector<bool>
        testBody(ISecretShareEngine& engine, const std::vector<bool>&),
    bool resultNeedsOpen = true) {
  return [inputsArrangement, resultNeedsOpen, testBody](
             std::unique_ptr<ISecretShareEngine> engine,
             int myId,
             int numberOfParty) {
    std::vector<bool> inputs;
    for (size_t i = 0; i < inputsArrangement.size(); i++) {
      if (inputsArrangement[i].second < numberOfParty) {
        // a private value
        if (myId == inputsArrangement[i].second) {
          inputs.push_back(engine->setInput(
              inputsArrangement[i].second, inputsArrangement[i].first));
        } else {
          inputs.push_back(engine->setInput(inputsArrangement[i].second));
        }
      } else {
        // a public value
        inputs.push_back(inputsArrangement[i].first);
      }
    }
    auto outputs = testBody(*engine, inputs);
    if (resultNeedsOpen) {
      return engine->revealToParty(0, outputs);
    } else {
      return outputs;
    }
  };
}

std::function<std::vector<uint64_t>(
    std::unique_ptr<ISecretShareEngine> engine,
    int myId,
    int numberOfParty)>
testTemplate(
    const std::vector<std::pair<uint64_t, int>>& inputsArrangement,
    std::vector<uint64_t>
        testBody(ISecretShareEngine& engine, const std::vector<uint64_t>&),
    bool resultNeedsOpen = true) {
  return [inputsArrangement, resultNeedsOpen, testBody](
             std::unique_ptr<ISecretShareEngine> engine,
             int myId,
             int numberOfParty) {
    std::vector<uint64_t> inputs;
    for (size_t i = 0; i < inputsArrangement.size(); i++) {
      if (inputsArrangement[i].second < numberOfParty) {
        // a private value
        if (myId == inputsArrangement[i].second) {
          inputs.push_back(engine->setIntegerInput(
              inputsArrangement[i].second, inputsArrangement[i].first));
        } else {
          inputs.push_back(
              engine->setIntegerInput(inputsArrangement[i].second));
        }
      } else {
        // a public value
        inputs.push_back(inputsArrangement[i].first);
      }
    }
    auto outputs = testBody(*engine, inputs);
    if (resultNeedsOpen) {
      return engine->revealToParty(0, outputs);
    } else {
      return outputs;
    }
  };
}

std::function<std::pair<std::vector<bool>, std::vector<std::vector<bool>>>(
    std::unique_ptr<ISecretShareEngine> engine,
    int myId,
    int numberOfParty)>
testTemplate(
    const std::vector<std::pair<bool, int>>& inputsArrangement,
    std::pair<std::vector<bool>, std::vector<std::vector<bool>>>
        testBody(ISecretShareEngine& engine, const std::vector<bool>&),
    bool resultNeedsOpen = true) {
  return [inputsArrangement, resultNeedsOpen, testBody](
             std::unique_ptr<ISecretShareEngine> engine,
             int myId,
             int numberOfParty) {
    std::vector<bool> inputs;
    for (int i = 0; i < inputsArrangement.size(); i++) {
      if (inputsArrangement[i].second < numberOfParty) {
        // a private value
        if (myId == inputsArrangement[i].second) {
          inputs.push_back(engine->setInput(
              inputsArrangement[i].second, inputsArrangement[i].first));
        } else {
          inputs.push_back(engine->setInput(inputsArrangement[i].second));
        }
      } else {
        // a public value
        inputs.push_back(inputsArrangement[i].first);
      }
    }
    auto outputs = testBody(*engine, inputs);
    if (resultNeedsOpen) {
      auto andResults = engine->revealToParty(0, std::get<0>(outputs));
      std::vector<std::vector<bool>> compositeANDResults = std::get<1>(outputs);
      for (int i = 0; i < compositeANDResults.size(); i++) {
        compositeANDResults[i] =
            engine->revealToParty(0, compositeANDResults[i]);
      }
      return std::make_pair(andResults, compositeANDResults);
    } else {
      return outputs;
    }
  };
}

std::function<std::tuple<
    std::vector<bool>,
    std::vector<std::vector<bool>>,
    std::vector<uint64_t>>(
    std::unique_ptr<ISecretShareEngine> engine,
    int myId,
    int numberOfParty)>
testTemplate(
    const std::vector<std::pair<bool, int>>& boolInputsArrangement,
    const std::vector<std::pair<uint64_t, int>>& intInputsArrangement,
    std::tuple<
        std::vector<bool>,
        std::vector<std::vector<bool>>,
        std::vector<uint64_t>>
        testBody(
            ISecretShareEngine& engine,
            const std::vector<bool>&,
            const std::vector<uint64_t>&),
    bool resultNeedsOpen = true) {
  return [boolInputsArrangement,
          intInputsArrangement,
          resultNeedsOpen,
          testBody](
             std::unique_ptr<ISecretShareEngine> engine,
             int myId,
             int numberOfParty) {
    std::vector<bool> boolInputs;
    for (int i = 0; i < boolInputsArrangement.size(); i++) {
      if (boolInputsArrangement[i].second < numberOfParty) {
        // a private value
        if (myId == boolInputsArrangement[i].second) {
          boolInputs.push_back(engine->setInput(
              boolInputsArrangement[i].second, boolInputsArrangement[i].first));
        } else {
          boolInputs.push_back(
              engine->setInput(boolInputsArrangement[i].second));
        }
      } else {
        // a public value
        boolInputs.push_back(boolInputsArrangement[i].first);
      }
    }

    std::vector<uint64_t> intInputs;
    for (size_t i = 0; i < intInputsArrangement.size(); i++) {
      if (intInputsArrangement[i].second < numberOfParty) {
        // a private value
        if (myId == intInputsArrangement[i].second) {
          intInputs.push_back(engine->setIntegerInput(
              intInputsArrangement[i].second, intInputsArrangement[i].first));
        } else {
          intInputs.push_back(
              engine->setIntegerInput(intInputsArrangement[i].second));
        }
      } else {
        // a public value
        intInputs.push_back(intInputsArrangement[i].first);
      }
    }
    auto outputs = testBody(*engine, boolInputs, intInputs);
    if (resultNeedsOpen) {
      auto andResults = engine->revealToParty(0, std::get<0>(outputs));
      std::vector<std::vector<bool>> compositeANDResults = std::get<1>(outputs);
      for (int i = 0; i < compositeANDResults.size(); i++) {
        compositeANDResults[i] =
            engine->revealToParty(0, compositeANDResults[i]);
      }
      auto multResults = engine->revealToParty(0, std::get<2>(outputs));
      return std::make_tuple(andResults, compositeANDResults, multResults);
    } else {
      return outputs;
    }
  };
}

std::vector<std::pair<bool, int>>
generateRandomInputs(int numberOfParty, int size, int startOfPublicValues) {
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint8_t> binaryDist(0, 1);
  std::uniform_int_distribution<uint8_t> partyDist(0, numberOfParty - 1);

  std::vector<std::pair<bool, int>> rst(size);
  for (int i = 0; i < size; i++) {
    rst[i] = {binaryDist(e), partyDist(e)};
  }

  for (int i = startOfPublicValues; i < size; i++) {
    rst[i].second = numberOfParty + 1;
  }

  return rst;
}

std::vector<std::pair<uint64_t, int>> generateRandomIntegerInputs(
    int numberOfParty,
    int size,
    int startOfPublicValues) {
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint64_t> dist;
  std::uniform_int_distribution<uint8_t> partyDist(0, numberOfParty - 1);

  std::vector<std::pair<uint64_t, int>> rst(size);
  for (int i = 0; i < size; i++) {
    rst[i] = {dist(e), partyDist(e)};
  }

  for (int i = startOfPublicValues; i < size; i++) {
    rst[i].second = numberOfParty + 1;
  }

  return rst;
}

std::vector<bool> inputAndOutputTestBody(
    ISecretShareEngine& /*engine*/,
    const std::vector<bool>& inputs) {
  return inputs;
}

TEST(SecretShareEngineTest, TestInputAndOutputWithDummyComponents) {
  int numberOfParty = 4;
  int size = 16384;
  auto inputs = generateRandomInputs(numberOfParty, size, size);

  auto rst = testHelper(
      numberOfParty,
      testTemplate(inputs, inputAndOutputTestBody),
      getInsecureEngineFactoryWithDummyTupleGenerator,
      assertPartyResultsConsistent);
  ASSERT_EQ(rst.size(), size);
  for (int i = 0; i < size; i++) {
    EXPECT_EQ(rst[i], inputs[i].first);
  }
}

std::vector<uint64_t> inputAndOutputTestBody(
    ISecretShareEngine& /*engine*/,
    const std::vector<uint64_t>& inputs) {
  return inputs;
}

TEST(SecretShareEngineTest, TestIntegerInputAndOutputWithDummyComponents) {
  int numberOfParty = 4;
  int size = 16384;
  auto inputs = generateRandomIntegerInputs(numberOfParty, size, size);

  auto rst = testHelper(
      numberOfParty,
      testTemplate(inputs, inputAndOutputTestBody),
      getInsecureEngineFactoryWithDummyTupleGenerator,
      assertPartyResultsConsistent);
  ASSERT_EQ(rst.size(), size);
  for (int i = 0; i < size; i++) {
    EXPECT_EQ(rst[i], inputs[i].first);
  }
}

std::vector<bool> symmetricNOTTestBody(
    ISecretShareEngine& engine,
    const std::vector<bool>& inputs) {
  std::vector<bool> rst(inputs.size());
  for (int i = 0; i < inputs.size(); i++) {
    rst[i] = engine.computeSymmetricNOT(inputs[i]);
  }
  return rst;
}

std::vector<bool> asymmetricNOTTestBody(
    ISecretShareEngine& engine,
    const std::vector<bool>& inputs) {
  std::vector<bool> rst(inputs.size());
  for (size_t i = 0; i < inputs.size(); i++) {
    rst[i] = engine.computeAsymmetricNOT(inputs[i]);
  }
  return rst;
}

TEST(SecretShareEngineTest, TestNOTWithDummyComponents) {
  int numberOfParty = 4;
  int size = 16384;
  auto inputs1 = generateRandomInputs(numberOfParty, size, 0);
  auto inputs2 = generateRandomInputs(numberOfParty, size, size);
  auto rst1 = testHelper(
      numberOfParty,
      testTemplate(inputs1, symmetricNOTTestBody, false),
      getInsecureEngineFactoryWithDummyTupleGenerator,
      assertPartyResultsConsistent);
  auto rst2 = testHelper(
      numberOfParty,
      testTemplate(inputs2, asymmetricNOTTestBody),
      getInsecureEngineFactoryWithDummyTupleGenerator,
      assertPartyResultsConsistent);
  EXPECT_EQ(rst1.size(), size);
  EXPECT_EQ(rst2.size(), size);
  for (int i = 0; i < size; i++) {
    EXPECT_EQ(rst1[i], !inputs1[i].first);
    EXPECT_EQ(rst2[i], !inputs2[i].first);
  }
}

std::vector<bool> batchSymmetricNOTTestBody(
    ISecretShareEngine& engine,
    const std::vector<bool>& inputs) {
  return engine.computeBatchSymmetricNOT(inputs);
}

std::vector<bool> batchAsymmetricNOTTestBody(
    ISecretShareEngine& engine,
    const std::vector<bool>& inputs) {
  return engine.computeBatchAsymmetricNOT(inputs);
}

TEST(SecretShareEngineTest, TestBatchNOTWithDummyComponents) {
  int numberOfParty = 4;
  int size = 16384;
  auto inputs1 = generateRandomInputs(numberOfParty, size, 0);
  auto inputs2 = generateRandomInputs(numberOfParty, size, size);

  auto rst1 = testHelper(
      numberOfParty,
      testTemplate(inputs1, batchSymmetricNOTTestBody, false),
      getInsecureEngineFactoryWithDummyTupleGenerator,
      assertPartyResultsConsistent);
  auto rst2 = testHelper(
      numberOfParty,
      testTemplate(inputs2, batchAsymmetricNOTTestBody),
      getInsecureEngineFactoryWithDummyTupleGenerator,
      assertPartyResultsConsistent);
  EXPECT_EQ(rst1.size(), size);
  EXPECT_EQ(rst2.size(), size);
  for (int i = 0; i < size; i++) {
    EXPECT_EQ(rst1[i], !inputs1[i].first);
    EXPECT_EQ(rst2[i], !inputs2[i].first);
  }
}

std::vector<uint64_t> symmetricNegTestBody(
    ISecretShareEngine& engine,
    const std::vector<uint64_t>& inputs) {
  std::vector<uint64_t> rst(inputs.size());
  for (int i = 0; i < inputs.size(); i++) {
    rst[i] = engine.computeSymmetricNeg(inputs[i]);
  }
  return rst;
}

TEST(SecretShareEngineTest, TestNegWithDummyComponents) {
  int numberOfParty = 4;
  int size = 16384;
  auto inputs = generateRandomIntegerInputs(numberOfParty, size, 0);
  auto rst = testHelper(
      numberOfParty,
      testTemplate(inputs, symmetricNegTestBody, false),
      getInsecureEngineFactoryWithDummyTupleGenerator,
      assertPartyResultsConsistent);
  EXPECT_EQ(rst.size(), size);
  for (int i = 0; i < size; i++) {
    EXPECT_EQ(rst[i], -inputs[i].first);
  }
}

std::vector<uint64_t> batchSymmetricNegTestBody(
    ISecretShareEngine& engine,
    const std::vector<uint64_t>& inputs) {
  return engine.computeBatchSymmetricNeg(inputs);
}

TEST(SecretShareEngineTest, TestBatchNegWithDummyComponents) {
  int numberOfParty = 4;
  int size = 16384;
  auto inputs = generateRandomIntegerInputs(numberOfParty, size, 0);

  auto rst = testHelper(
      numberOfParty,
      testTemplate(inputs, batchSymmetricNegTestBody, false),
      getInsecureEngineFactoryWithDummyTupleGenerator,
      assertPartyResultsConsistent);
  EXPECT_EQ(rst.size(), size);
  for (int i = 0; i < size; i++) {
    EXPECT_EQ(rst[i], -inputs[i].first);
  }
}

std::vector<bool> symmetricXORTestBody(
    ISecretShareEngine& engine,
    const std::vector<bool>& inputs) {
  auto size = inputs.size();
  EXPECT_EQ(size % 2, 0);

  std::vector<bool> rst(size / 2);
  for (size_t i = 0; i < size / 2; i++) {
    rst[i] = engine.computeSymmetricXOR(inputs[i], inputs[i + size / 2]);
  }
  return rst;
}

std::vector<bool> asymmetricXORTestBody(
    ISecretShareEngine& engine,
    const std::vector<bool>& inputs) {
  auto size = inputs.size();
  EXPECT_EQ(size % 2, 0);

  std::vector<bool> rst(size / 2);
  for (size_t i = 0; i < size / 2; i++) {
    rst[i] = engine.computeAsymmetricXOR(inputs[i], inputs[i + size / 2]);
  }
  return rst;
}

TEST(SecretShareEngineTest, TestXORtWitDummyComponents) {
  int numberOfParty = 4;
  int size = 16384;
  auto inputs1 = generateRandomInputs(numberOfParty, size, size);
  auto inputs2 = generateRandomInputs(numberOfParty, size, size / 2);
  auto inputs3 = generateRandomInputs(numberOfParty, size, 0);
  auto rst1 = testHelper(
      numberOfParty,
      testTemplate(inputs1, symmetricXORTestBody),
      getInsecureEngineFactoryWithDummyTupleGenerator,
      assertPartyResultsConsistent);
  auto rst2 = testHelper(
      numberOfParty,
      testTemplate(inputs2, asymmetricXORTestBody),
      getInsecureEngineFactoryWithDummyTupleGenerator,
      assertPartyResultsConsistent);
  auto rst3 = testHelper(
      numberOfParty,
      testTemplate(inputs3, symmetricXORTestBody, false),
      getInsecureEngineFactoryWithDummyTupleGenerator,
      assertPartyResultsConsistent);
  EXPECT_EQ(rst1.size(), size / 2);
  EXPECT_EQ(rst2.size(), size / 2);
  EXPECT_EQ(rst3.size(), size / 2);
  for (int i = 0; i < size / 2; i++) {
    EXPECT_EQ(rst1[i], inputs1[i].first ^ inputs1[i + size / 2].first);
    EXPECT_EQ(rst2[i], inputs2[i].first ^ inputs2[i + size / 2].first);
    EXPECT_EQ(rst3[i], inputs3[i].first ^ inputs3[i + size / 2].first);
  }
}

std::vector<bool> batchSymmetricXORTestBody(
    ISecretShareEngine& engine,
    const std::vector<bool>& inputs) {
  EXPECT_EQ(inputs.size() % 2, 0);
  auto size = inputs.size();
  auto firstHalfInput =
      std::vector<bool>(inputs.begin(), inputs.begin() + size / 2);

  auto secondHalfInput =
      std::vector<bool>(inputs.begin() + size / 2, inputs.end());

  return engine.computeBatchSymmetricXOR(firstHalfInput, secondHalfInput);
}

std::vector<bool> batchAsymmetricXORTestBody(
    ISecretShareEngine& engine,
    const std::vector<bool>& inputs) {
  EXPECT_EQ(inputs.size() % 2, 0);
  auto size = inputs.size();
  auto firstHalfInput =
      std::vector<bool>(inputs.begin(), inputs.begin() + size / 2);

  auto secondHalfInput =
      std::vector<bool>(inputs.begin() + size / 2, inputs.end());

  return engine.computeBatchAsymmetricXOR(firstHalfInput, secondHalfInput);
}

TEST(SecretShareEngineTest, TestBatchXORWithDummyComponents) {
  int numberOfParty = 4;
  int size = 16384;
  auto inputs1 = generateRandomInputs(numberOfParty, size, size);
  auto inputs2 = generateRandomInputs(numberOfParty, size, size / 2);
  auto inputs3 = generateRandomInputs(numberOfParty, size, 0);

  auto rst1 = testHelper(
      numberOfParty,
      testTemplate(inputs1, batchSymmetricXORTestBody),
      getInsecureEngineFactoryWithDummyTupleGenerator,
      assertPartyResultsConsistent);
  auto rst2 = testHelper(
      numberOfParty,
      testTemplate(inputs2, batchAsymmetricXORTestBody),
      getInsecureEngineFactoryWithDummyTupleGenerator,
      assertPartyResultsConsistent);
  auto rst3 = testHelper(
      numberOfParty,
      testTemplate(inputs3, batchSymmetricXORTestBody, false),
      getInsecureEngineFactoryWithDummyTupleGenerator,
      assertPartyResultsConsistent);
  EXPECT_EQ(rst1.size(), size / 2);
  EXPECT_EQ(rst2.size(), size / 2);
  EXPECT_EQ(rst3.size(), size / 2);

  for (int i = 0; i < size / 2; i++) {
    EXPECT_EQ(rst1[i], inputs1[i].first ^ inputs1[i + size / 2].first);
    EXPECT_EQ(rst2[i], inputs2[i].first ^ inputs2[i + size / 2].first);
    EXPECT_EQ(rst3[i], inputs3[i].first ^ inputs3[i + size / 2].first);
  }
}

std::vector<uint64_t> symmetricPlusTestBody(
    ISecretShareEngine& engine,
    const std::vector<uint64_t>& inputs) {
  auto size = inputs.size();
  EXPECT_EQ(size % 2, 0);

  std::vector<uint64_t> rst(size / 2);
  for (size_t i = 0; i < size / 2; i++) {
    rst[i] = engine.computeSymmetricPlus(inputs[i], inputs[i + size / 2]);
  }
  return rst;
}

std::vector<uint64_t> asymmetricPlusTestBody(
    ISecretShareEngine& engine,
    const std::vector<uint64_t>& inputs) {
  auto size = inputs.size();
  EXPECT_EQ(size % 2, 0);

  std::vector<uint64_t> rst(size / 2);
  for (size_t i = 0; i < size / 2; i++) {
    rst[i] = engine.computeAsymmetricPlus(inputs[i], inputs[i + size / 2]);
  }
  return rst;
}

TEST(SecretShareEngineTest, TestPlustWitDummyComponents) {
  int numberOfParty = 4;
  int size = 16384;
  auto inputs1 = generateRandomIntegerInputs(numberOfParty, size, size);
  auto inputs2 = generateRandomIntegerInputs(numberOfParty, size, size / 2);
  auto inputs3 = generateRandomIntegerInputs(numberOfParty, size, 0);
  auto rst1 = testHelper(
      numberOfParty,
      testTemplate(inputs1, symmetricPlusTestBody),
      getInsecureEngineFactoryWithDummyTupleGenerator,
      assertPartyResultsConsistent);
  auto rst2 = testHelper(
      numberOfParty,
      testTemplate(inputs2, asymmetricPlusTestBody),
      getInsecureEngineFactoryWithDummyTupleGenerator,
      assertPartyResultsConsistent);
  auto rst3 = testHelper(
      numberOfParty,
      testTemplate(inputs3, symmetricPlusTestBody, false),
      getInsecureEngineFactoryWithDummyTupleGenerator,
      assertPartyResultsConsistent);
  EXPECT_EQ(rst1.size(), size / 2);
  EXPECT_EQ(rst2.size(), size / 2);
  EXPECT_EQ(rst3.size(), size / 2);
  for (int i = 0; i < size / 2; i++) {
    EXPECT_EQ(rst1[i], inputs1[i].first + inputs1[i + size / 2].first);
    EXPECT_EQ(rst2[i], inputs2[i].first + inputs2[i + size / 2].first);
    EXPECT_EQ(rst3[i], inputs3[i].first + inputs3[i + size / 2].first);
  }
}

std::vector<uint64_t> batchSymmetricPlusTestBody(
    ISecretShareEngine& engine,
    const std::vector<uint64_t>& inputs) {
  EXPECT_EQ(inputs.size() % 2, 0);
  auto size = inputs.size();
  auto firstHalfInput =
      std::vector<uint64_t>(inputs.begin(), inputs.begin() + size / 2);

  auto secondHalfInput =
      std::vector<uint64_t>(inputs.begin() + size / 2, inputs.end());

  return engine.computeBatchSymmetricPlus(firstHalfInput, secondHalfInput);
}

std::vector<uint64_t> batchAsymmetricPlusTestBody(
    ISecretShareEngine& engine,
    const std::vector<uint64_t>& inputs) {
  EXPECT_EQ(inputs.size() % 2, 0);
  auto size = inputs.size();
  auto firstHalfInput =
      std::vector<uint64_t>(inputs.begin(), inputs.begin() + size / 2);

  auto secondHalfInput =
      std::vector<uint64_t>(inputs.begin() + size / 2, inputs.end());

  return engine.computeBatchAsymmetricPlus(firstHalfInput, secondHalfInput);
}

TEST(SecretShareEngineTest, TestBatchPlusWithDummyComponents) {
  int numberOfParty = 4;
  int size = 16384;
  auto inputs1 = generateRandomIntegerInputs(numberOfParty, size, size);
  auto inputs2 = generateRandomIntegerInputs(numberOfParty, size, size / 2);
  auto inputs3 = generateRandomIntegerInputs(numberOfParty, size, 0);

  auto rst1 = testHelper(
      numberOfParty,
      testTemplate(inputs1, batchSymmetricPlusTestBody),
      getInsecureEngineFactoryWithDummyTupleGenerator,
      assertPartyResultsConsistent);
  auto rst2 = testHelper(
      numberOfParty,
      testTemplate(inputs2, batchAsymmetricPlusTestBody),
      getInsecureEngineFactoryWithDummyTupleGenerator,
      assertPartyResultsConsistent);
  auto rst3 = testHelper(
      numberOfParty,
      testTemplate(inputs3, batchSymmetricPlusTestBody, false),
      getInsecureEngineFactoryWithDummyTupleGenerator,
      assertPartyResultsConsistent);
  EXPECT_EQ(rst1.size(), size / 2);
  EXPECT_EQ(rst2.size(), size / 2);
  EXPECT_EQ(rst3.size(), size / 2);

  for (int i = 0; i < size / 2; i++) {
    EXPECT_EQ(rst1[i], inputs1[i].first + inputs1[i + size / 2].first);
    EXPECT_EQ(rst2[i], inputs2[i].first + inputs2[i + size / 2].first);
    EXPECT_EQ(rst3[i], inputs3[i].first + inputs3[i + size / 2].first);
  }
}

std::tuple<
    std::vector<bool>,
    std::vector<std::vector<bool>>,
    std::vector<uint64_t>>
ANDMultTestBody(
    ISecretShareEngine& engine,
    const std::vector<bool>& boolInputs,
    const std::vector<uint64_t>& intInputs) {
  auto intSize = intInputs.size();
  auto boolSize = boolInputs.size();
  EXPECT_EQ(intSize % 16, 0);
  EXPECT_EQ(boolSize % 16, 0);

  // regular Mult's
  std::vector<int> regularMultIndex(intSize / 2);
  for (int i = 0; i < intSize / 2; i++) {
    regularMultIndex[i] =
        engine.scheduleMult(intInputs[i], intInputs[i + intSize / 2]);
  }

  // regular AND's
  std::vector<int> regularANDIndex(boolSize / 2);
  for (int i = 0; i < boolSize / 2; i++) {
    regularANDIndex[i] =
        engine.scheduleAND(boolInputs[i], boolInputs[i + boolSize / 2]);
  }

  // batch Mult's
  auto firstHalfIntInput =
      std::vector<uint64_t>(intInputs.begin(), intInputs.begin() + intSize / 2);

  auto secondHalfIntInput =
      std::vector<uint64_t>(intInputs.begin() + intSize / 2, intInputs.end());

  auto batchIndexInt0 =
      engine.scheduleBatchMult(firstHalfIntInput, secondHalfIntInput);
  auto batchIndexInt1 =
      engine.scheduleBatchMult(firstHalfIntInput, secondHalfIntInput);

  // batch AND's
  auto firstHalfBoolInput =
      std::vector<bool>(boolInputs.begin(), boolInputs.begin() + boolSize / 2);

  auto secondHalfBoolInput =
      std::vector<bool>(boolInputs.begin() + boolSize / 2, boolInputs.end());
  engine.computeBatchANDImmediately(firstHalfBoolInput, secondHalfBoolInput);

  auto batchIndexBool0 =
      engine.scheduleBatchAND(firstHalfBoolInput, secondHalfBoolInput);
  auto batchIndexBool1 =
      engine.scheduleBatchAND(firstHalfBoolInput, secondHalfBoolInput);

  std::vector<bool> leftComposite15 =
      std::vector<bool>(boolInputs.begin(), boolInputs.begin() + boolSize / 16);
  std::vector<std::vector<bool>> rightComposite15(15);

  for (int i = 0; i < boolSize / 16; i++) {
    for (int j = 0; j < 15; j++) {
      rightComposite15[j].push_back(boolInputs[boolSize / 16 + 15 * i + j]);
    }
  }

  std::vector<bool> leftComposite7 =
      std::vector<bool>(boolInputs.begin(), boolInputs.begin() + boolSize / 8);
  std::vector<std::vector<bool>> rightComposite7(7);

  for (int i = 0; i < boolSize / 8; i++) {
    for (int j = 0; j < 7; j++) {
      rightComposite7[j].push_back(boolInputs[boolSize / 8 + 7 * i + j]);
    }
  }

  std::vector<bool> leftComposite3 =
      std::vector<bool>(boolInputs.begin(), boolInputs.begin() + boolSize / 4);
  std::vector<std::vector<bool>> rightComposite3(3);

  for (int i = 0; i < boolSize / 4; i++) {
    for (int j = 0; j < 3; j++) {
      rightComposite3[j].push_back(boolInputs[boolSize / 4 + 3 * i + j]);
    }
  }

  // schedule composite runs by running through batch structures individually
  std::vector<int> compositeANDIndex(
      boolSize / 16 + boolSize / 8 + boolSize / 4);
  for (int i = 0; i < boolSize / 16; i++) {
    std::vector<bool> rights(15);
    for (int j = 0; j < 15; j++) {
      rights[j] = rightComposite15[j][i];
    }
    compositeANDIndex[i] =
        engine.scheduleCompositeAND(leftComposite15[i], rights);
  }

  for (int i = 0; i < boolSize / 8; i++) {
    std::vector<bool> rights(7);
    for (int j = 0; j < 7; j++) {
      rights[j] = rightComposite7[j][i];
    }
    compositeANDIndex[boolSize / 16 + i] =
        engine.scheduleCompositeAND(leftComposite7[i], rights);
  }
  for (int i = 0; i < boolSize / 4; i++) {
    std::vector<bool> rights(3);
    for (int j = 0; j < 3; j++) {
      rights[j] = rightComposite3[j][i];
    }
    compositeANDIndex[boolSize / 16 + boolSize / 8 + i] =
        engine.scheduleCompositeAND(leftComposite3[i], rights);
  }

  // schedule batch composite runs
  auto batchCompositeIndex0 =
      engine.scheduleBatchCompositeAND(leftComposite15, rightComposite15);
  auto batchCompositeIndex1 =
      engine.scheduleBatchCompositeAND(leftComposite7, rightComposite7);
  auto batchCompositeIndex2 =
      engine.scheduleBatchCompositeAND(leftComposite3, rightComposite3);

  engine.executeScheduledOperations();

  // Regular Mult
  std::vector<uint64_t> multResult(intSize / 2);
  for (size_t i = 0; i < intSize / 2; i++) {
    multResult[i] = engine.getMultExecutionResult(regularMultIndex[i]);
  }
  auto tmp0 = engine.getBatchMultExecutionResult(batchIndexInt0);
  multResult.insert(multResult.end(), tmp0.begin(), tmp0.end());

  tmp0 = engine.getBatchMultExecutionResult(batchIndexInt1);
  multResult.insert(multResult.end(), tmp0.begin(), tmp0.end());

  tmp0 =
      engine.computeBatchMultImmediately(firstHalfIntInput, secondHalfIntInput);
  multResult.insert(multResult.end(), tmp0.begin(), tmp0.end());

  // Regular AND
  std::vector<bool> andResult(boolSize / 2);
  for (size_t i = 0; i < boolSize / 2; i++) {
    andResult[i] = engine.getANDExecutionResult(regularANDIndex[i]);
  }
  auto tmp1 = engine.getBatchANDExecutionResult(batchIndexBool0);
  andResult.insert(andResult.end(), tmp1.begin(), tmp1.end());

  tmp1 = engine.getBatchANDExecutionResult(batchIndexBool1);
  andResult.insert(andResult.end(), tmp1.begin(), tmp1.end());

  tmp1 = engine.computeBatchANDImmediately(
      firstHalfBoolInput, secondHalfBoolInput);
  andResult.insert(andResult.end(), tmp1.begin(), tmp1.end());

  // Composite AND
  std::vector<std::vector<bool>> compositeAndResult;
  for (auto compositeIndex : compositeANDIndex) {
    tmp1 = engine.getCompositeANDExecutionResult(compositeIndex);
    compositeAndResult.push_back(tmp1);
  }

  auto tmp2 = engine.getBatchCompositeANDExecutionResult(batchCompositeIndex0);
  compositeAndResult.insert(compositeAndResult.end(), tmp2.begin(), tmp2.end());

  tmp2 = engine.getBatchCompositeANDExecutionResult(batchCompositeIndex1);
  compositeAndResult.insert(compositeAndResult.end(), tmp2.begin(), tmp2.end());

  tmp2 = engine.getBatchCompositeANDExecutionResult(batchCompositeIndex2);
  compositeAndResult.insert(compositeAndResult.end(), tmp2.begin(), tmp2.end());

  return std::make_tuple(andResult, compositeAndResult, multResult);
}

class NonFreeMultAndANDTestFixture
    : public ::testing::TestWithParam<std::tuple<
          std::string, // Human readable name
          size_t, // number of parties
          std::function<std::unique_ptr<SecretShareEngineFactory>(
              int myId,
              int numberOfParty,
              communication::IPartyCommunicationAgentFactory& agentFactory,
              std::shared_ptr<fbpcf::util::MetricCollector>
                  metricCollector)>>> {};

INSTANTIATE_TEST_SUITE_P(
    SecretShareEngineTest,
    NonFreeMultAndANDTestFixture,
    ::testing::Values(
        std::make_tuple<
            std::string,
            size_t,
            std::function<std::unique_ptr<SecretShareEngineFactory>(
                int myId,
                int numberOfParty,
                communication::IPartyCommunicationAgentFactory& agentFactory,
                std::shared_ptr<fbpcf::util::MetricCollector>
                    metricCollector)>>(
            "InsecureEngineWithDummyTupleGenerator",
            2,
            getInsecureEngineFactoryWithDummyTupleGenerator),
        std::make_tuple<
            std::string,
            size_t,
            std::function<std::unique_ptr<SecretShareEngineFactory>(
                int myId,
                int numberOfParty,
                communication::IPartyCommunicationAgentFactory& agentFactory,
                std::shared_ptr<fbpcf::util::MetricCollector>
                    metricCollector)>>(
            "InsecureEngineWithDummyTupleGenerator",
            4,
            getInsecureEngineFactoryWithDummyTupleGenerator),
        std::make_tuple<
            std::string,
            size_t,
            std::function<std::unique_ptr<SecretShareEngineFactory>(
                int myId,
                int numberOfParty,
                communication::IPartyCommunicationAgentFactory& agentFactory,
                std::shared_ptr<fbpcf::util::MetricCollector>
                    metricCollector)>>(
            "SecureEngineWithFerret",
            2,
            getSecureEngineFactoryWithBooleanAndIntegerTupleGenerator)),
    [](const testing::TestParamInfo<NonFreeMultAndANDTestFixture::ParamType>&
           info) {
      return std::get<0>(info.param) + '_' +
          std::to_string(std::get<1>(info.param)) + "Party";
    });

TEST_P(NonFreeMultAndANDTestFixture, TestMultAndAND) {
  size_t numberOfParty = std::get<1>(GetParam());
  size_t size = 16384;
  auto boolInputs = generateRandomInputs(numberOfParty, size, size);
  auto intInputs = generateRandomIntegerInputs(numberOfParty, size, size);

  auto result = testHelper(
      numberOfParty,
      testTemplate(boolInputs, intInputs, ANDMultTestBody),
      std::get<2>(GetParam()),
      assertPartyResultsConsistent);
  auto andResult = std::get<0>(result);
  auto compositeAndResult = std::get<1>(result);
  auto multResult = std::get<2>(result);

  ASSERT_EQ(andResult.size(), 2 * size);
  for (int i = 0; i < size / 2; i++) {
    EXPECT_EQ(
        andResult[i], boolInputs[i].first && boolInputs[i + size / 2].first);
    EXPECT_EQ(
        andResult[i + size / 2],
        boolInputs[i].first && boolInputs[i + size / 2].first);
    EXPECT_EQ(
        andResult[i + size],
        boolInputs[i].first && boolInputs[i + size / 2].first);
    EXPECT_EQ(
        andResult[i + size / 2 + size],
        boolInputs[i].first && boolInputs[i + size / 2].first);
  }

  int composite15EndIndex = size / 16;
  int composite7EndIndex = composite15EndIndex + size / 8;
  int composite3EndIndex = composite7EndIndex + size / 4;
  int batchComposite15EndIndex = composite3EndIndex + 15;
  int batchComposite7EndIndex = batchComposite15EndIndex + 7;

  // First check 1:15 runs
  for (int i = 0; i < size / 16; i++) {
    for (int j = 0; j < 15; j++) {
      EXPECT_EQ(
          compositeAndResult[i][j],
          boolInputs[i].first && boolInputs[size / 16 + 15 * i + j].first);
      EXPECT_EQ(
          compositeAndResult[j + composite3EndIndex][i],
          boolInputs[i].first && boolInputs[size / 16 + 15 * i + j].first);
    }
  }

  for (int i = 0; i < size / 8; i++) {
    for (int j = 0; j < 7; j++) {
      EXPECT_EQ(
          compositeAndResult[i + composite15EndIndex][j],
          boolInputs[i].first && boolInputs[size / 8 + 7 * i + j].first);
      EXPECT_EQ(
          compositeAndResult[j + batchComposite15EndIndex][i],
          boolInputs[i].first && boolInputs[size / 8 + 7 * i + j].first);
    }
  }

  // check 1:3 runs
  for (int i = 0; i < size / 4; i++) {
    for (int j = 0; j < 3; j++) {
      EXPECT_EQ(
          compositeAndResult[i + composite7EndIndex][j],
          boolInputs[i].first && boolInputs[size / 4 + 3 * i + j].first);
      EXPECT_EQ(
          compositeAndResult[j + batchComposite7EndIndex][i],
          boolInputs[i].first && boolInputs[size / 4 + 3 * i + j].first);
    }
  }

  ASSERT_EQ(multResult.size(), 2 * size);
  for (int i = 0; i < size / 2; i++) {
    EXPECT_EQ(
        multResult[i], intInputs[i].first * intInputs[i + size / 2].first);
    EXPECT_EQ(
        multResult[i + size / 2],
        intInputs[i].first * intInputs[i + size / 2].first);
    EXPECT_EQ(
        multResult[i + size],
        intInputs[i].first * intInputs[i + size / 2].first);
    EXPECT_EQ(
        multResult[i + size / 2 + size],
        intInputs[i].first * intInputs[i + size / 2].first);
  }
}

std::pair<std::vector<bool>, std::vector<std::vector<bool>>> ANDTestBody(
    ISecretShareEngine& engine,
    const std::vector<bool>& inputs) {
  auto [andResult, compositeAndResult, multResult] =
      ANDMultTestBody(engine, inputs, std::vector<uint64_t>());

  return std::make_pair(andResult, compositeAndResult);
}

class NonFreeAndTestFixture
    : public ::testing::TestWithParam<std::tuple<
          std::string, // Human readable name
          size_t, // number of parties
          std::function<std::unique_ptr<SecretShareEngineFactory>(
              int myId,
              int numberOfParty,
              communication::IPartyCommunicationAgentFactory& agentFactory,
              std::shared_ptr<fbpcf::util::MetricCollector>
                  metricCollector)>>> {};

INSTANTIATE_TEST_SUITE_P(
    SecretShareEngineTest,
    NonFreeAndTestFixture,
    ::testing::Values(
        std::make_tuple<
            std::string,
            size_t,
            std::function<std::unique_ptr<SecretShareEngineFactory>(
                int myId,
                int numberOfParty,
                communication::IPartyCommunicationAgentFactory& agentFactory,
                std::shared_ptr<fbpcf::util::MetricCollector>
                    metricCollector)>>(
            "InsecureEngineWithDummyTupleGenerator",
            2,
            getInsecureEngineFactoryWithDummyTupleGenerator),
        std::make_tuple<
            std::string,
            size_t,
            std::function<std::unique_ptr<SecretShareEngineFactory>(
                int myId,
                int numberOfParty,
                communication::IPartyCommunicationAgentFactory& agentFactory,
                std::shared_ptr<fbpcf::util::MetricCollector>
                    metricCollector)>>(
            "InsecureEngineWithDummyTupleGenerator",
            4,
            getInsecureEngineFactoryWithDummyTupleGenerator),
        std::make_tuple<
            std::string,
            size_t,
            std::function<std::unique_ptr<SecretShareEngineFactory>(
                int myId,
                int numberOfParty,
                communication::IPartyCommunicationAgentFactory& agentFactory,
                std::shared_ptr<fbpcf::util::MetricCollector>
                    metricCollector)>>(
            "SecureEngineWithFerret",
            2,
            getSecureEngineFactoryWithFERRET),
        std::make_tuple<
            std::string,
            size_t,
            std::function<std::unique_ptr<SecretShareEngineFactory>(
                int myId,
                int numberOfParty,
                communication::IPartyCommunicationAgentFactory& agentFactory,
                std::shared_ptr<fbpcf::util::MetricCollector>
                    metricCollector)>>(
            "SecureEngineWithFerret",
            3,
            getSecureEngineFactoryWithFERRET)),
    [](const testing::TestParamInfo<NonFreeAndTestFixture::ParamType>& info) {
      return std::get<0>(info.param) + '_' +
          std::to_string(std::get<1>(info.param)) + "Party";
    });

TEST_P(NonFreeAndTestFixture, TestAnd) {
  size_t numberOfParty = std::get<1>(GetParam());
  size_t size = 16384;
  auto inputs = generateRandomInputs(numberOfParty, size, size);

  auto rst = testHelper(
      numberOfParty,
      testTemplate(inputs, ANDTestBody),
      std::get<2>(GetParam()),
      assertPartyResultsConsistent);
  auto andResult = std::get<0>(rst);
  auto compositeAndResult = std::get<1>(rst);
  ASSERT_EQ(andResult.size(), 2 * size);
  for (int i = 0; i < size / 2; i++) {
    EXPECT_EQ(andResult[i], inputs[i].first && inputs[i + size / 2].first);
    EXPECT_EQ(
        andResult[i + size / 2], inputs[i].first && inputs[i + size / 2].first);
    EXPECT_EQ(
        andResult[i + size], inputs[i].first && inputs[i + size / 2].first);
    EXPECT_EQ(
        andResult[i + size / 2 + size],
        inputs[i].first && inputs[i + size / 2].first);
  }

  // Ordering of composite results
  // First size / 16 vectors is 1:15 runs (size 15)
  // Next size / 8 vectors is 1:7 runs (size 7)
  // Next size / 4 vectors is 1:3 runs (size 3)
  // Next 15 vectors is batch 1:15 run (batch size = size / 16)
  // Next 7 vectors is batch 1:7 run (batch size = size / 8)
  // Next 3 vectors is batch 1:3 run (batch size = size / 4)

  int composite15EndIndex = size / 16;
  int composite7EndIndex = composite15EndIndex + size / 8;
  int composite3EndIndex = composite7EndIndex + size / 4;
  int batchComposite15EndIndex = composite3EndIndex + 15;
  int batchComposite7EndIndex = batchComposite15EndIndex + 7;

  // First check 1:15 runs
  for (int i = 0; i < size / 16; i++) {
    for (int j = 0; j < 15; j++) {
      EXPECT_EQ(
          compositeAndResult[i][j],
          inputs[i].first && inputs[size / 16 + 15 * i + j].first);
      EXPECT_EQ(
          compositeAndResult[j + composite3EndIndex][i],
          inputs[i].first && inputs[size / 16 + 15 * i + j].first);
    }
  }

  for (int i = 0; i < size / 8; i++) {
    for (int j = 0; j < 7; j++) {
      EXPECT_EQ(
          compositeAndResult[i + composite15EndIndex][j],
          inputs[i].first && inputs[size / 8 + 7 * i + j].first);
      EXPECT_EQ(
          compositeAndResult[j + batchComposite15EndIndex][i],
          inputs[i].first && inputs[size / 8 + 7 * i + j].first);
    }
  }

  // check 1:3 runs
  for (int i = 0; i < size / 4; i++) {
    for (int j = 0; j < 3; j++) {
      EXPECT_EQ(
          compositeAndResult[i + composite7EndIndex][j],
          inputs[i].first && inputs[size / 4 + 3 * i + j].first);
      EXPECT_EQ(
          compositeAndResult[j + batchComposite7EndIndex][i],
          inputs[i].first && inputs[size / 4 + 3 * i + j].first);
    }
  }
}

std::vector<uint64_t> MultTestBody(
    ISecretShareEngine& engine,
    const std::vector<uint64_t>& inputs) {
  auto [andResult, compositeAndResult, multResult] =
      ANDMultTestBody(engine, std::vector<bool>(), inputs);
  return multResult;
}

class NonFreeMultTestFixture
    : public ::testing::TestWithParam<std::tuple<
          std::string, // Human readable name
          size_t, // number of parties
          std::function<std::unique_ptr<SecretShareEngineFactory>(
              int myId,
              int numberOfParty,
              communication::IPartyCommunicationAgentFactory& agentFactory,
              std::shared_ptr<fbpcf::util::MetricCollector>
                  metricCollector)>>> {};

INSTANTIATE_TEST_SUITE_P(
    SecretShareEngineTest,
    NonFreeMultTestFixture,
    ::testing::Values(
        std::make_tuple<
            std::string,
            size_t,
            std::function<std::unique_ptr<SecretShareEngineFactory>(
                int myId,
                int numberOfParty,
                communication::IPartyCommunicationAgentFactory& agentFactory,
                std::shared_ptr<fbpcf::util::MetricCollector>
                    metricCollector)>>(
            "InsecureEngineWithDummyTupleGenerator",
            2,
            getInsecureEngineFactoryWithDummyTupleGenerator),
        std::make_tuple<
            std::string,
            size_t,
            std::function<std::unique_ptr<SecretShareEngineFactory>(
                int myId,
                int numberOfParty,
                communication::IPartyCommunicationAgentFactory& agentFactory,
                std::shared_ptr<fbpcf::util::MetricCollector>
                    metricCollector)>>(
            "InsecureEngineWithDummyTupleGenerator",
            4,
            getInsecureEngineFactoryWithDummyTupleGenerator),
        std::make_tuple<
            std::string,
            size_t,
            std::function<std::unique_ptr<SecretShareEngineFactory>(
                int myId,
                int numberOfParty,
                communication::IPartyCommunicationAgentFactory& agentFactory,
                std::shared_ptr<fbpcf::util::MetricCollector>
                    metricCollector)>>(
            "SecureEngineWithFerret",
            2,
            getSecureEngineFactoryWithIntegerOnlyTupleGenerator),
        std::make_tuple<
            std::string,
            size_t,
            std::function<std::unique_ptr<SecretShareEngineFactory>(
                int myId,
                int numberOfParty,
                communication::IPartyCommunicationAgentFactory& agentFactory,
                std::shared_ptr<fbpcf::util::MetricCollector>
                    metricCollector)>>(
            "SecureEngineWithFerret",
            3,
            getSecureEngineFactoryWithIntegerOnlyTupleGenerator)),
    [](const testing::TestParamInfo<NonFreeMultTestFixture::ParamType>& info) {
      return std::get<0>(info.param) + '_' +
          std::to_string(std::get<1>(info.param)) + "Party";
    });

TEST_P(NonFreeMultTestFixture, TestMult) {
  size_t numberOfParty = std::get<1>(GetParam());
  size_t size = 16384;
  auto inputs = generateRandomIntegerInputs(numberOfParty, size, size);

  auto multResult = testHelper(
      numberOfParty,
      testTemplate(inputs, MultTestBody),
      std::get<2>(GetParam()),
      assertPartyResultsConsistent);
  ASSERT_EQ(multResult.size(), 2 * size);
  for (int i = 0; i < size / 2; i++) {
    EXPECT_EQ(multResult[i], inputs[i].first * inputs[i + size / 2].first);
    EXPECT_EQ(
        multResult[i + size / 2], inputs[i].first * inputs[i + size / 2].first);
    EXPECT_EQ(
        multResult[i + size], inputs[i].first * inputs[i + size / 2].first);
    EXPECT_EQ(
        multResult[i + size / 2 + size],
        inputs[i].first * inputs[i + size / 2].first);
  }
}

std::vector<bool> FreeANDTestBody(
    ISecretShareEngine& engine,
    const std::vector<bool>& inputs) {
  auto size = inputs.size();
  EXPECT_EQ(size % 2, 0);

  std::vector<bool> rst(size / 2);
  for (size_t i = 0; i < size / 2; i++) {
    rst[i] = engine.computeFreeAND(inputs[i], inputs[i + size / 2]);
  }
  return rst;
}

TEST(SecretShareEngineTest, TestFreeANDWithDummyComponents) {
  int numberOfParty = 4;
  int size = 16384;
  auto inputs1 = generateRandomInputs(numberOfParty, size, size / 2);
  auto inputs2 = generateRandomInputs(numberOfParty, size, 0);

  auto rst1 = testHelper(
      numberOfParty,
      testTemplate(inputs1, FreeANDTestBody),
      getInsecureEngineFactoryWithDummyTupleGenerator,
      assertPartyResultsConsistent);
  auto rst2 = testHelper(
      numberOfParty,
      testTemplate(inputs2, FreeANDTestBody, false),
      getInsecureEngineFactoryWithDummyTupleGenerator,
      assertPartyResultsConsistent);
  EXPECT_EQ(rst1.size(), size / 2);
  EXPECT_EQ(rst2.size(), size / 2);
  for (int i = 0; i < size / 2; i++) {
    EXPECT_EQ(rst1[i], inputs1[i].first && inputs1[i + size / 2].first);
    EXPECT_EQ(rst2[i], inputs2[i].first && inputs2[i + size / 2].first);
  }
}

std::vector<bool> BatchFreeANDTestBody(
    ISecretShareEngine& engine,
    const std::vector<bool>& inputs) {
  EXPECT_EQ(inputs.size() % 2, 0);
  auto size = inputs.size();
  auto firstHalfInput =
      std::vector<bool>(inputs.begin(), inputs.begin() + size / 2);

  auto secondHalfInput =
      std::vector<bool>(inputs.begin() + size / 2, inputs.end());

  return engine.computeBatchFreeAND(firstHalfInput, secondHalfInput);
}

TEST(SecretShareEngineTest, TestBatchFreeANDWithDummyComponents) {
  int numberOfParty = 4;
  int size = 16384;
  auto inputs1 = generateRandomInputs(numberOfParty, size, size / 2);
  auto inputs2 = generateRandomInputs(numberOfParty, size, 0);
  auto rst1 = testHelper(
      numberOfParty,
      testTemplate(inputs1, BatchFreeANDTestBody),
      getInsecureEngineFactoryWithDummyTupleGenerator,
      assertPartyResultsConsistent);
  auto rst2 = testHelper(
      numberOfParty,
      testTemplate(inputs2, BatchFreeANDTestBody, false),
      getInsecureEngineFactoryWithDummyTupleGenerator,
      assertPartyResultsConsistent);
  EXPECT_EQ(rst1.size(), size / 2);
  for (int i = 0; i < size / 2; i++) {
    EXPECT_EQ(rst1[i], inputs1[i].first && inputs1[i + size / 2].first);
    EXPECT_EQ(rst2[i], inputs2[i].first && inputs2[i + size / 2].first);
  }
}

std::vector<uint64_t> FreeMultTestBody(
    ISecretShareEngine& engine,
    const std::vector<uint64_t>& inputs) {
  auto size = inputs.size();
  EXPECT_EQ(size % 2, 0);

  std::vector<uint64_t> rst(size / 2);
  for (size_t i = 0; i < size / 2; i++) {
    rst[i] = engine.computeFreeMult(inputs[i], inputs[i + size / 2]);
  }
  return rst;
}

TEST(SecretShareEngineTest, TestFreeMultWithDummyComponents) {
  int numberOfParty = 4;
  int size = 16384;
  auto inputs1 = generateRandomIntegerInputs(numberOfParty, size, size / 2);
  auto inputs2 = generateRandomIntegerInputs(numberOfParty, size, 0);

  auto rst1 = testHelper(
      numberOfParty,
      testTemplate(inputs1, FreeMultTestBody),
      getInsecureEngineFactoryWithDummyTupleGenerator,
      assertPartyResultsConsistent);
  auto rst2 = testHelper(
      numberOfParty,
      testTemplate(inputs2, FreeMultTestBody, false),
      getInsecureEngineFactoryWithDummyTupleGenerator,
      assertPartyResultsConsistent);
  EXPECT_EQ(rst1.size(), size / 2);
  EXPECT_EQ(rst2.size(), size / 2);
  for (int i = 0; i < size / 2; i++) {
    EXPECT_EQ(rst1[i], inputs1[i].first * inputs1[i + size / 2].first);
    EXPECT_EQ(rst2[i], inputs2[i].first * inputs2[i + size / 2].first);
  }
}

std::vector<uint64_t> BatchFreeMultTestBody(
    ISecretShareEngine& engine,
    const std::vector<uint64_t>& inputs) {
  EXPECT_EQ(inputs.size() % 2, 0);
  auto size = inputs.size();
  auto firstHalfInput =
      std::vector<uint64_t>(inputs.begin(), inputs.begin() + size / 2);

  auto secondHalfInput =
      std::vector<uint64_t>(inputs.begin() + size / 2, inputs.end());

  return engine.computeBatchFreeMult(firstHalfInput, secondHalfInput);
}

TEST(SecretShareEngineTest, TestBatchFreeMultWithDummyComponents) {
  int numberOfParty = 4;
  int size = 16384;
  auto inputs1 = generateRandomIntegerInputs(numberOfParty, size, size / 2);
  auto inputs2 = generateRandomIntegerInputs(numberOfParty, size, 0);
  auto rst1 = testHelper(
      numberOfParty,
      testTemplate(inputs1, BatchFreeMultTestBody),
      getInsecureEngineFactoryWithDummyTupleGenerator,
      assertPartyResultsConsistent);
  auto rst2 = testHelper(
      numberOfParty,
      testTemplate(inputs2, BatchFreeMultTestBody, false),
      getInsecureEngineFactoryWithDummyTupleGenerator,
      assertPartyResultsConsistent);
  EXPECT_EQ(rst1.size(), size / 2);
  for (int i = 0; i < size / 2; i++) {
    EXPECT_EQ(rst1[i], inputs1[i].first * inputs1[i + size / 2].first);
    EXPECT_EQ(rst2[i], inputs2[i].first * inputs2[i + size / 2].first);
  }
}

TEST(DummySecretShareEngineTest, TestInputAndOutput) {
  int numberOfParty = 4;
  int size = 16384;
  auto inputs = generateRandomInputs(numberOfParty, size, size);

  auto rst = testHelper(
      numberOfParty,
      testTemplate(inputs, inputAndOutputTestBody),
      getDummyEngineFactory,
      assertPartyResultsConsistent);
  ASSERT_EQ(rst.size(), size);
}

TEST(DummySecretShareEngineTest, TestNOTAndXORAndFreeAND) {
  int numberOfParty = 4;
  int size = 16384;
  auto inputs1 = generateRandomInputs(numberOfParty, size, 0);
  auto inputs2 = generateRandomInputs(numberOfParty, size, size);
  auto inputs3 = generateRandomInputs(numberOfParty, size, size / 2);

  // Test NOT
  auto rst1NOT = testHelper(
      numberOfParty,
      testTemplate(inputs1, symmetricNOTTestBody, false),
      getDummyEngineFactory,
      assertPartyResultsConsistent);
  auto rst2NOT = testHelper(
      numberOfParty,
      testTemplate(inputs2, asymmetricNOTTestBody),
      getDummyEngineFactory,
      assertPartyResultsConsistent);
  EXPECT_EQ(rst1NOT.size(), size);
  EXPECT_EQ(rst2NOT.size(), size);

  // Test XOR
  auto rst1XOR = testHelper(
      numberOfParty,
      testTemplate(inputs1, symmetricXORTestBody, false),
      getDummyEngineFactory,
      assertPartyResultsConsistent);
  auto rst2XOR = testHelper(
      numberOfParty,
      testTemplate(inputs2, symmetricXORTestBody),
      getDummyEngineFactory,
      assertPartyResultsConsistent);
  auto rst3XOR = testHelper(
      numberOfParty,
      testTemplate(inputs3, asymmetricXORTestBody),
      getDummyEngineFactory,
      assertPartyResultsConsistent);
  EXPECT_EQ(rst1XOR.size(), size / 2);
  EXPECT_EQ(rst2XOR.size(), size / 2);
  EXPECT_EQ(rst3XOR.size(), size / 2);

  // Test FreeAND
  auto rst1FreeAND = testHelper(
      numberOfParty,
      testTemplate(inputs1, FreeANDTestBody, false),
      getDummyEngineFactory,
      assertPartyResultsConsistent);
  auto rst3FreeAND = testHelper(
      numberOfParty,
      testTemplate(inputs3, FreeANDTestBody),
      getDummyEngineFactory,
      assertPartyResultsConsistent);
  EXPECT_EQ(rst1FreeAND.size(), size / 2);
  EXPECT_EQ(rst3FreeAND.size(), size / 2);
}

TEST(DummySecretShareEngineTest, TestBatchNOTAndBatchXORAndBatchFreeAnd) {
  int numberOfParty = 4;
  int size = 16384;
  auto inputs1 = generateRandomInputs(numberOfParty, size, 0);
  auto inputs2 = generateRandomInputs(numberOfParty, size, size);
  auto inputs3 = generateRandomInputs(numberOfParty, size, size / 2);

  // Test BatchNOT
  auto rst1NOT = testHelper(
      numberOfParty,
      testTemplate(inputs1, batchSymmetricNOTTestBody, false),
      getDummyEngineFactory,
      assertPartyResultsConsistent);
  auto rst2NOT = testHelper(
      numberOfParty,
      testTemplate(inputs2, batchAsymmetricNOTTestBody),
      getDummyEngineFactory,
      assertPartyResultsConsistent);
  EXPECT_EQ(rst1NOT.size(), size);
  EXPECT_EQ(rst2NOT.size(), size);

  // Test BatchXOR
  auto rst1XOR = testHelper(
      numberOfParty,
      testTemplate(inputs1, batchSymmetricXORTestBody, false),
      getDummyEngineFactory,
      assertPartyResultsConsistent);
  auto rst2XOR = testHelper(
      numberOfParty,
      testTemplate(inputs2, batchSymmetricXORTestBody),
      getDummyEngineFactory,
      assertPartyResultsConsistent);
  auto rst3XOR = testHelper(
      numberOfParty,
      testTemplate(inputs3, batchAsymmetricXORTestBody),
      getDummyEngineFactory,
      assertPartyResultsConsistent);
  EXPECT_EQ(rst1XOR.size(), size / 2);
  EXPECT_EQ(rst2XOR.size(), size / 2);
  EXPECT_EQ(rst3XOR.size(), size / 2);

  // Test BatchFreeAND
  auto rst1FreeAND = testHelper(
      numberOfParty,
      testTemplate(inputs1, BatchFreeANDTestBody, false),
      getDummyEngineFactory,
      assertPartyResultsConsistent);
  auto rst3FreeAND = testHelper(
      numberOfParty,
      testTemplate(inputs3, BatchFreeANDTestBody),
      getDummyEngineFactory,
      assertPartyResultsConsistent);
  EXPECT_EQ(rst1FreeAND.size(), size / 2);
  EXPECT_EQ(rst3FreeAND.size(), size / 2);
}

class DummyNonFreeAndTestFixture
    : public ::testing::TestWithParam<std::tuple<
          std::string, // Human readable name
          size_t, // number of parties
          std::function<std::unique_ptr<DummySecretShareEngineFactory>(
              int myId,
              int numberOfParty,
              communication::IPartyCommunicationAgentFactory& agentFactory,
              std::shared_ptr<fbpcf::util::MetricCollector>
                  metricCollector)>>> {};

INSTANTIATE_TEST_SUITE_P(
    DummySecretShareEngineTest,
    DummyNonFreeAndTestFixture,
    ::testing::Values(
        std::make_tuple<
            std::string,
            size_t,
            std::function<std::unique_ptr<DummySecretShareEngineFactory>(
                int myId,
                int numberOfParty,
                communication::IPartyCommunicationAgentFactory& agentFactory,
                std::shared_ptr<fbpcf::util::MetricCollector>
                    metricCollector)>>("DummyEngine", 2, getDummyEngineFactory),
        std::make_tuple<
            std::string,
            size_t,
            std::function<std::unique_ptr<DummySecretShareEngineFactory>(
                int myId,
                int numberOfParty,
                communication::IPartyCommunicationAgentFactory& agentFactory,
                std::shared_ptr<fbpcf::util::MetricCollector>
                    metricCollector)>>(
            "DummyEngine",
            5,
            getDummyEngineFactory)),
    [](const testing::TestParamInfo<DummyNonFreeAndTestFixture::ParamType>&
           info) {
      return std::get<0>(info.param) + '_' +
          std::to_string(std::get<1>(info.param)) + "Party";
    });

TEST_P(DummyNonFreeAndTestFixture, TestNonFreeAnd) {
  size_t numberOfParty = std::get<1>(GetParam());
  size_t size = 16384;
  auto inputs = generateRandomInputs(numberOfParty, size, size);

  auto rst = testHelper(
      numberOfParty,
      testTemplate(inputs, ANDTestBody),
      std::get<2>(GetParam()),
      assertPartyResultsConsistent);
  auto andResult = std::get<0>(rst);
  auto compositeAndResult = std::get<1>(rst);
  ASSERT_EQ(andResult.size(), 2 * size);

  // Ordering of composite results
  // First size / 16 vectors is 1:15 runs (size 15)
  // Next size / 8 vectors is 1:7 runs (size 7)
  // Next size / 4 vectors is 1:3 runs (size 3)
  // Next 15 vectors is batch 1:15 run (batch size = size / 16)
  // Next 7 vectors is batch 1:7 run (batch size = size / 8)
  // Next 3 vectors is batch 1:3 run (batch size = size / 4)

  int composite15EndIndex = size / 16;
  int composite7EndIndex = composite15EndIndex + size / 8;
  int composite3EndIndex = composite7EndIndex + size / 4;
  int batchComposite15EndIndex = composite3EndIndex + 15;
  int batchComposite7EndIndex = batchComposite15EndIndex + 7;

  // First check 1:15 runs
  for (int i = 0; i < size / 16; i++) {
    EXPECT_EQ(compositeAndResult[i].size(), 15);
    for (int j = 0; j < 15; j++) {
      EXPECT_EQ(compositeAndResult[j + composite3EndIndex].size(), size / 16);
    }
  }

  // check 1:7 runs
  for (int i = 0; i < size / 8; i++) {
    EXPECT_EQ(compositeAndResult[i + composite15EndIndex].size(), 7);
    for (int j = 0; j < 7; j++) {
      EXPECT_EQ(
          compositeAndResult[j + batchComposite15EndIndex].size(), size / 8);
    }
  }

  // check 1:3 runs
  for (int i = 0; i < size / 4; i++) {
    EXPECT_EQ(compositeAndResult[i + composite7EndIndex].size(), 3);
    for (int j = 0; j < 3; j++) {
      EXPECT_EQ(
          compositeAndResult[j + batchComposite7EndIndex].size(), size / 4);
    }
  }
}

} // namespace fbpcf::engine
