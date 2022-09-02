/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <emmintrin.h>
#include <vector>
#include "fbpcf/engine/util/util.h"

#include <fbpcf/mpc_std_lib/walr_multiplication/util/COTWithRandomMessage.h>

namespace fbpcf::mpc_std_lib::walr::util {

std::pair<std::vector<__m128i>, std::vector<__m128i>>
COTWithRandomMessage::send(size_t size) {
  if (role_ != engine::util::Role::sender) {
    throw std::runtime_error("Only sender of the OT can invoke this method.");
  }
  auto rcotSender0Messages = rcot_->rcot(size); // k0
  assert(rcotSender0Messages.size() == size);
  std::vector<__m128i> rcotSender1Messages(rcotSender0Messages);
  auto flipIndicator = agent_->receiveBool(size);
  assert(flipIndicator.size() == size);
  for (size_t i = 0; i < size; i++) {
    // k1
    if (flipIndicator[i]) {
      rcotSender0Messages[i] = _mm_xor_si128(rcotSender0Messages[i], delta_);
    } else {
      rcotSender1Messages[i] = _mm_xor_si128(rcotSender0Messages[i], delta_);
    }
  }

  return {rcotSender0Messages, rcotSender1Messages};
}

std::vector<__m128i> COTWithRandomMessage::receive(
    const std::vector<bool>& choice) {
  auto size = choice.size();
  auto rcotReceiverMessages = rcot_->rcot(size);
  assert(rcotReceiverMessages.size() == size);
  std::vector<bool> flipIndicator(size);
  for (size_t i = 0; i < size; ++i) {
    flipIndicator[i] =
        engine::util::getLsb(rcotReceiverMessages[i]) ^ choice[i];
  }
  agent_->sendBool(flipIndicator);
  return rcotReceiverMessages;
}

} // namespace fbpcf::mpc_std_lib::walr::util
