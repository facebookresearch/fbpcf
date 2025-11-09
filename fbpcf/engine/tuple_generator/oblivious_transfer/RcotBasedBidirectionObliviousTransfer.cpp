/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/engine/tuple_generator/oblivious_transfer/RcotBasedBidirectionObliviousTransfer.h"
#include <emmintrin.h>
#include <future>
#include <thread>
#include <vector>

namespace fbpcf::engine::tuple_generator::oblivious_transfer {

RcotBasedBidirectionObliviousTransfer::RcotBasedBidirectionObliviousTransfer(
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
 * assume the sender gets random k0, k1 from rcot and receiver gets p and kp
 * Then the the two parties are secret sharing (k1 - k0)*p = delta * p:
 * sender's share is -k0 and receiver's share is kp
 * To convert this into a OT with chosen inputs and choice bit, each party will
 * send their choice masked by the rcot bit to the other party. The other party
 * will then compute two corrections based on this masked bit + the messages
 * they want to send + rcot messages. The parties will exchange these correction
 * messages and retrieve the chosen message using their choice bit and rcot
 * message.
 */
std::vector<bool> RcotBasedBidirectionObliviousTransfer::biDirectionOT(
    const std::vector<bool>& input0,
    const std::vector<bool>& input1,
    const std::vector<bool>& choice) {
  auto otSize = input0.size();

  auto future =
      std::async([otSize, this]() { return receiverRcot_->rcot(otSize); });

  // k0 / l0
  auto rcotSender0Messages = senderRcot_->rcot(otSize);

  // lr / kp
  auto rcotReceiverMessages = future.get();

  assert(rcotSender0Messages.size() == otSize);
  assert(rcotReceiverMessages.size() == otSize);

  std::vector<__m128i> rcotSender1Messages(otSize);
  for (size_t i = 0; i < otSize; i++) {
    // k1 / l1
    rcotSender1Messages[i] = _mm_xor_si128(rcotSender0Messages[i], delta_);
  }

  std::vector<bool> maskedChoice(otSize);
  for (size_t i = 0; i < otSize; i++) {
    // r + choice1 / p + choice2
    maskedChoice[i] = util::getLsb(rcotReceiverMessages[i]) ^ choice[i];
  }

  agent_->sendBool(maskedChoice);
  // p + choice2 / r + choice1
  auto flipIndicator = agent_->receiveBool(otSize);

  assert(flipIndicator.size() == otSize);
  // h(k0) / h(l0)
  hashFromAes_.inPlaceHash(rcotSender0Messages);
  // h(k1) / h(l1)
  hashFromAes_.inPlaceHash(rcotSender1Messages);
  // h(lr) / h(kp)
  hashFromAes_.inPlaceHash(rcotReceiverMessages);

  std::vector<bool> maskedInput0(otSize);
  std::vector<bool> maskedInput1(otSize);

  for (size_t i = 0; i < otSize; i++) {
    // h(key, n) = AES_HASH(0, key) & ((1 << n) - 1) if n <= 128
    // h(key, n) = AES_PRG(key, n) if n > 128
    // mask(x, key) = x + h(key, |x|)

    // mask(x0, p + choice2 ? k1 : k0) / mask(y0, r + choice1 ? l1 ? l0)
    // x0 + h((p + choice2) * delta1 + k0, |x0|) /
    // y0 + h((r + choice1) * delta2 + l0, |y0|)
    maskedInput0[i] = util::Masker<bool>::mask(
        input0[i],
        flipIndicator[i] ? rcotSender1Messages[i] : rcotSender0Messages[i]);

    // mask(x1, p + choice2 ? k0 : k1) / mask(y1, r + choice1 ? l0 ? l1)
    // x1 + h((p + choice2) * delta1 + k1, |x1|) /
    // y1 + h((r + choice1) * delta2 + l1, |y1|)
    maskedInput1[i] = util::Masker<bool>::mask(
        input1[i],
        flipIndicator[i] ? rcotSender0Messages[i] : rcotSender1Messages[i]);
  }

  agent_->sendT<bool>(maskedInput0);
  agent_->sendT<bool>(maskedInput1);
  // c0 = y0 + h((r + choice1) * delta2 + l0, |y0|) /
  // c0 = x0 + h((p + choice2) * delta1 + k0, |x0|)
  auto correction0 = agent_->receiveT<bool>(maskedInput0.size());
  // c1 = y1 + h((r + choice1) * delta2 + l1, |y1|) /
  // c1 = x1 + h((p + choice2) * delta1 + k1, |x1|)
  auto correction1 = agent_->receiveT<bool>(maskedInput1.size());

  assert(correction0.size() == otSize);
  assert(correction1.size() == otSize);

  std::vector<bool> output(otSize);

  for (size_t i = 0; i < otSize; i++) {
    // unmask(key, b, c0, c1) = h(key, |c0|) + (b ? c1 : c0)

    // unmask(lr, choice1, c0, c1) =
    // h(lr, |c0|) +
    // choice1 ?
    //  y1 + h((r + choice1) * delta2 + l1, |y1|) :
    //  y0 + h((r + choice1) * delta2 + l0, |y0|)
    //
    // = h(lr, |c0|) + choice1 ?
    //  y1 + h((r + 1) * delta2 + l1, |y1|)
    //= y1 + h(r * delta2 + delta2 + l1, |y1|)
    //= y1 + h(r * delta2 + l0, |x1|)
    //= y1 + h(lr, |x1|) :
    //  y0 + h((r + 0) * delta2 + l0, |x0|)
    //= y0 + h(lr, |x0|)
    // = choice1 ? y1 : y0 /
    //
    // = choice2 ? x1 : x0
    output[i] = util::Masker<bool>::unmask(
        rcotReceiverMessages[i], choice[i], correction0[i], correction1[i]);
  }

  return output;
}

std::vector<uint64_t> RcotBasedBidirectionObliviousTransfer::biDirectionOT(
    const std::vector<uint64_t>& input0,
    const std::vector<uint64_t>& input1,
    const std::vector<bool>& choice) {
  auto otSize = input0.size();

  auto future =
      std::async([otSize, this]() { return receiverRcot_->rcot(otSize); });

  // k0 / l0
  auto rcotSender0Messages = senderRcot_->rcot(otSize);

  // lr / kp
  auto rcotReceiverMessages = future.get();

  assert(rcotSender0Messages.size() == otSize);
  assert(rcotReceiverMessages.size() == otSize);

  std::vector<__m128i> rcotSender1Messages(otSize);
  for (size_t i = 0; i < otSize; i++) {
    // k1 / l1
    rcotSender1Messages[i] = _mm_xor_si128(rcotSender0Messages[i], delta_);
  }

  std::vector<bool> maskedChoice(otSize);
  for (size_t i = 0; i < otSize; i++) {
    // r + choice1 / p + choice2
    maskedChoice[i] = util::getLsb(rcotReceiverMessages[i]) ^ choice[i];
  }

  agent_->sendBool(maskedChoice);
  // p + choice2 / r + choice1
  auto flipIndicator = agent_->receiveBool(otSize);

  assert(flipIndicator.size() == otSize);
  // h(k0) / h(l0)
  hashFromAes_.inPlaceHash(rcotSender0Messages);
  // h(k1) / h(l1)
  hashFromAes_.inPlaceHash(rcotSender1Messages);
  // h(lr) / h(kp)
  hashFromAes_.inPlaceHash(rcotReceiverMessages);

  std::vector<uint64_t> maskedInput0(otSize);
  std::vector<uint64_t> maskedInput1(otSize);

  for (size_t i = 0; i < otSize; i++) {
    // h(key, n) = AES_HASH(0, key) & ((1 << n) - 1) if n <= 128
    // h(key, n) = AES_PRG(key, n) if n > 128
    // mask(x, key) = x + h(key, |x|)

    // mask(x0, p + choice2 ? k1 : k0) / mask(y0, r + choice1 ? l1 ? l0)
    // x0 + h((p + choice2) * delta1 + k0, |x0|) /
    // y0 + h((r + choice1) * delta2 + l0, |y0|)
    maskedInput0[i] = util::Masker<uint64_t>::mask(
        input0[i],
        flipIndicator[i] ? rcotSender1Messages[i] : rcotSender0Messages[i]);

    // mask(x1, p + choice2 ? k0 : k1) / mask(y1, r + choice1 ? l0 ? l1)
    // x1 + h((p + choice2) * delta1 + k1, |x1|) /
    // y1 + h((r + choice1) * delta2 + l1, |y1|)
    maskedInput1[i] = util::Masker<uint64_t>::mask(
        input1[i],
        flipIndicator[i] ? rcotSender0Messages[i] : rcotSender1Messages[i]);
  }

  agent_->sendT<uint64_t>(maskedInput0);
  agent_->sendT<uint64_t>(maskedInput1);
  // c0 = y0 + h((r + choice1) * delta2 + l0, |y0|) /
  // c0 = x0 + h((p + choice2) * delta1 + k0, |x0|)
  auto correction0 = agent_->receiveT<uint64_t>(maskedInput0.size());
  // c1 = y1 + h((r + choice1) * delta2 + l1, |y1|) /
  // c1 = x1 + h((p + choice2) * delta1 + k1, |x1|)
  auto correction1 = agent_->receiveT<uint64_t>(maskedInput1.size());

  assert(correction0.size() == otSize);
  assert(correction1.size() == otSize);

  std::vector<uint64_t> output(otSize);

  for (size_t i = 0; i < otSize; i++) {
    // unmask(key, b, c0, c1) = h(key, |c0|) + (b ? c1 : c0)

    // unmask(lr, choice1, c0, c1) =
    // h(lr, |c0|) +
    // choice1 ?
    //  y1 + h((r + choice1) * delta2 + l1, |y1|) :
    //  y0 + h((r + choice1) * delta2 + l0, |y0|)
    //
    // = h(lr, |c0|) + choice1 ?
    //  y1 + h((r + 1) * delta2 + l1, |y1|)
    //= y1 + h(r * delta2 + delta2 + l1, |y1|)
    //= y1 + h(r * delta2 + l0, |x1|)
    //= y1 + h(lr, |x1|) :
    //  y0 + h((r + 0) * delta2 + l0, |x0|)
    //= y0 + h(lr, |x0|)
    // = choice1 ? y1 : y0 /
    //
    // = choice2 ? x1 : x0
    output[i] = util::Masker<uint64_t>::unmask(
        rcotReceiverMessages[i], choice[i], correction0[i], correction1[i]);
  }

  return output;
}

std::pair<uint64_t, uint64_t>
RcotBasedBidirectionObliviousTransfer::getTrafficStatistics() const {
  auto rst = agent_->getTrafficStatistics();
  auto senderCost = senderRcot_->getTrafficStatistics();
  auto receiverCost = receiverRcot_->getTrafficStatistics();
  rst.first += senderCost.first + receiverCost.first;
  rst.second += senderCost.second + receiverCost.second;
  return rst;
}

} // namespace fbpcf::engine::tuple_generator::oblivious_transfer
