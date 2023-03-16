/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emmintrin.h>
#include <sys/types.h>
#include "fbpcf/frontend/BitString.h"
#include "fbpcf/mpc_std_lib/unified_data_process/data_processor/IUdpEncryption.h"
#include "fbpcf/primitive/mac/S2v.h"
#include "fbpcf/primitive/mac/S2vFactory.h"

namespace fbpcf::mpc_std_lib::unified_data_process::data_processor {

class UdpUtil {
  template <int schedulerId>
  using SecBit = frontend::Bit<true, schedulerId, true>;

  static const size_t kBlockSize = 16;

 public:
  static std::pair<std::vector<std::vector<uint8_t>>, std::vector<uint8_t>>
  localEncryption(
      const std::vector<std::vector<unsigned char>>& plaintextData,
      __m128i prgKey,
      uint64_t indexOffset = 0);

  static std::vector<__m128i>
  generateCounterBlocks(__m128i nonce, uint64_t startingIndex, size_t size);

  template <int schedulerId>
  static std::vector<SecBit<schedulerId>> privatelyShareByteStream(
      const std::vector<std::vector<unsigned char>>& localData,
      int inputPartyID);

  template <int schedulerId>
  static std::vector<SecBit<schedulerId>> privatelyShareM128iStream(
      const std::vector<std::vector<__m128i>>& localDataM128i,
      int inputPartyID);

  template <int schedulerId>
  static std::vector<SecBit<schedulerId>> privatelyShareExpandedKey(
      const std::vector<__m128i>& localKeyM128i,
      size_t batchSize,
      int inputPartyID);
};

void writeEncryptionResultsToFile(
    const IUdpEncryption::EncryptionResults& encryptionResults,
    const std::string& file);

void writeExpandedKeyToFile(
    const std::vector<__m128i>& expandedKey,
    const std::string& file);

std::vector<__m128i> readExpandedKeyFromFile(const std::string& file);

IUdpEncryption::EncryptionResults readEncryptionResultsFromFile(
    const std::string& file);

std::vector<IUdpEncryption::EncryptionResults> splitEncryptionResults(
    const IUdpEncryption::EncryptionResults& encryptionResults,
    int count);

// decides the i-th shard size. i is the shard index
inline size_t
getShardSize(size_t totalCount, size_t shardIndex, size_t totalShard) {
  return (totalCount * (shardIndex + 1)) / totalShard -
      (totalCount * shardIndex) / totalShard;
}

} // namespace fbpcf::mpc_std_lib::unified_data_process::data_processor

#include "fbpcf/mpc_std_lib/unified_data_process/data_processor/UdpUtil_impl.h"
