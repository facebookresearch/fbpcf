/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

namespace fbpcf::mpc_std_lib::unified_data_process::data_processor {

template <int schedulerId>
std::vector<UdpUtil::SecBit<schedulerId>> UdpUtil::privatelyShareByteStream(
    const std::vector<std::vector<unsigned char>>& localData,
    int inputPartyID) {
  size_t unitSize = sizeof(unsigned char) * 8;
  size_t localDataWidth = localData.at(0).size() * unitSize;
  size_t stringWidth = localDataWidth % 128 == 0
      ? localDataWidth
      : localDataWidth + 128 - localDataWidth % 128;
  size_t inputSize = localData.size();
  std::vector<SecBit<schedulerId>> sharedData(stringWidth);
  for (size_t i = 0; i < localDataWidth; i++) {
    std::vector<bool> sharedBit(inputSize);
    for (size_t j = 0; j < inputSize; j++) {
      sharedBit.at(j) =
          ((localData.at(j).at(i / unitSize) >> (unitSize - 1 - i % unitSize)) &
           1);
    }
    sharedData.at(i) = SecBit<schedulerId>(sharedBit, inputPartyID);
  }
  // padding
  for (size_t i = localDataWidth; i < stringWidth; ++i) {
    std::vector<bool> sharedBit(inputSize);
    sharedData.at(i) = SecBit<schedulerId>(sharedBit, inputPartyID);
  }
  return sharedData;
}

template <int schedulerId>
std::vector<UdpUtil::SecBit<schedulerId>> UdpUtil::privatelyShareM128iStream(
    const std::vector<std::vector<__m128i>>& localDataM128i,
    int inputPartyID) {
  size_t unitSize = 128;
  size_t batchSize = localDataM128i.size();
  size_t rowSize = localDataM128i.at(0).size();
  std::vector<std::vector<bool>> localDataBool(
      batchSize * rowSize, std::vector<bool>(unitSize));
  for (size_t i = 0; i < batchSize; ++i) {
    for (size_t j = 0; j < rowSize; ++j) {
      // The bits extracted from extractLnbToVector() is the following order:
      // All bytes are in a order that from most significant byte to least
      // significant bytes. The bits in each byte is in a order that from lsb
      // to msb.
      fbpcf::engine::util::extractLnbToVector(
          localDataM128i.at(i).at(j), localDataBool.at(i * rowSize + j));
    }
  }
  std::vector<SecBit<schedulerId>> sharedDataBit(rowSize * 128);
  for (size_t i = 0; i < rowSize * 128; ++i) {
    std::vector<bool> sharedBit(batchSize);
    for (size_t j = 0; j < batchSize; ++j) {
      sharedBit.at(j) = localDataBool.at(j * rowSize + i / 128)
                            .at(i % 128 / 8 * 8 + (7 - i % 8));
    }
    sharedDataBit.at(i) = SecBit<schedulerId>(sharedBit, inputPartyID);
  }
  return sharedDataBit;
}

template <int schedulerId>
std::vector<UdpUtil::SecBit<schedulerId>> UdpUtil::privatelyShareExpandedKey(
    const std::vector<__m128i>& localKeyM128i,
    size_t batchSize,
    int inputPartyID) {
  size_t unitSize = 128;
  size_t blockNo = localKeyM128i.size(); // should be 11
  std::vector<std::vector<bool>> localDataBool(
      blockNo, std::vector<bool>(unitSize));
  for (size_t i = 0; i < blockNo; ++i) {
    // The bits extracted from extractLnbToVector() is the following order:
    // All bytes are in a order that from most significant byte to least
    // significant bytes. The bits in each byte is in a order that from lsb to
    // msb.
    fbpcf::engine::util::extractLnbToVector(
        localKeyM128i.at(i), localDataBool.at(i));
  }
  std::vector<SecBit<schedulerId>> sharedKeyBit(blockNo * unitSize);
  for (size_t i = 0; i < blockNo * unitSize; ++i) {
    std::vector<bool> sharedBit(
        batchSize,
        localDataBool.at(i / unitSize).at(i % unitSize / 8 * 8 + (7 - i % 8)));
    sharedKeyBit.at(i) = SecBit<schedulerId>(sharedBit, inputPartyID);
  }
  return sharedKeyBit;
}

} // namespace fbpcf::mpc_std_lib::unified_data_process::data_processor
