/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emmintrin.h>
#include "fbpcf/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/engine/util/util.h"
#include "fbpcf/mpc_std_lib/unified_data_process/data_processor/UdpUtil.h"

namespace fbpcf::mpc_std_lib::unified_data_process::data_processor {

template <int schedulerId>
class DataProcessor;

/**
 * This class handles the encryption step of UDP at scale. The raw data can
 *be passed into this object in batches. This object is not thread-safe but
 *it will spin up multiple threads internally.
 **/
class UdpEncryption {
  template <int schedulerId>
  friend class DataProcessor;

 public:
  explicit UdpEncryption(
      std::unique_ptr<fbpcf::engine::communication::IPartyCommunicationAgent>
          agent);

  void prepareToProcessMyData(size_t myDataWidth);

  // process my data via UDP encryption. This API should be called in coordinate
  // with "ProcessPeerData" on peer's side. If this API is ever called, calling
  // "getExpandedKey" to retrive the expanded key for decryption later.
  void processMyData(
      const std::vector<std::vector<unsigned char>>& plaintextData);

  std::vector<__m128i> getExpandedKey() {
    if (statusOfProcessingMyData_ != Status::inProgress) {
      throw std::runtime_error(
          "Can't call get ExapndedKey before preparation!");
    }
    statusOfProcessingMyData_ = Status::idle;

    auto expandedKey = engine::util::Aes::expandEncryptionKey(prgKey_);
    return std::vector<__m128i>(expandedKey.begin(), expandedKey.end());
  }

  void prepareToProcessPeerData(
      size_t peerDataWidth,
      const std::vector<int32_t>& indexes);

  // process peer data via UDP encryption. This API should be called in
  // coordinate with "ProcessMyData" on peer's side. This API is ever called,
  // calling "getProcessedData" to retrive the cherry-picked encryption later.
  void processPeerData(size_t dataSize);

  // returning the ciphertext, nonce, and index of cherry-picked rows
  std::tuple<
      std::vector<std::vector<unsigned char>>,
      std::vector<__m128i>,
      std::vector<int32_t>>
  getProcessedData() {
    if (statusOfProcessingPeerData_ != Status::inProgress) {
      throw std::runtime_error(
          "Can't call getProcessedData before preparation!");
    }
    statusOfProcessingPeerData_ = Status::idle;
    return {
        std::move(cherryPickedEncryption_),
        std::move(cherryPickedNonce_),
        std::move(cherryPickedIndex_)};
  }

 private:
  std::unique_ptr<fbpcf::engine::communication::IPartyCommunicationAgent>
      agent_;

  uint64_t myDataIndexOffset_;
  uint64_t peerDataIndexOffset_;

  size_t myDataWidth_;
  __m128i prgKey_;

  size_t peerDataWidth_;
  // this map records the index-to-order map, the i-th row in peer's input
  // should be indexToOrderMap_.at(i)-th in the encryption result, if i is in
  // this map.
  std::map<int32_t, int32_t> indexToOrderMap_;

  // the vector of cherry-picked data of matched user, the three vectors
  // consists of ciphertext, nonce, and index, respectively. We need to save the
  // nonce because different nonces would be used in different "batches".

  std::vector<std::vector<unsigned char>> cherryPickedEncryption_;
  std::vector<__m128i> cherryPickedNonce_;
  std::vector<int32_t> cherryPickedIndex_;

  static const size_t kBlockSize = 16;

  // record the status of this object of processing data
  enum Status {
    idle, // need to call setup first before any other APIs
    inProgress, // allowed to call "processXXData" and getters. Calling getters
                // will reset this object to idle status.
  };
  enum Status statusOfProcessingMyData_;
  enum Status statusOfProcessingPeerData_;
};

} // namespace fbpcf::mpc_std_lib::unified_data_process::data_processor
