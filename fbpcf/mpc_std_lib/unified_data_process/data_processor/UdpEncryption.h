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
#include "fbpcf/mpc_std_lib/unified_data_process/data_processor/IUdpEncryption.h"
#include "fbpcf/mpc_std_lib/unified_data_process/data_processor/UdpUtil.h"

namespace fbpcf::mpc_std_lib::unified_data_process::data_processor {

template <int schedulerId>
class DataProcessor;

/**
 * This class handles the encryption step of UDP at scale. The raw data can
 *be passed into this object in batches. This object is not thread-safe but
 *it will spin up multiple threads internally.
 **/
class UdpEncryption final : public IUdpEncryption {
  template <int schedulerId>
  friend class DataProcessor;

 public:
  explicit UdpEncryption(
      std::unique_ptr<fbpcf::engine::communication::IPartyCommunicationAgent>
          agent);

  void prepareToProcessMyData(size_t myDataWidth) override;

  // process my data via UDP encryption. This API should be called in coordinate
  // with "ProcessPeerData" on peer's side. If this API is ever called, calling
  // "getExpandedKey" to retrive the expanded key for decryption later.
  void processMyData(
      const std::vector<std::vector<unsigned char>>& plaintextData,
      const std::vector<uint64_t>& indexes) override;

  std::vector<__m128i> getExpandedKey() override {
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
      const std::vector<uint64_t>& indexes) override;

  // process peer data via UDP encryption. This API should be called in
  // coordinate with "ProcessMyData" on peer's side. This API is ever called,
  // calling "getProcessedData" to retrive the cherry-picked encryption later.
  void processPeerData(size_t dataSize) override;

  // returning the ciphertext, nonce, and index of cherry-picked rows
  EncryptionResults getProcessedData() override {
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
  std::unique_ptr<engine::communication::IPartyCommunicationAgent> agent_;

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
  std::vector<uint64_t> cherryPickedIndex_;

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
