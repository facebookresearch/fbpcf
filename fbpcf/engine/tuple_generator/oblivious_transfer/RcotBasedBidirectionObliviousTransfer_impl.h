/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emmintrin.h>
#include <smmintrin.h>
#include <future>
#include <thread>
#include <vector>

namespace fbpcf::engine::tuple_generator::oblivious_transfer {

template <class T>
RcotBasedBidirectionObliviousTransfer<T>::RcotBasedBidirectionObliviousTransfer(
    std::unique_ptr<communication::IPartyCommunicationAgent> agent,
    __m128i delta,
    std::unique_ptr<IRandomCorrelatedObliviousTransfer> senderRcot,
    std::unique_ptr<IRandomCorrelatedObliviousTransfer> receiverRcot)
    : hashFromAes_(util::Aes::getFixedKey()),
      agent_(std::move(agent)),
      delta_(delta),
      senderRcot_(std::move(senderRcot)),
      receiverRcot_(std::move(receiverRcot)) {}

/**
 * From rcot to ot:
 * assume the sender gets random x0, x1 from rcot and receiver gets b and xb
 * Then the the two parties are secret sharing (x1 - x0)*b:
 * sender's share is -x0 and receiver's share is xb
 * To convert this into a OT with chosen inputs and choice
 */
template <class T>
std::vector<T> RcotBasedBidirectionObliviousTransfer<T>::biDirectionOT(
    const std::vector<T>& input0,
    const std::vector<T>& input1,
    const std::vector<bool>& choice) {
  auto otSize = input0.size();

  auto future =
      std::async([otSize, this]() { return receiverRcot_->rcot(otSize); });

  // u_0
  auto rcotSender0Messages = senderRcot_->rcot(otSize);

  // u_c
  auto rcotReceiverMessages = future.get();

  assert(rcotSender0Messages.size() == otSize);
  assert(rcotReceiverMessages.size() == otSize);

  std::vector<__m128i> rcotSender1Messages(otSize);
  for (size_t i = 0; i < otSize; i++) {
    // u_1
    rcotSender1Messages[i] = _mm_xor_si128(rcotSender0Messages[i], delta_);
  }

  std::vector<bool> maskedChoice(otSize);
  for (size_t i = 0; i < otSize; i++) {
    // c ^ b
    maskedChoice[i] = util::getLsb(rcotReceiverMessages[i]) ^ choice[i];
  }

  agent_->sendBool(maskedChoice);
  // c ^ b
  auto flipIndicator = agent_->receiveBool(otSize);

  assert(flipIndicator.size() == otSize);

  hashFromAes_.inPlaceHash(rcotSender0Messages);
  hashFromAes_.inPlaceHash(rcotSender1Messages);
  hashFromAes_.inPlaceHash(rcotReceiverMessages);

  std::vector<T> maskedInput0(otSize);
  std::vector<T> maskedInput1(otSize);

  for (size_t i = 0; i < otSize; i++) {
    maskedInput0[i] = util::Masker<T>::mask(
        input0[i],
        flipIndicator[i] ? rcotSender1Messages[i] : rcotSender0Messages[i]);

    maskedInput1[i] = util::Masker<T>::mask(
        input1[i],
        flipIndicator[i] ? rcotSender0Messages[i] : rcotSender1Messages[i]);
  }

  agent_->sendT<T>(maskedInput0);
  agent_->sendT<T>(maskedInput1);
  auto correction0 = agent_->receiveT<T>(maskedInput0.size());
  auto correction1 = agent_->receiveT<T>(maskedInput1.size());

  assert(correction0.size() == otSize);
  assert(correction1.size() == otSize);

  std::vector<T> output(otSize);

  for (size_t i = 0; i < otSize; i++) {
    output[i] = util::Masker<T>::unmask(
        rcotReceiverMessages[i], choice[i], correction0[i], correction1[i]);
  }

  return output;
}

template <class T>
std::pair<uint64_t, uint64_t>
RcotBasedBidirectionObliviousTransfer<T>::getTrafficStatistics() const {
  auto rst = agent_->getTrafficStatistics();
  auto senderCost = senderRcot_->getTrafficStatistics();
  auto receiverCost = receiverRcot_->getTrafficStatistics();
  rst.first += senderCost.first + receiverCost.first;
  rst.second += senderCost.second + receiverCost.second;
  return rst;
}

} // namespace fbpcf::engine::tuple_generator::oblivious_transfer
