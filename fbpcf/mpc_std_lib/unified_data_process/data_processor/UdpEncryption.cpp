/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/mpc_std_lib/unified_data_process/data_processor/UdpEncryption.h"
#include <stdexcept>
#include <string>
#include "fbpcf/primitive/mac/S2v.h"
#include "fbpcf/primitive/mac/S2vFactory.h"

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
  indexOffset_ = 0;
}

void UdpEncryption::processMyData(
    const std::vector<std::vector<unsigned char>>& plaintextData) {
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
      UdpUtil::localEncryption(plaintextData, prgKey_, indexOffset_);
  agent_->send(nonce);
  for (size_t i = 0; i < ciphertext.size(); i++) {
    agent_->send(ciphertext.at(i));
  }
  indexOffset_ += plaintextData.size();
}

void UdpEncryption::prepareToProcessPeerData(
    size_t peerDataWidth,
    const std::vector<int32_t>& indexes) {
  if (statusOfProcessingPeerData_ != Status::idle) {
    throw std::runtime_error(
        "Can't call prepare when already processing peer data!");
  }
  statusOfProcessingPeerData_ = Status::inProgress;

  for (size_t i = 0; i < indexes.size(); i++) {
    indexToOrderMap_.emplace(indexes.at(i), i);
  }

  peerDataWidth_ = peerDataWidth;
  indexOffset_ = 0;

  cherryPickedEncryption_ =
      std::vector<std::vector<unsigned char>>(indexes.size());
  cherryPickedNonce_ = std::vector<__m128i>(indexes.size());
  cherryPickedIndex_ = std::vector<int32_t>(indexes.size());
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
    auto ciphertext = agent_->receive(peerDataWidth_);
    auto pos = indexToOrderMap_.find(i + indexOffset_);
    if (pos != indexToOrderMap_.end()) {
      // this ciphertext should be picked up
      cherryPickedEncryption_.at(pos->second) = std::move(ciphertext);
      cherryPickedNonce_.at(pos->second) = nonce;
      cherryPickedIndex_.at(pos->second) = i + indexOffset_;
      indexToOrderMap_.erase(pos);
      // TODO: this can be further optimized by not copying duplicated nonce.
    }
  }
  indexOffset_ += dataSize;
}

} // namespace fbpcf::mpc_std_lib::unified_data_process::data_processor
