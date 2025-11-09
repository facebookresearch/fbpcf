/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <cmath>
#include <future>
#include <memory>
#include <random>

#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/mpc_std_lib/unified_data_process/data_processor/DataProcessor.h"
#include "fbpcf/mpc_std_lib/unified_data_process/data_processor/DataProcessorFactory.h"
#include "fbpcf/mpc_std_lib/unified_data_process/data_processor/DummyDataProcessorFactory.h"
#include "fbpcf/mpc_std_lib/unified_data_process/data_processor/IDataProcessor.h"
#include "fbpcf/scheduler/SchedulerHelper.h"
#include "fbpcf/test/TestHelper.h"
#include "folly/Format.h"
#include "folly/Random.h"

namespace fbpcf::mpc_std_lib::unified_data_process::data_processor {

std::tuple<
    std::vector<std::vector<std::vector<uint8_t>>>,
    std::vector<uint64_t>,
    std::vector<std::vector<uint8_t>>>
generateDataProcessorTestData(size_t dataWidth, size_t numberOfShards) {
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<int32_t> randomSize(10, 0xFF);
  std::uniform_int_distribution<uint8_t> randomData(0, 0xFF);
  auto outputSize = randomSize(e);
  auto inputSize = outputSize + randomSize(e);

  std::vector<std::vector<uint8_t>> inputData(
      inputSize, std::vector<uint8_t>(dataWidth));

  for (auto& item : inputData) {
    for (auto& data : item) {
      data = randomData(e);
    }
  }
  std::vector<uint64_t> index(inputSize);
  for (size_t i = 0; i < inputSize; i++) {
    index[i] = i;
  }
  std::random_shuffle(index.begin(), index.end());
  index.erase(index.begin() + outputSize, index.end());

  std::vector<std::vector<uint8_t>> expectedOutput(outputSize);
  for (size_t i = 0; i < outputSize; i++) {
    expectedOutput[i] = inputData.at(index.at(i));
  }
  std::vector<std::vector<std::vector<uint8_t>>> shards(numberOfShards);
  for (size_t i = 0; i < numberOfShards; i++) {
    shards.at(i) = std::vector<std::vector<uint8_t>>(
        inputData.begin() + (i * inputData.size() / numberOfShards),
        inputData.begin() + ((i + 1) * inputData.size() / numberOfShards));
  }
  return {shards, index, expectedOutput};
}

void testDataProcessor(
    std::unique_ptr<IDataProcessor<0>> processor0,
    std::unique_ptr<IDataProcessor<1>> processor1) {
  auto [shard, index, expectedOutput] = generateDataProcessorTestData(20, 1);
  auto& data = shard.at(0);
  auto outputSize = index.size();
  auto dataSize = data.size();
  auto dataWidth = data.at(0).size();
  auto task0 = [](std::unique_ptr<IDataProcessor<0>> processor,
                  const std::vector<std::vector<unsigned char>>& plaintextData,
                  size_t outputSize,
                  size_t dataWidth) {
    auto secretSharedOutput =
        processor->processMyData(plaintextData, outputSize);
    auto plaintextOutputBitString =
        secretSharedOutput.openToParty(0).getValue();
    std::vector<std::vector<uint8_t>> rst(
        outputSize, std::vector<uint8_t>(dataWidth));
    for (size_t i = 0; i < dataWidth; i++) {
      for (uint8_t j = 0; j < 8; j++) {
        for (size_t k = 0; k < outputSize; k++) {
          rst[k][i] += (plaintextOutputBitString.at(i * 8 + j).at(k) << j);
        }
      }
    }
    return rst;
  };
  auto task1 = [](std::unique_ptr<IDataProcessor<1>> processor,
                  size_t dataSize,
                  const std::vector<uint64_t>& indexes,
                  size_t dataWidth) {
    auto secretSharedOutput =
        processor->processPeersData(dataSize, indexes, dataWidth);
    secretSharedOutput.openToParty(0);
  };

  auto future0 =
      std::async(task0, std::move(processor0), data, outputSize, dataWidth);
  auto future1 =
      std::async(task1, std::move(processor1), dataSize, index, dataWidth);
  future1.get();
  auto rst = future0.get();
  for (size_t i = 0; i < outputSize; i++) {
    fbpcf::testVectorEq(rst.at(i), expectedOutput.at(i));
  }
}

TEST(DummyDataProcessor, testDummyDataProcessor) {
  auto agentFactories = engine::communication::getInMemoryAgentFactory(2);
  setupRealBackend<0, 1>(*agentFactories[0], *agentFactories[1]);

  insecure::DummyDataProcessorFactory<0> factory0(0, 1, *agentFactories[0]);
  insecure::DummyDataProcessorFactory<1> factory1(1, 0, *agentFactories[1]);
  testDataProcessor(factory0.create(), factory1.create());
}

TEST(DataProcessor, testDataProcessor) {
  auto agentFactories = engine::communication::getInMemoryAgentFactory(2);
  setupRealBackend<0, 1>(*agentFactories[0], *agentFactories[1]);
  auto factory0 =
      getDataProcessorFactoryWithAesCtr<0>(0, 1, *agentFactories[0]);
  auto factory1 =
      getDataProcessorFactoryWithAesCtr<1>(1, 0, *agentFactories[1]);

  testDataProcessor(factory0->create(), factory1->create());
}

template <typename T>
std::tuple<std::vector<T>, std::vector<T>> split(
    const std::vector<T>& src,
    size_t cutPosition) {
  return {
      std::vector<T>(src.begin(), src.begin() + cutPosition),
      std::vector<T>(src.begin() + cutPosition, src.end())};
}

std::vector<std::vector<uint8_t>> convertToVecs(
    size_t rowCount,
    size_t dataWidth,
    const std::vector<std::vector<bool>>& src) {
  std::vector<std::vector<uint8_t>> rst(
      rowCount, std::vector<uint8_t>(dataWidth));

  for (size_t i = 0; i < dataWidth; i++) {
    for (uint8_t j = 0; j < 8; j++) {
      for (size_t k = 0; k < rowCount; k++) {
        rst.at(k).at(i) += (src.at(i * 8 + j).at(k) << j);
      }
    }
  }
  return rst;
}

void testUdpEncryptionAndDecryptionObjects(
    std::unique_ptr<engine::communication::IPartyCommunicationAgent> agent0,
    std::unique_ptr<engine::communication::IPartyCommunicationAgent> agent1) {
  int numberOfInputShards = 5;
  auto [shard, indexes, expectedOutput] =
      generateDataProcessorTestData(20, numberOfInputShards);
  auto outputSize = indexes.size();
  auto dataWidth = shard.at(0).at(0).size();
  std::vector<size_t> sizes(numberOfInputShards);
  for (size_t i = 0; i < numberOfInputShards; i++) {
    sizes.at(i) = shard.at(i).size();
  }

  auto udpEnc0 = std::make_unique<UdpEncryption>(std::move(agent0));
  auto udpEnc1 = std::make_unique<UdpEncryption>(std::move(agent1));

  auto udpDec00 = std::make_unique<UdpDecryption<0>>(0, 1);
  auto udpDec01 = std::make_unique<UdpDecryption<1>>(1, 0);

  auto udpDec10 = std::make_unique<UdpDecryption<2>>(0, 1);
  auto udpDec11 = std::make_unique<UdpDecryption<3>>(1, 0);

  auto task0 = [](std::unique_ptr<UdpEncryption> udpEnc,
                  std::unique_ptr<UdpDecryption<0>> udpDec0,
                  std::unique_ptr<UdpDecryption<2>> udpDec1,
                  const std::vector<std::vector<std::vector<unsigned char>>>&
                      plaintextDataInShards,
                  size_t dataWidth,
                  size_t outputSize,
                  const std::vector<uint64_t>& indexes_2,
                  const std::vector<size_t>& sizes) {
    udpEnc->prepareToProcessMyData(dataWidth);
    size_t myDataIndexOffset = 0;
    for (size_t i = 0; i < plaintextDataInShards.size(); i++) {
      std::vector<uint64_t> u64indexes(plaintextDataInShards.at(i).size());
      // generate 0 to n-1 vector
      std::iota(u64indexes.begin(), u64indexes.end(), myDataIndexOffset);
      myDataIndexOffset += plaintextDataInShards.at(i).size();
      udpEnc->processMyData(plaintextDataInShards.at(i), u64indexes);
    };
    udpEnc->prepareToProcessPeerData(dataWidth, indexes_2);
    for (size_t i = 0; i < sizes.size(); i++) {
      udpEnc->processPeerData(sizes.at(i));
    }

    size_t outputShard0Size = outputSize / 2;
    size_t outputShard1Size = outputSize - outputShard0Size;
    auto key = udpEnc->getExpandedKey();

    auto result0 = udpDec0->decryptMyData(key, dataWidth, outputShard0Size)
                       .openToParty(0)
                       .getValue();
    auto result1 = udpDec1->decryptMyData(key, dataWidth, outputShard1Size)
                       .openToParty(0)
                       .getValue();

    auto rst0 = convertToVecs(outputShard0Size, dataWidth, result0);
    auto rst1 = convertToVecs(outputShard1Size, dataWidth, result1);
    rst0.insert(rst0.end(), rst1.begin(), rst1.end());

    auto [intersection, nonces, pickedIndexes] = udpEnc->getProcessedData();

    auto [intersection0, intersection1] = split(intersection, outputShard0Size);
    auto [nonces0, nonces1] = split(nonces, outputShard0Size);
    auto [indexes0, indexes1] = split(pickedIndexes, outputShard0Size);

    auto rst2 = convertToVecs(
        outputShard0Size,
        dataWidth,
        udpDec0->decryptPeerData(intersection0, nonces0, indexes0)
            .openToParty(0)
            .getValue());
    auto rst3 = convertToVecs(
        outputShard1Size,
        dataWidth,
        udpDec1->decryptPeerData(intersection1, nonces1, indexes1)
            .openToParty(0)
            .getValue());
    rst2.insert(rst2.end(), rst3.begin(), rst3.end());
    return std::make_tuple(rst0, rst2);
  };

  auto task1 = [](std::unique_ptr<UdpEncryption> udpEnc,
                  std::unique_ptr<UdpDecryption<1>> udpDec0,
                  std::unique_ptr<UdpDecryption<3>> udpDec1,
                  const std::vector<uint64_t>& indexes_2,
                  const std::vector<size_t>& sizes,
                  const std::vector<std::vector<std::vector<unsigned char>>>&
                      plaintextDataInShards,
                  size_t dataWidth) {
    udpEnc->prepareToProcessPeerData(dataWidth, indexes_2);
    for (size_t i = 0; i < sizes.size(); i++) {
      udpEnc->processPeerData(sizes.at(i));
    }
    udpEnc->prepareToProcessMyData(dataWidth);
    size_t myDataIndexOffset = 0;
    for (size_t i = 0; i < plaintextDataInShards.size(); i++) {
      std::vector<uint64_t> u64indexes(plaintextDataInShards.at(i).size());
      // generate 0 to n-1 vector
      std::iota(u64indexes.begin(), u64indexes.end(), myDataIndexOffset);
      myDataIndexOffset += plaintextDataInShards.at(i).size();
      udpEnc->processMyData(plaintextDataInShards.at(i), u64indexes);
    };

    auto [intersection, nonces, pickedIndexes] = udpEnc->getProcessedData();

    size_t outputShard0Size = intersection.size() / 2;
    size_t outputShard1Size = intersection.size() - outputShard0Size;

    auto [intersection0, intersection1] = split(intersection, outputShard0Size);
    auto [nonces0, nonces1] = split(nonces, outputShard0Size);
    auto [indexes0, indexes1] = split(pickedIndexes, outputShard0Size);

    udpDec0->decryptPeerData(intersection0, nonces0, indexes0).openToParty(0);
    udpDec1->decryptPeerData(intersection1, nonces1, indexes1).openToParty(0);

    auto key = udpEnc->getExpandedKey();

    udpDec0->decryptMyData(key, dataWidth, outputShard0Size).openToParty(0);
    udpDec1->decryptMyData(key, dataWidth, outputShard1Size).openToParty(0);
  };

  auto future1 = std::async(
      task1,
      std::move(udpEnc1),
      std::move(udpDec01),
      std::move(udpDec11),
      indexes,
      sizes,
      shard,
      dataWidth);
  auto rst = task0(
      std::move(udpEnc0),
      std::move(udpDec00),
      std::move(udpDec10),
      shard,
      dataWidth,
      outputSize,
      indexes,
      sizes);
  future1.get();

  for (size_t i = 0; i < outputSize; i++) {
    fbpcf::testVectorEq(std::get<0>(rst).at(i), expectedOutput.at(i));
  }
  for (size_t i = 0; i < outputSize; i++) {
    fbpcf::testVectorEq(std::get<1>(rst).at(i), expectedOutput.at(i));
  }
}

TEST(DataProcessorSubObjects, testUdpEncryptionAndDecryptionObjects) {
  auto agentFactories = engine::communication::getInMemoryAgentFactory(2);
  setupRealBackend<0, 1>(*agentFactories[0], *agentFactories[1]);
  setupRealBackend<2, 3>(*agentFactories[0], *agentFactories[1]);
  testUdpEncryptionAndDecryptionObjects(
      agentFactories.at(0)->create(1, "sender"),
      agentFactories.at(1)->create(0, "receiver"));
}

TEST(TestFileReadAndWrite, testExpandedKeyReadAndWrite) {
  const int batchSize = 11;
  std::vector<__m128i> key(batchSize);
  for (size_t i = 0; i < batchSize; i++) {
    key.at(i) = _mm_set_epi32(
        folly::Random::rand32(),
        folly::Random::rand32(),
        folly::Random::rand32(),
        folly::Random::rand32());
  }
  const std::string expandedKeyFile =
      folly::sformat("./expanded_key_{}", folly::Random::rand32());

  writeExpandedKeyToFile(key, expandedKeyFile);
  auto key1 = readExpandedKeyFromFile(expandedKeyFile);
  std::remove(expandedKeyFile.c_str());
  fbpcf::testEq(key, key1);
}

TEST(TestFileReadAndWrite, testEncryptionResultReadAndWrite) {
  IUdpEncryption::EncryptionResults results;

  const int batchSize = 11;
  results.ciphertexts = std::vector<std::vector<unsigned char>>(batchSize);
  results.indexes = std::vector<uint64_t>(batchSize);
  results.nonces = std::vector<__m128i>(batchSize);
  for (size_t i = 0; i < batchSize; i++) {
    results.nonces.at(i) = _mm_set_epi32(
        folly::Random::rand32(),
        folly::Random::rand32(),
        folly::Random::rand32(),
        folly::Random::rand32());
    results.indexes.at(i) = folly::Random::rand32();
    results.ciphertexts.at(i) = std::vector<unsigned char>(60);
    for (size_t j = 0; j < results.ciphertexts.size(); j++) {
      results.ciphertexts.at(i).at(j) = i + j;
    }
  }
  const std::string resultFile =
      folly::sformat("./encryptionResult_{}", folly::Random::rand32());

  writeEncryptionResultsToFile(results, resultFile);
  auto results1 = readEncryptionResultsFromFile(resultFile);
  std::remove(resultFile.c_str());
  fbpcf::testEq(results.nonces, results1.nonces);
  fbpcf::testVectorEq(results.indexes, results1.indexes);
  for (size_t i = 0; i < batchSize; i++) {
    fbpcf::testVectorEq(results.ciphertexts.at(i), results1.ciphertexts.at(i));
  }
}

TEST(TestSplit, getShardSize) {
  EXPECT_EQ(getShardSize(11, 0, 3), 3);
  EXPECT_EQ(getShardSize(11, 1, 3), 4);
  EXPECT_EQ(getShardSize(11, 2, 3), 4);
}

TEST(TestSplit, testSplit) {
  IUdpEncryption::EncryptionResults results;
  int batchSize = 11;
  results.ciphertexts = std::vector<std::vector<unsigned char>>(batchSize);
  results.indexes = std::vector<uint64_t>(batchSize);
  results.nonces = std::vector<__m128i>(batchSize);
  for (size_t i = 0; i < batchSize; i++) {
    results.nonces.at(i) = _mm_set_epi32(
        folly::Random::rand32(),
        folly::Random::rand32(),
        folly::Random::rand32(),
        folly::Random::rand32());
    results.indexes.at(i) = folly::Random::rand32();
    results.ciphertexts.at(i) = std::vector<unsigned char>(60);
    for (size_t j = 0; j < results.ciphertexts.size(); j++) {
      results.ciphertexts.at(i).at(j) = i + j;
    }
  }
  auto resultsVec = splitEncryptionResults(results, 3);
  for (auto& it : resultsVec) {
    EXPECT_EQ(it.ciphertexts.size(), it.nonces.size());
    EXPECT_EQ(it.ciphertexts.size(), it.indexes.size());
  }
  EXPECT_EQ(resultsVec.at(0).ciphertexts.size(), 3);
  EXPECT_EQ(resultsVec.at(1).ciphertexts.size(), 4);
  EXPECT_EQ(resultsVec.at(2).ciphertexts.size(), 4);
}

} // namespace fbpcf::mpc_std_lib::unified_data_process::data_processor
