/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emmintrin.h>
#include <stdexcept>
#include "fbpcf/engine/util/util.h"
#include "fbpcf/mpc_std_lib/aes_circuit/AesCircuitCtrFactory.h"
#include "fbpcf/mpc_std_lib/aes_circuit/IAesCircuitCtr.h"
#include "fbpcf/mpc_std_lib/unified_data_process/data_processor/UdpUtil.h"

namespace fbpcf::mpc_std_lib::unified_data_process::data_processor {

/**
 * This is the implementation of the decryption phase of the UDP data processor.
 */
template <int schedulerId>
class UdpDecryption {
 public:
  using SecBit = frontend::Bit<true, schedulerId, true>;
  using SecString = frontend::BitString<true, schedulerId, true>;
  using AesCtr =
      aes_circuit::IAesCircuitCtr<frontend::Bit<true, schedulerId, true>>;

  explicit UdpDecryption(int32_t myId, int32_t partnerId)
      : myId_(myId), partnerId_(partnerId) {
    aesCircuitCtr_ =
        std::make_unique<aes_circuit::AesCircuitCtrFactory<SecBit>>()->create();
  }

  SecString decryptMyData(
      const std::vector<__m128i>& expandedKey,
      size_t outputWidth,
      size_t outputSize) const {
    auto keyString = UdpUtil::privatelyShareExpandedKey<schedulerId>(
        expandedKey, outputSize, myId_);

    auto filteredCiphertext = UdpUtil::privatelyShareByteStream<schedulerId>(
        std::vector<std::vector<unsigned char>>(
            outputSize, std::vector<unsigned char>(outputWidth)),
        partnerId_);

    auto filteredCounters = UdpUtil::privatelyShareM128iStream<schedulerId>(
        std::vector<std::vector<__m128i>>(
            outputSize, std::vector<__m128i>(filteredCiphertext.size() / 128)),
        partnerId_);

    return generatingOutput(
        aesCircuitCtr_->decrypt(
            filteredCiphertext, keyString, filteredCounters),
        outputWidth * 8);
  }

  SecString decryptPeerData(
      const std::vector<std::vector<unsigned char>>& cherryPickedEncryption,
      const std::vector<__m128i>& cherryPickedNonce,
      const std::vector<int32_t>& cherryPickedIndex) const {
    size_t outputWidth = cherryPickedEncryption.at(0).size();
    size_t outputSize = cherryPickedEncryption.size();

    auto keyString = UdpUtil::privatelyShareExpandedKey<schedulerId>(
        std::vector<__m128i>(11), outputSize, partnerId_);

    auto filteredCiphertext = UdpUtil::privatelyShareByteStream<schedulerId>(
        cherryPickedEncryption, myId_);

    size_t cipherBlocks = (outputWidth + 15) / 16;

    std::vector<std::vector<__m128i>> filteredCountersM128i(
        outputSize, std::vector<__m128i>(cipherBlocks));
    for (uint64_t i = 0; i < outputSize; ++i) {
      for (uint64_t j = 0; j < cipherBlocks; ++j) {
        filteredCountersM128i.at(i).at(j) =
            _mm_set_epi64x(0, cherryPickedIndex.at(i) * cipherBlocks + j);
        filteredCountersM128i.at(i).at(j) = _mm_add_epi64(
            cherryPickedNonce.at(i), filteredCountersM128i.at(i).at(j));
      }
    }
    auto filteredCounters = UdpUtil::privatelyShareM128iStream<schedulerId>(
        filteredCountersM128i, myId_);

    return generatingOutput(
        aesCircuitCtr_->decrypt(
            filteredCiphertext, keyString, filteredCounters),
        outputWidth * 8);
  }

 private:
  SecString generatingOutput(
      const std::vector<SecBit>& decryptionResult,
      size_t width) const {
    // reverse each byte from little endian into big endian order
    SecString outputShare(width);
    for (size_t i = 0; i < width; ++i) {
      outputShare[8 * (i / 8) + (7 - i % 8)] = decryptionResult.at(i);
    }
    return outputShare;
  }

  int32_t myId_;
  int32_t partnerId_;
  std::unique_ptr<AesCtr> aesCircuitCtr_;
};

} // namespace fbpcf::mpc_std_lib::unified_data_process::data_processor
