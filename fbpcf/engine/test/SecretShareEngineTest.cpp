/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <regex.h>
#include <functional>
#include <future>
#include <memory>
#include <random>
#include <thread>
#include <utility>
#include <vector>

#include "fbpcf/engine/ISecretShareEngine.h"
#include "fbpcf/engine/SecretShareEngine.h"
#include "fbpcf/engine/SecretShareEngineFactory.h"
#include "fbpcf/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"

namespace fbpcf::engine {

template <typename T>
T testHelper(
    int numberOfParty,
    std::function<
        T(std::unique_ptr<ISecretShareEngine> engine,
          int myId,
          int numberOfParty)> test,
    std::function<std::unique_ptr<SecretShareEngineFactory>(
        int myId,
        int numberOfParty,
        communication::IPartyCommunicationAgentFactory& agentFactory)>
        engineFactory,
    void (*assertPartyResultsConsistent)(T base, T comparison)) {
  auto agentFactories = communication::getInMemoryAgentFactory(numberOfParty);

  std::vector<std::future<T>> futures;
  for (auto i = 0; i < numberOfParty; ++i) {
    futures.push_back(std::async(
        [i, numberOfParty, test, engineFactory](
            std::reference_wrapper<
                communication::IPartyCommunicationAgentFactory> agentFactory) {
          auto engine = engineFactory(i, numberOfParty, agentFactory)->create();
          return test(std::move(engine), i, numberOfParty);
        },
        std::reference_wrapper<communication::IPartyCommunicationAgentFactory>(
            *agentFactories.at(i))));
  }
  auto rst = futures[0].get();

  for (auto i = 1; i < numberOfParty; ++i) {
    auto tmp = futures[i].get();
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

std::pair<std::vector<bool>, std::vector<std::vector<bool>>> ANDTestBody(
    ISecretShareEngine& engine,
    const std::vector<bool>& inputs) {
  auto size = inputs.size();
  EXPECT_EQ(size % 16, 0);

  // regular AND's
  std::vector<int> regularANDIndex(size / 2);
  for (int i = 0; i < size / 2; i++) {
    regularANDIndex[i] = engine.scheduleAND(inputs[i], inputs[i + size / 2]);
  }

  // batch AND's

  auto firstHalfInput =
      std::vector<bool>(inputs.begin(), inputs.begin() + size / 2);

  auto secondHalfInput =
      std::vector<bool>(inputs.begin() + size / 2, inputs.end());
  engine.computeBatchANDImmediately(firstHalfInput, secondHalfInput);

  auto batchIndex0 = engine.scheduleBatchAND(firstHalfInput, secondHalfInput);
  auto batchIndex1 = engine.scheduleBatchAND(firstHalfInput, secondHalfInput);

  std::vector<bool> leftComposite15 =
      std::vector<bool>(inputs.begin(), inputs.begin() + size / 16);
  std::vector<std::vector<bool>> rightComposite15(15);

  for (int i = 0; i < size / 16; i++) {
    for (int j = 0; j < 15; j++) {
      rightComposite15[j].push_back(inputs[size / 16 + 15 * i + j]);
    }
  }

  std::vector<bool> leftComposite7 =
      std::vector<bool>(inputs.begin(), inputs.begin() + size / 8);
  std::vector<std::vector<bool>> rightComposite7(7);

  for (int i = 0; i < size / 8; i++) {
    for (int j = 0; j < 7; j++) {
      rightComposite7[j].push_back(inputs[size / 8 + 7 * i + j]);
    }
  }

  std::vector<bool> leftComposite3 =
      std::vector<bool>(inputs.begin(), inputs.begin() + size / 4);
  std::vector<std::vector<bool>> rightComposite3(3);

  for (int i = 0; i < size / 4; i++) {
    for (int j = 0; j < 3; j++) {
      rightComposite3[j].push_back(inputs[size / 4 + 3 * i + j]);
    }
  }

  // schedule composite runs by running through batch structures individually
  std::vector<int> compositeANDIndex(size / 16 + size / 8 + size / 4);
  for (int i = 0; i < size / 16; i++) {
    std::vector<bool> rights(15);
    for (int j = 0; j < 15; j++) {
      rights[j] = rightComposite15[j][i];
    }
    compositeANDIndex[i] =
        engine.scheduleCompositeAND(leftComposite15[i], rights);
  }

  for (int i = 0; i < size / 8; i++) {
    std::vector<bool> rights(7);
    for (int j = 0; j < 7; j++) {
      rights[j] = rightComposite7[j][i];
    }
    compositeANDIndex[size / 16 + i] =
        engine.scheduleCompositeAND(leftComposite7[i], rights);
  }
  for (int i = 0; i < size / 4; i++) {
    std::vector<bool> rights(3);
    for (int j = 0; j < 3; j++) {
      rights[j] = rightComposite3[j][i];
    }
    compositeANDIndex[size / 16 + size / 8 + i] =
        engine.scheduleCompositeAND(leftComposite3[i], rights);
  }

  // schedule batch composite runs
  auto batchCompositeIndex0 =
      engine.scheduleBatchCompositeAND(leftComposite15, rightComposite15);
  auto batchCompositeIndex1 =
      engine.scheduleBatchCompositeAND(leftComposite7, rightComposite7);
  auto batchCompositeIndex2 =
      engine.scheduleBatchCompositeAND(leftComposite3, rightComposite3);

  engine.executeScheduledAND();

  // Regular AND
  std::vector<bool> andResult(size / 2);
  for (size_t i = 0; i < size / 2; i++) {
    andResult[i] = engine.getANDExecutionResult(regularANDIndex[i]);
  }
  auto tmp = engine.getBatchANDExecutionResult(batchIndex0);
  andResult.insert(andResult.end(), tmp.begin(), tmp.end());

  tmp = engine.getBatchANDExecutionResult(batchIndex1);
  andResult.insert(andResult.end(), tmp.begin(), tmp.end());

  tmp = engine.computeBatchANDImmediately(firstHalfInput, secondHalfInput);
  andResult.insert(andResult.end(), tmp.begin(), tmp.end());

  // Composite AND
  std::vector<std::vector<bool>> compositeAndResult;
  for (auto compositeIndex : compositeANDIndex) {
    tmp = engine.getCompositeANDExecutionResult(compositeIndex);
    compositeAndResult.push_back(tmp);
  }

  auto tmp2 = engine.getBatchCompositeANDExecutionResult(batchCompositeIndex0);
  compositeAndResult.insert(compositeAndResult.end(), tmp2.begin(), tmp2.end());

  tmp2 = engine.getBatchCompositeANDExecutionResult(batchCompositeIndex1);
  compositeAndResult.insert(compositeAndResult.end(), tmp2.begin(), tmp2.end());

  tmp2 = engine.getBatchCompositeANDExecutionResult(batchCompositeIndex2);
  compositeAndResult.insert(compositeAndResult.end(), tmp2.begin(), tmp2.end());

  return std::make_pair(andResult, compositeAndResult);
}

class NonFreeAndTestFixture
    : public ::testing::TestWithParam<std::tuple<
          std::string, // Human readable name
          size_t, // number of parties
          std::function<std::unique_ptr<SecretShareEngineFactory>(
              int myId,
              int numberOfParty,
              communication::IPartyCommunicationAgentFactory& agentFactory)>>> {
};

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
                communication::IPartyCommunicationAgentFactory& agentFactory)>>(
            "InsecureEngineWithDummyTupleGenerator",
            2,
            getInsecureEngineFactoryWithDummyTupleGenerator),
        std::make_tuple<
            std::string,
            size_t,
            std::function<std::unique_ptr<SecretShareEngineFactory>(
                int myId,
                int numberOfParty,
                communication::IPartyCommunicationAgentFactory& agentFactory)>>(
            "InsecureEngineWithDummyTupleGenerator",
            4,
            getInsecureEngineFactoryWithDummyTupleGenerator),
        std::make_tuple<
            std::string,
            size_t,
            std::function<std::unique_ptr<SecretShareEngineFactory>(
                int myId,
                int numberOfParty,
                communication::IPartyCommunicationAgentFactory& agentFactory)>>(
            "SecureEngineWithFerret",
            2,
            getSecureEngineFactoryWithFERRET<bool>),
        std::make_tuple<
            std::string,
            size_t,
            std::function<std::unique_ptr<SecretShareEngineFactory>(
                int myId,
                int numberOfParty,
                communication::IPartyCommunicationAgentFactory& agentFactory)>>(
            "SecureEngineWithFerret",
            3,
            getSecureEngineFactoryWithFERRET<bool>)),
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
    EXPECT_EQ(andResult[i], inputs[i].first & inputs[i + size / 2].first);
    EXPECT_EQ(
        andResult[i + size / 2], inputs[i].first & inputs[i + size / 2].first);
    EXPECT_EQ(
        andResult[i + size], inputs[i].first & inputs[i + size / 2].first);
    EXPECT_EQ(
        andResult[i + size / 2 + size],
        inputs[i].first & inputs[i + size / 2].first);
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
          inputs[i].first & inputs[size / 16 + 15 * i + j].first);
      EXPECT_EQ(
          compositeAndResult[j + composite3EndIndex][i],
          inputs[i].first & inputs[size / 16 + 15 * i + j].first);
    }
  }

  for (int i = 0; i < size / 8; i++) {
    for (int j = 0; j < 7; j++) {
      EXPECT_EQ(
          compositeAndResult[i + composite15EndIndex][j],
          inputs[i].first & inputs[size / 8 + 7 * i + j].first);
      EXPECT_EQ(
          compositeAndResult[j + batchComposite15EndIndex][i],
          inputs[i].first & inputs[size / 8 + 7 * i + j].first);
    }
  }

  // check 1:3 runs
  for (int i = 0; i < size / 4; i++) {
    for (int j = 0; j < 3; j++) {
      EXPECT_EQ(
          compositeAndResult[i + composite7EndIndex][j],
          inputs[i].first & inputs[size / 4 + 3 * i + j].first);
      EXPECT_EQ(
          compositeAndResult[j + batchComposite7EndIndex][i],
          inputs[i].first & inputs[size / 4 + 3 * i + j].first);
    }
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
    EXPECT_EQ(rst1[i], inputs1[i].first & inputs1[i + size / 2].first);
    EXPECT_EQ(rst2[i], inputs2[i].first & inputs2[i + size / 2].first);
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
    EXPECT_EQ(rst1[i], inputs1[i].first & inputs1[i + size / 2].first);
    EXPECT_EQ(rst2[i], inputs2[i].first & inputs2[i + size / 2].first);
  }
}

} // namespace fbpcf::engine
