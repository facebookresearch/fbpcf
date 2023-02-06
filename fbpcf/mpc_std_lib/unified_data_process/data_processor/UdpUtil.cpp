/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/mpc_std_lib/unified_data_process/data_processor/UdpUtil.h"
#include <cstdint>
#include <cstring>

namespace fbpcf::mpc_std_lib::unified_data_process::data_processor {

std::vector<__m128i> UdpUtil::generateCounterBlocks(
    __m128i nonce,
    uint64_t startingIndex,
    size_t size) {
  std::vector<__m128i> rst(size);
  for (size_t i = 0; i < size; i++) {
    rst.at(i) = _mm_add_epi64(nonce, _mm_set_epi64x(0, i + startingIndex));
  }
  return rst;
}

std::pair<std::vector<std::vector<uint8_t>>, std::vector<uint8_t>>
UdpUtil::localEncryption(
    const std::vector<std::vector<unsigned char>>& plaintextData,
    __m128i prgKey,
    uint64_t indexOffset) {
  size_t rowCounts = plaintextData.size();
  size_t rowSize = plaintextData.at(0).size();
  size_t rowBlocks = (rowSize + kBlockSize - 1) / kBlockSize;

  fbpcf::engine::util::Aes localAes(prgKey);

  // generate counters for each block

  __m128i s2vRes;
  {
    const primitive::mac::S2vFactory s2vFactory;
    std::vector<unsigned char> keyByte(kBlockSize);
    _mm_storeu_si128((__m128i*)keyByte.data(), prgKey);
    const auto s2v = s2vFactory.create(keyByte);

    std::vector<unsigned char> plaintextCombined;
    plaintextCombined.reserve(rowSize * rowCounts);
    std::for_each(
        plaintextData.begin(),
        plaintextData.end(),
        [&plaintextCombined](const auto& v) {
          std::copy(v.begin(), v.end(), std::back_inserter(plaintextCombined));
        });
    s2vRes = s2v->getMacM128i(plaintextCombined);
  }

  std::vector<std::vector<__m128i>> counterM128i(rowCounts);

  for (uint64_t i = 0; i < counterM128i.size(); ++i) {
    counterM128i.at(i) =
        generateCounterBlocks(s2vRes, (indexOffset + i) * rowBlocks, rowBlocks);
    // encrypt counters
    localAes.encryptInPlace(counterM128i.at(i));
  }

  std::vector<std::vector<uint8_t>> ciphertextByte(
      rowCounts, std::vector<uint8_t>(rowSize));

  for (size_t i = 0; i < rowCounts; ++i) {
    std::vector<uint8_t> mask(rowBlocks * kBlockSize);
    memcpy(mask.data(), counterM128i.at(i).data(), rowBlocks * kBlockSize);
    for (size_t j = 0; j < rowSize; ++j) {
      ciphertextByte.at(i).at(j) = mask.at(j) ^ plaintextData.at(i).at(j);
    }
  }

  std::vector<unsigned char> s2vVec(kBlockSize);
  _mm_storeu_si128((__m128i*)s2vVec.data(), s2vRes);
  return {ciphertextByte, s2vVec};
}

} // namespace fbpcf::mpc_std_lib::unified_data_process::data_processor
