/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emmintrin.h>
#include <fbpcf/engine/util/util.h>
#include "fbpcf/engine/util/aes.h"
#include "fbpcf/mpc_std_lib/aes_circuit/AesCircuitCtr.h"
#include "fbpcf/mpc_std_lib/unified_data_process/data_processor/DataProcessor.h"

#include "fbpcf/mpc_std_lib/unified_data_process/data_processor/UdpUtil.h"

namespace fbpcf::mpc_std_lib::unified_data_process::data_processor {

template <int schedulerId>
typename IDataProcessor<schedulerId>::SecString
DataProcessor<schedulerId>::processMyData(
    const std::vector<std::vector<unsigned char>>& plaintextData,
    size_t outputSize) {
  size_t dataSize = plaintextData.size();
  size_t dataWidth = plaintextData.at(0).size();

  encrypter_.prepareToProcessMyData(dataWidth);
  encrypter_.processMyData(plaintextData);

  // 1b. (peer)receive encryted data from peer
  // 2b. (peer)pick desired ciphertext blocks
  // 3a. share key
  std::vector<__m128i> expandedKeyVectorM128i = encrypter_.getExpandedKey();
  auto keyString = UdpUtil::privatelyShareExpandedKey<schedulerId>(
      expandedKeyVectorM128i, outputSize, myId_);

  // 3b. (peer)share ciphertext and mask
  std::vector<std::vector<unsigned char>> ciphertextPlaceholder(
      outputSize, std::vector<unsigned char>(dataWidth));
  auto filteredCiphertext = UdpUtil::privatelyShareByteStream<schedulerId>(
      ciphertextPlaceholder, partnerId_);

  std::vector<std::vector<__m128i>> countersPlaceholderM128i(
      outputSize, std::vector<__m128i>(filteredCiphertext.size() / 128));
  auto filteredCounters = UdpUtil::privatelyShareM128iStream<schedulerId>(
      countersPlaceholderM128i, partnerId_);

  // 4a/b. decryt the data jointly (input my key privately)
  auto decryptData =
      aesCircuitCtr_->decrypt(filteredCiphertext, keyString, filteredCounters);

  // reverse each byte from little endian into big endian order
  std::vector<typename IDataProcessor<schedulerId>::SecBit> reversedData(
      decryptData.size());
  for (int i = 0; i < decryptData.size(); ++i) {
    reversedData[8 * (i / 8) + (7 - i % 8)] = decryptData[i];
  }
  // 5a/b. output decrypted data
  // remove the trailing padding bits
  typename IDataProcessor<schedulerId>::SecString outputShare(dataWidth * 8);
  for (size_t i = 0; i < dataWidth * 8; ++i) {
    outputShare[i] = reversedData[i];
  }
  return outputShare;
}

template <int schedulerId>
typename IDataProcessor<schedulerId>::SecString
DataProcessor<schedulerId>::processPeersData(
    size_t dataSize,
    const std::vector<int32_t>& indexes,
    size_t dataWidth) {
  encrypter_.prepareToProcessPeerData(dataWidth, indexes);
  encrypter_.processPeerData(dataSize);

  auto [intersection, nonces, _] = encrypter_.getProcessedData();

  size_t intersectionSize = intersection.size();
  // 3a. (peer)share key
  std::vector<__m128i> keyPlaceholderM128i(11);
  auto keyString = UdpUtil::privatelyShareExpandedKey<schedulerId>(
      keyPlaceholderM128i, intersectionSize, partnerId_);

  // 3b. share ciphertext and mask
  size_t cipherWidth =
      dataWidth % 16 == 0 ? dataWidth : dataWidth + 16 - dataWidth % 16;
  size_t cipherBlocks = cipherWidth / 16;
  auto filteredCiphertext =
      UdpUtil::privatelyShareByteStream<schedulerId>(intersection, myId_);

  std::vector<std::vector<__m128i>> filteredCountersM128i(
      intersectionSize, std::vector<__m128i>(cipherBlocks));
  for (uint64_t i = 0; i < intersectionSize; ++i) {
    for (uint64_t j = 0; j < cipherBlocks; ++j) {
      filteredCountersM128i[i][j] =
          _mm_set_epi64x(0, indexes[i] * cipherBlocks + j);
      filteredCountersM128i[i][j] =
          _mm_add_epi64(nonces.at(0), filteredCountersM128i[i][j]);
    }
  }
  auto filteredCounters = UdpUtil::privatelyShareM128iStream<schedulerId>(
      filteredCountersM128i, myId_);

  // 4a/b. decryt the picked blocks jointly (input the ciphertext and mask
  // privately)
  auto decryptData =
      aesCircuitCtr_->decrypt(filteredCiphertext, keyString, filteredCounters);

  // reverse each byte from little endian into big endian order
  std::vector<typename IDataProcessor<schedulerId>::SecBit> reversedData(
      decryptData.size());
  for (size_t i = 0; i < decryptData.size(); ++i) {
    reversedData[8 * (i / 8) + (7 - i % 8)] = decryptData[i];
  }

  // 5a/b. output decrypted data
  // remove the trailing padding bits
  typename IDataProcessor<schedulerId>::SecString outputShare(dataWidth * 8);
  for (size_t i = 0; i < dataWidth * 8; ++i) {
    outputShare[i] = reversedData[i];
  }
  return outputShare;
}

} // namespace fbpcf::mpc_std_lib::unified_data_process::data_processor
