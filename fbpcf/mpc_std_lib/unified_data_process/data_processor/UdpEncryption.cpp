/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/mpc_std_lib/unified_data_process/data_processor/UdpEncryption.h"
#include <stdexcept>
#include <string>

namespace fbpcf::mpc_std_lib::unified_data_process::data_processor {

UdpEncryption::UdpEncryption(
    std::unique_ptr<fbpcf::engine::communication::IPartyCommunicationAgent>
        agent)
    : agent_(std::move(agent)),
      statusOfProcessingMyData_(Status::idle),
      statusOfProcessingPeerData_(Status::idle) {}

void UdpEncryption::prepareToProcessMyData(size_t myDataWidth) {
  if (statusOfProcessingMyData_ != Status::idle) {
    throw std::runtime_error("Can't call prepare when processing my data!");
  }
  statusOfProcessingMyData_ = Status::inProgress;
  myDataWidth_ = myDataWidth;
  prgKey_ = fbpcf::engine::util::getRandomM128iFromSystemNoise();
}

void UdpEncryption::processMyData(
    const std::vector<std::vector<unsigned char>>& plaintextData,
    const std::vector<uint64_t>& indexes) {
  if (statusOfProcessingMyData_ != Status::inProgress) {
    throw std::runtime_error("Can't call procesMyData before preparation!");
  }
  if (plaintextData.size() == 0) {
    throw std::invalid_argument("can't use empty inputs");
  }
  if (plaintextData.at(0).size() != myDataWidth_) {
    throw std::invalid_argument(
        "Inconsistent data width, expecting " + std::to_string(myDataWidth_) +
        " but get " + std::to_string(plaintextData.at(0).size()));
  }

  auto [ciphertext, nonce] =
      UdpUtil::localEncryption(plaintextData, prgKey_, indexes);
  agent_->send(nonce);
  for (size_t i = 0; i < ciphertext.size(); i++) {
    agent_->sendSingleT<uint64_t>(indexes.at(i));
    agent_->send(ciphertext.at(i));
  }
}

void UdpEncryption::prepareToProcessPeerData(
    size_t peerDataWidth,
    const std::vector<uint64_t>& indexes) {
  if (statusOfProcessingPeerData_ != Status::idle) {
    throw std::runtime_error(
        "Can't call prepare when already processing peer data!");
  }
  statusOfProcessingPeerData_ = Status::inProgress;

  for (size_t i = 0; i < indexes.size(); i++) {
    indexToOrderMap_.emplace(indexes.at(i), i);
  }

  peerDataWidth_ = peerDataWidth;

  cherryPickedEncryption_ =
      std::vector<std::vector<unsigned char>>(indexes.size());
  cherryPickedNonce_ = std::vector<__m128i>(indexes.size());
  cherryPickedIndex_ = std::vector<uint64_t>(indexes.size());
}

void UdpEncryption::processPeerData(size_t dataSize) {
  if (statusOfProcessingPeerData_ != Status::inProgress) {
    throw std::runtime_error("Can't call procesPeerData before preparation!");
  }
  __m128i nonce;
  {
    auto nonceData = agent_->receive(kBlockSize);
    nonce = _mm_lddqu_si128((__m128i*)nonceData.data());
  }

  for (size_t i = 0; i < dataSize; i++) {
    auto index = agent_->receiveSingleT<uint64_t>();
    auto ciphertext = agent_->receive(peerDataWidth_);
    auto pos = indexToOrderMap_.find(index);
    if (pos != indexToOrderMap_.end()) {
      // this ciphertext should be picked up
      cherryPickedEncryption_.at(pos->second) = std::move(ciphertext);
      cherryPickedNonce_.at(pos->second) = nonce;
      cherryPickedIndex_.at(pos->second) = index;
      indexToOrderMap_.erase(pos);
      // TODO: this can be further optimized by not copying duplicated nonce.
    }
  }
}

} // namespace fbpcf::mpc_std_lib::unified_data_process::data_processor
