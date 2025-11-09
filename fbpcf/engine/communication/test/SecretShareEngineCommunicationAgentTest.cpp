/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/engine/communication/SecretShareEngineCommunicationAgent.h"
#include <emmintrin.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <smmintrin.h>
#include <random>
#include <thread>
#include <vector>
#include "fbpcf/engine/communication/test/SecretShareEngineCommunicationAgentTestHelper.h"

namespace fbpcf::engine::communication {

void exchangeKeyTest(
    std::unique_ptr<SecretShareEngineCommunicationAgent> agent,
    int myId,
    int numberOfParty) {
  std::map<int, __m128i> keys;
  for (int i = 0; i < numberOfParty; i++) {
    if (i != myId) {
      keys.emplace(i, _mm_set_epi32(myId, 0, 0, i));
    }
  }
  auto receivedKeys = agent->exchangeKeys(keys);
  EXPECT_EQ(receivedKeys.size(), numberOfParty - 1);
  for (int i = 0; i < numberOfParty; i++) {
    if (i != myId) {
      EXPECT_NE(receivedKeys.find(i), receivedKeys.end());
      auto key = receivedKeys.at(i);
      EXPECT_EQ(i, _mm_extract_epi32(key, 3));
      EXPECT_EQ(0, _mm_extract_epi32(key, 2));
      EXPECT_EQ(0, _mm_extract_epi32(key, 1));
      EXPECT_EQ(myId, _mm_extract_epi32(key, 0));
    }
  }
}

TEST(secretShareEngineCommunicationAgentTest, testExchangeKey) {
  SecretShareEngineCommunicationAgentTestHelper helper;
  int numberOfParty = 4;
  auto agents = helper.createAgents(numberOfParty);
  std::vector<std::thread> threads;
  for (int i = 0; i < numberOfParty; i++) {
    threads.push_back(
        std::thread(exchangeKeyTest, std::move(agents[i]), i, numberOfParty));
  }
  for (int i = 0; i < numberOfParty; i++) {
    threads[i].join();
  }
}

void openSecretToAllTest(
    std::unique_ptr<SecretShareEngineCommunicationAgent> agent,
    int myId,
    int numberOfParty,
    const std::vector<bool>& secrets,
    const std::vector<bool>& expections) {
  auto openedSecrets = agent->openSecretsToAll(secrets);
  EXPECT_EQ(openedSecrets.size(), secrets.size());
  for (int i = 0; i < secrets.size(); i++) {
    EXPECT_EQ(openedSecrets[i], expections[i]);
  }
}

TEST(secretShareEngineCommunicationAgentTest, testOpenToAll) {
  SecretShareEngineCommunicationAgentTestHelper helper;
  int numberOfParty = 4;
  int size = 131;

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint8_t> dist(0, 1);

  std::vector<std::vector<bool>> secrets(numberOfParty);
  std::vector<bool> plaintext;
  for (int i = 0; i < size; i++) {
    plaintext.push_back(false);
    for (int j = 0; j < numberOfParty; j++) {
      secrets[j].push_back(dist(e));
      plaintext[i] = plaintext[i] ^ secrets[j][i];
    }
  }

  auto agents = helper.createAgents(numberOfParty);
  std::vector<std::thread> threads;
  for (int i = 0; i < numberOfParty; i++) {
    threads.push_back(std::thread(
        openSecretToAllTest,
        std::move(agents[i]),
        i,
        numberOfParty,
        secrets[i],
        plaintext));
  }
  for (int i = 0; i < numberOfParty; i++) {
    threads[i].join();
  }
}

void openSecretToPartyTest(
    std::unique_ptr<SecretShareEngineCommunicationAgent> agent,
    int myId,
    int numberOfParty,
    int receivingParty,
    const std::vector<bool>& secrets,
    const std::vector<bool>& expections) {
  auto openedSecrets = agent->openSecretsToParty(receivingParty, secrets);
  ASSERT_EQ(openedSecrets.size(), secrets.size());
  if (receivingParty == myId) {
    for (int i = 0; i < secrets.size(); i++) {
      EXPECT_EQ(openedSecrets[i], expections[i]);
    }
  } else {
    for (int i = 0; i < secrets.size(); i++) {
      EXPECT_EQ(openedSecrets[i], false);
    }
  }
}

TEST(secretShareEngineCommunicationAgentTest, testOpenToParty) {
  SecretShareEngineCommunicationAgentTestHelper helper;
  int numberOfParty = 4;
  int size = 131;

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint8_t> dist(0, 1);

  std::vector<std::vector<bool>> secrets(numberOfParty);
  std::vector<bool> plaintext;
  for (int i = 0; i < size; i++) {
    plaintext.push_back(false);
    for (int j = 0; j < numberOfParty; j++) {
      secrets[j].push_back(dist(e));
      plaintext[i] = plaintext[i] ^ secrets[j][i];
    }
  }

  auto agents = helper.createAgents(numberOfParty);
  std::vector<std::thread> threads;
  for (int i = 0; i < numberOfParty; i++) {
    threads.push_back(std::thread(
        openSecretToPartyTest,
        std::move(agents[i]),
        i,
        numberOfParty,
        0,
        secrets[i],
        plaintext));
  }
  for (int i = 0; i < numberOfParty; i++) {
    threads[i].join();
  }
}

void openIntegerSecretToPartyTest(
    std::unique_ptr<SecretShareEngineCommunicationAgent> agent,
    int myId,
    int numberOfParty,
    int receivingParty,
    const std::vector<uint64_t>& secrets,
    const std::vector<uint64_t>& expections) {
  auto openedSecrets = agent->openSecretsToParty(receivingParty, secrets);
  ASSERT_EQ(openedSecrets.size(), secrets.size());
  if (receivingParty == myId) {
    for (int i = 0; i < secrets.size(); i++) {
      EXPECT_EQ(openedSecrets[i], expections[i]);
    }
  } else {
    for (int i = 0; i < secrets.size(); i++) {
      EXPECT_EQ(openedSecrets[i], 0);
    }
  }
}

TEST(secretShareEngineCommunicationAgentTest, testOpenToPartyForIntegers) {
  SecretShareEngineCommunicationAgentTestHelper helper;
  int numberOfParty = 4;
  int size = 131;

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint64_t> dist;

  std::vector<std::vector<uint64_t>> secrets(numberOfParty);
  std::vector<uint64_t> plaintext;
  for (int i = 0; i < size; i++) {
    plaintext.push_back(0);
    for (int j = 0; j < numberOfParty; j++) {
      secrets[j].push_back(dist(e));
      plaintext[i] = plaintext[i] + secrets[j][i];
    }
  }

  auto agents = helper.createAgents(numberOfParty);
  std::vector<std::thread> threads;
  for (int i = 0; i < numberOfParty; i++) {
    threads.push_back(std::thread(
        openIntegerSecretToPartyTest,
        std::move(agents[i]),
        i,
        numberOfParty,
        0,
        secrets[i],
        plaintext));
  }
  for (int i = 0; i < numberOfParty; i++) {
    threads[i].join();
  }
}
} // namespace fbpcf::engine::communication
