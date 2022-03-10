/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/engine/tuple_generator/TwoPartyTupleGenerator.h"
#include "fbpcf/engine/util/util.h"

namespace fbpcf::engine::tuple_generator {

TwoPartyTupleGenerator::TwoPartyTupleGenerator(
    std::unique_ptr<oblivious_transfer::IRandomCorrelatedObliviousTransfer>
        senderRcot,
    std::unique_ptr<oblivious_transfer::IRandomCorrelatedObliviousTransfer>
        receiverRcot,
    __m128i delta,
    uint64_t bufferSize)
    : // the key itself is not important as long as it's a pre-agreed value
      hashFromAes_(util::Aes::getFixedKey()),
      senderRcot_{std::move(senderRcot)},
      receiverRcot_{std::move(receiverRcot)},
      delta_{delta},
      buffer_{bufferSize, [this](uint64_t size) {
                return generateTuples(size);
              }} {}

std::vector<ITupleGenerator::BooleanTuple>
TwoPartyTupleGenerator::getBooleanTuple(uint32_t size) {
  return buffer_.getData(size);
}

/**
 * Two party tuple generation algorithm:
 *
 * Party 1 sends k_0 and k_1 = k_0 + delta_1 to party 2
 * Party 2 sends l_0 and l_1 = l_0 + delta_2 to party 1
 *
 * Party 1 chooses r and receives l_r from party 2
 * Party 2 chooses p and receives k_p from party 1
 *
 * Party 1 computes:
 *   a_1 = h(k_0) ^ h(k_1)
 *   b_1 = r
 *   c_1 = (a_1 & b_1) ^ h(k_0) ^ h(l_r) = h(k_r) ^ h(l_r)
 *
 * Party 2 computes:
 *   a_2 = h(l_0) ^ h(l_1)
 *   b_2 = p
 *   c_2 = (a_2 & b_2) ^ h(l_0) ^ h(k_p) = h(k_p) ^ h(l_p)
 *
 * Note that:
 * (a_1 ^ a_2) & (b_1 ^ b_2)
 * = (h(k_0) ^ h(k_1) ^ h(l_0) ^ h(l_1)) & (r ^ p)
 * = h(k_0) ^ h(k_r) ^ h(k_0) ^ h(k_p) ^ h(l_0) ^ h(l_r) ^ h(l_0) ^ h(l_p)
 * = h(k_r) ^ h(k_p) ^ h(l_r) ^ h(l_p)
 * = c_1 ^ c_2
 */
std::vector<ITupleGenerator::BooleanTuple>
TwoPartyTupleGenerator::generateTuples(uint64_t size) {
  auto receiverMessagesFuture =
      std::async([size, this]() { return receiverRcot_->rcot(size); });

  auto sender0Messages = senderRcot_->rcot(size);

  std::vector<__m128i> sender1Messages(size);
  for (auto i = 0; i < size; ++i) {
    sender1Messages[i] = _mm_xor_si128(sender0Messages.at(i), delta_);
  }

  auto receiverMessages = receiverMessagesFuture.get();

  std::vector<bool> choiceBits(size);
  for (auto i = 0; i < size; ++i) {
    choiceBits[i] = util::getLsb(receiverMessages.at(i));
  }

  hashFromAes_.inPlaceHash(sender0Messages);
  hashFromAes_.inPlaceHash(sender1Messages);
  hashFromAes_.inPlaceHash(receiverMessages);

  std::vector<ITupleGenerator::BooleanTuple> booleanTuples(size);
  for (size_t i = 0; i < size; i++) {
    auto a = util::getLsb(sender0Messages.at(i)) ^
        util::getLsb(sender1Messages.at(i));
    auto b = choiceBits.at(i);
    auto c = (a & b) ^ util::getLsb(sender0Messages.at(i)) ^
        util::getLsb(receiverMessages.at(i));

    booleanTuples[i] = BooleanTuple(a, b, c);
  }
  return booleanTuples;
}

std::pair<uint64_t, uint64_t> TwoPartyTupleGenerator::getTrafficStatistics()
    const {
  std::pair<uint64_t, uint64_t> rst = {0, 0};

  auto senderStats = senderRcot_->getTrafficStatistics();
  auto receiverStats = receiverRcot_->getTrafficStatistics();
  rst.first += senderStats.first + receiverStats.first;
  rst.second += senderStats.first + receiverStats.second;

  return rst;
}

} // namespace fbpcf::engine::tuple_generator
