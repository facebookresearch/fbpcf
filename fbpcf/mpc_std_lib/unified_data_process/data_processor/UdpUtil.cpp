/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/mpc_std_lib/unified_data_process/data_processor/UdpUtil.h"
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <cstdint>
#include <cstring>
#include <iterator>
#include "fbpcf/io/api/FileIOWrappers.h"

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
    __m128i sivKey = fbpcf::engine::util::getRandomM128iFromSystemNoise();
    const primitive::mac::S2vFactory s2vFactory;
    std::vector<unsigned char> keyByte(kBlockSize);
    _mm_storeu_si128((__m128i*)keyByte.data(), sivKey);
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

static std::vector<__m128i> convertCharVecToM128i(
    const std::vector<unsigned char>& src) {
  if (src.size() % sizeof(__m128i) != 0) {
    throw std::invalid_argument("Unable to convert to std::vector<__m128i>");
  }
  std::vector<__m128i> rst(src.size() / sizeof(__m128i));
  for (size_t i = 0; i < rst.size(); i++) {
    rst.at(i) = _mm_set_epi8(
        src.at(16 * i + 15),
        src.at(16 * i + 14),
        src.at(16 * i + 13),
        src.at(16 * i + 12),
        src.at(16 * i + 11),
        src.at(16 * i + 10),
        src.at(16 * i + 9),
        src.at(16 * i + 8),
        src.at(16 * i + 7),
        src.at(16 * i + 6),
        src.at(16 * i + 5),
        src.at(16 * i + 4),
        src.at(16 * i + 3),
        src.at(16 * i + 2),
        src.at(16 * i + 1),
        src.at(16 * i));
  }
  return rst;
}

static std::vector<unsigned char> convertM128iToCharVec(
    const std::vector<__m128i>& src) {
  std::vector<unsigned char> rst(src.size() * sizeof(__m128i));
  memcpy(rst.data(), src.data(), rst.size());
  return rst;
}

void writeEncryptionResultsToFile(
    const IUdpEncryption::EncryptionResuts& EncryptionResuts,
    const std::string& file) {
  std::ostringstream s;
  boost::archive::text_oarchive oa(s);
  oa << EncryptionResuts.ciphertexts << EncryptionResuts.indexes
     << convertM128iToCharVec(EncryptionResuts.nonces);

  fbpcf::io::FileIOWrappers::writeFile(file, s.str());
}

void writeExpandedKeyToFile(
    const std::vector<__m128i>& expandedKey,
    const std::string& file) {
  std::ostringstream s;
  boost::archive::text_oarchive oa(s);
  oa << convertM128iToCharVec(expandedKey);
  fbpcf::io::FileIOWrappers::writeFile(file, s.str());
}

IUdpEncryption::EncryptionResuts readEncryptionResultsFromFile(
    const std::string& file) {
  std::istringstream s(fbpcf::io::FileIOWrappers::readFile(file));

  IUdpEncryption::EncryptionResuts data;
  boost::archive::text_iarchive ia(s);
  ia >> data.ciphertexts;
  ia >> data.indexes;
  std::vector<unsigned char> tmp;
  ia >> tmp;

  data.nonces = convertCharVecToM128i(tmp);
  return data;
}

std::vector<__m128i> readExpandedKeyFromFile(const std::string& file) {
  std::vector<unsigned char> data(sizeof(__m128i) * 11);
  std::istringstream s(fbpcf::io::FileIOWrappers::readFile(file));
  boost::archive::text_iarchive ia(s);
  ia >> data;
  return convertCharVecToM128i(data);
}

std::vector<IUdpEncryption::EncryptionResuts> splitEncryptionResults(
    const IUdpEncryption::EncryptionResuts& encryptionResults,
    int count) {
  std::vector<IUdpEncryption::EncryptionResuts> rst;
  rst.reserve(count);
  size_t originalSize = encryptionResults.nonces.size();
  auto ciphertextIt = encryptionResults.ciphertexts.begin();
  auto noncesIt = encryptionResults.nonces.begin();
  auto indexesIt = encryptionResults.indexes.begin();

  for (size_t i = 0; i < count; i++) {
    auto shardSize = getShardSize(originalSize, i, count);
    rst.push_back(IUdpEncryption::EncryptionResuts{
        .ciphertexts = std::vector<std::vector<unsigned char>>(
            std::make_move_iterator(ciphertextIt),
            std::make_move_iterator(ciphertextIt + shardSize)),
        .nonces = std::vector<__m128i>(
            std::make_move_iterator(noncesIt),
            std::make_move_iterator(noncesIt + shardSize)),
        .indexes = std::vector<int32_t>(
            std::make_move_iterator(indexesIt),
            std::make_move_iterator(indexesIt + shardSize))});
    ciphertextIt += shardSize;
    noncesIt += shardSize;
    indexesIt += shardSize;
  }
  return rst;
}

} // namespace fbpcf::mpc_std_lib::unified_data_process::data_processor
