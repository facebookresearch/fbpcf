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
#include <vector>

#include "fbpcf/engine/ISecretShareEngine.h"
#include "fbpcf/engine/SecretShareEngine.h"
#include "fbpcf/engine/SecretShareEngineFactory.h"
#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"

namespace fbpcf::engine {

std::vector<bool> testHelper(
    int numberOfParty,
    std::function<std::vector<bool>(
        std::unique_ptr<ISecretShareEngine> engine,
        int myId,
        int numberOfParty)> test) {
  auto agentFactories = communication::getInMemoryAgentFactory(numberOfParty);

  std::vector<std::future<std::vector<bool>>> futures;
  for (auto i = 0; i < numberOfParty; ++i) {
    futures.push_back(std::async(
        [i, numberOfParty, test](
            std::reference_wrapper<
                communication::IPartyCommunicationAgentFactory> agentFactory) {
          auto engine = getInsecureEngineFactoryWithDummyTupleGenerator(
                            i, numberOfParty, agentFactory)
                            ->create();
          return test(std::move(engine), i, numberOfParty);
        },
        std::reference_wrapper<communication::IPartyCommunicationAgentFactory>(
            *agentFactories.at(i))));
  }
  auto rst = futures[0].get();
  for (auto i = 1; i < numberOfParty; ++i) {
    auto tmp = futures[i].get();
    EXPECT_EQ(tmp.size(), rst.size());
  }
  return rst;
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

  auto rst =
      testHelper(numberOfParty, testTemplate(inputs, inputAndOutputTestBody));
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
      numberOfParty, testTemplate(inputs1, symmetricNOTTestBody, false));
  auto rst2 =
      testHelper(numberOfParty, testTemplate(inputs2, asymmetricNOTTestBody));
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
      numberOfParty, testTemplate(inputs1, batchSymmetricNOTTestBody, false));
  auto rst2 = testHelper(
      numberOfParty, testTemplate(inputs2, batchAsymmetricNOTTestBody));
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

TEST(SecretShareEngineTest, TestXORtWithDummyComponents) {
  int numberOfParty = 4;
  int size = 16384;
  auto inputs1 = generateRandomInputs(numberOfParty, size, size);
  auto inputs2 = generateRandomInputs(numberOfParty, size, size / 2);
  auto inputs3 = generateRandomInputs(numberOfParty, size, 0);
  auto rst1 =
      testHelper(numberOfParty, testTemplate(inputs1, symmetricXORTestBody));
  auto rst2 =
      testHelper(numberOfParty, testTemplate(inputs2, asymmetricXORTestBody));
  auto rst3 = testHelper(
      numberOfParty, testTemplate(inputs3, symmetricXORTestBody, false));
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

TEST(SecretShareEngineTest, TestBatchXORtWithDummyComponents) {
  int numberOfParty = 4;
  int size = 16384;
  auto inputs1 = generateRandomInputs(numberOfParty, size, size);
  auto inputs2 = generateRandomInputs(numberOfParty, size, size / 2);
  auto inputs3 = generateRandomInputs(numberOfParty, size, 0);

  auto rst1 = testHelper(
      numberOfParty, testTemplate(inputs1, batchSymmetricXORTestBody));
  auto rst2 = testHelper(
      numberOfParty, testTemplate(inputs2, batchAsymmetricXORTestBody));
  auto rst3 = testHelper(
      numberOfParty, testTemplate(inputs3, batchSymmetricXORTestBody, false));
  EXPECT_EQ(rst1.size(), size / 2);
  EXPECT_EQ(rst2.size(), size / 2);
  EXPECT_EQ(rst3.size(), size / 2);

  for (int i = 0; i < size / 2; i++) {
    EXPECT_EQ(rst1[i], inputs1[i].first ^ inputs1[i + size / 2].first);
    EXPECT_EQ(rst2[i], inputs2[i].first ^ inputs2[i + size / 2].first);
    EXPECT_EQ(rst3[i], inputs3[i].first ^ inputs3[i + size / 2].first);
  }
}

std::vector<bool> ANDTestBody(
    ISecretShareEngine& engine,
    const std::vector<bool>& inputs) {
  auto size = inputs.size();
  EXPECT_EQ(size % 2, 0);

  std::vector<int> index(size / 2);
  for (int i = 0; i < size / 2; i++) {
    index[i] = engine.scheduleAND(inputs[i], inputs[i + size / 2]);
  }

  auto firstHalfInput =
      std::vector<bool>(inputs.begin(), inputs.begin() + size / 2);

  auto secondHalfInput =
      std::vector<bool>(inputs.begin() + size / 2, inputs.end());
  engine.computeBatchAND(firstHalfInput, secondHalfInput);

  auto batchIndex0 = engine.scheduleBatchAND(firstHalfInput, secondHalfInput);
  auto batchIndex1 = engine.scheduleBatchAND(firstHalfInput, secondHalfInput);

  engine.executeScheduledAND();

  std::vector<bool> rst(size / 2);
  for (size_t i = 0; i < size / 2; i++) {
    rst[i] = engine.getANDExecutionResult(index[i]);
  }
  auto tmp = engine.getBatchANDExecutionResult(batchIndex0);
  rst.insert(rst.end(), tmp.begin(), tmp.end());

  tmp = engine.getBatchANDExecutionResult(batchIndex1);
  rst.insert(rst.end(), tmp.begin(), tmp.end());

  tmp = engine.computeBatchAND(firstHalfInput, secondHalfInput);
  rst.insert(rst.end(), tmp.begin(), tmp.end());
  return rst;
}

TEST(SecretShareEngineTest, TestANDtWithDummyComponents) {
  int numberOfParty = 4;
  int size = 16384;
  auto inputs = generateRandomInputs(numberOfParty, size, size);

  auto rst = testHelper(numberOfParty, testTemplate(inputs, ANDTestBody));
  ASSERT_EQ(rst.size(), size * 2);
  for (int i = 0; i < size / 2; i++) {
    EXPECT_EQ(rst[i], inputs[i].first & inputs[i + size / 2].first);
    EXPECT_EQ(rst[i + size / 2], inputs[i].first & inputs[i + size / 2].first);
    EXPECT_EQ(rst[i + size], inputs[i].first & inputs[i + size / 2].first);
    EXPECT_EQ(
        rst[i + size / 2 + size], inputs[i].first & inputs[i + size / 2].first);
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

TEST(SecretShareEngineTest, TestFreeANDtWithDummyComponents) {
  int numberOfParty = 4;
  int size = 16384;
  auto inputs1 = generateRandomInputs(numberOfParty, size, size / 2);
  auto inputs2 = generateRandomInputs(numberOfParty, size, 0);

  auto rst1 = testHelper(numberOfParty, testTemplate(inputs1, FreeANDTestBody));
  auto rst2 =
      testHelper(numberOfParty, testTemplate(inputs2, FreeANDTestBody, false));
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

TEST(SecretShareEngineTest, TestBatchFreeANDtWithDummyComponents) {
  int numberOfParty = 4;
  int size = 16384;
  auto inputs1 = generateRandomInputs(numberOfParty, size, size / 2);
  auto inputs2 = generateRandomInputs(numberOfParty, size, 0);
  auto rst1 =
      testHelper(numberOfParty, testTemplate(inputs1, BatchFreeANDTestBody));
  auto rst2 = testHelper(
      numberOfParty, testTemplate(inputs2, BatchFreeANDTestBody, false));
  EXPECT_EQ(rst1.size(), size / 2);
  for (int i = 0; i < size / 2; i++) {
    EXPECT_EQ(rst1[i], inputs1[i].first & inputs1[i + size / 2].first);
    EXPECT_EQ(rst2[i], inputs2[i].first & inputs2[i + size / 2].first);
  }
}

} // namespace fbpcf::engine
