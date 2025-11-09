/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/engine/tuple_generator/TwoPartyTupleGenerator.h"
#include "fbpcf/engine/util/AesPrg.h"
#include "fbpcf/engine/util/util.h"

namespace fbpcf::engine::tuple_generator {

TwoPartyTupleGenerator::TwoPartyTupleGenerator(
    std::unique_ptr<oblivious_transfer::IRandomCorrelatedObliviousTransfer>
        senderRcot,
    std::unique_ptr<oblivious_transfer::IRandomCorrelatedObliviousTransfer>
        receiverRcot,
    __m128i delta,
    std::shared_ptr<TuplesMetricRecorder> recorder,
    uint64_t bufferSize)
    : // the key itself is not important as long as it's a pre-agreed value
      hashFromAes_(util::Aes::getFixedKey()),
      senderRcot_{std::move(senderRcot)},
      receiverRcot_{std::move(receiverRcot)},
      delta_{delta},
      recorder_{recorder},
      booleanTupleBuffer_{
          bufferSize,
          [this](uint64_t size) {
            {
              std::lock_guard<std::mutex> lock(scheduleMutex_);
              toGenerate_.push_back(Boolean);
            }
            return std::async(
                [this](uint64_t size) { return generateNormalTuples(size); },
                size);
          }},
      rcotBuffer_{bufferSize, [this](uint64_t size) {
                    {
                      std::lock_guard<std::mutex> lock(scheduleMutex_);
                      toGenerate_.push_back(Composite);
                    }
                    return std::async(
                        [this](uint64_t size) {
                          return generateRcotResults(size);
                        },
                        size);
                  }} {}

std::vector<ITupleGenerator::BooleanTuple>
TwoPartyTupleGenerator::getBooleanTuple(uint32_t size) {
  recorder_->addTuplesConsumed(size);

  return booleanTupleBuffer_.getData(size);
}

std::map<size_t, std::vector<ITupleGenerator::CompositeBooleanTuple>>
TwoPartyTupleGenerator::getCompositeTuple(
    const std::map<size_t, uint32_t>& tupleSizes) {
  std::map<size_t, std::vector<ITupleGenerator::CompositeBooleanTuple>> tuples;
  for (auto& tupleSizeToCount : tupleSizes) {
    size_t tupleSize = std::get<0>(tupleSizeToCount);
    uint64_t count = std::get<1>(tupleSizeToCount);
    auto bufferedRcotResult = rcotBuffer_.getData(count);
    std::vector<__m128i> sender0Messages(count);
    std::vector<__m128i> receiverMessages(count);
    for (size_t i = 0; i < count; i++) {
      sender0Messages[i] = std::get<0>(bufferedRcotResult.at(i));
      receiverMessages[i] = std::get<1>(bufferedRcotResult.at(i));
    }
    tuples.emplace(
        tupleSize,
        expandRCOTResults<true>(
            std::move(sender0Messages),
            std::move(receiverMessages),
            tupleSize));
    if (tupleSize > kCompositeTupleExpansionThreshold) {
      recorder_->addCompositeTuplesRequiringExpansionRequested(
          count, tupleSize);
    } else {
      recorder_->addCompositeTuplesWithoutExpansionRequested(count, tupleSize);
    }
  }
  return tuples;
}

std::pair<
    std::vector<ITupleGenerator::BooleanTuple>,
    std::map<size_t, std::vector<ITupleGenerator::CompositeBooleanTuple>>>
TwoPartyTupleGenerator::getNormalAndCompositeBooleanTuples(
    uint32_t tupleSize,
    const std::map<size_t, uint32_t>& tupleSizes) {
  auto normalTuples = getBooleanTuple(tupleSize);
  auto compositeTuples = getCompositeTuple(tupleSizes);
  return std::make_pair(std::move(normalTuples), std::move(compositeTuples));
}

std::vector<ITupleGenerator::BooleanTuple>
TwoPartyTupleGenerator::generateNormalTuples(uint64_t size) {
  {
    std::unique_lock<std::mutex> scheduleLock(scheduleMutex_);
    cv_.wait(scheduleLock, [this] { return toGenerate_.front() == Boolean; });
  }

  auto receiverMessagesFuture =
      std::async([size, this]() { return receiverRcot_->rcot(size); });

  auto sender0Messages = senderRcot_->rcot(size);
  auto receiverMessages = receiverMessagesFuture.get();

  {
    std::unique_lock<std::mutex> scheduleLock(scheduleMutex_);
    toGenerate_.pop_front();
    cv_.notify_one();
  }
  recorder_->addTuplesGenerated(size);

  return expandRCOTResults<false>(
      std::move(sender0Messages), std::move(receiverMessages), 1);
}

std::vector<std::pair<__m128i, __m128i>>
TwoPartyTupleGenerator::generateRcotResults(uint64_t size) {
  {
    std::unique_lock<std::mutex> scheduleLock(scheduleMutex_);
    cv_.wait(scheduleLock, [this] { return toGenerate_.front() == Composite; });
  }
  auto receiverMessagesFuture =
      std::async([size, this]() { return receiverRcot_->rcot(size); });

  auto sender0Messages = senderRcot_->rcot(size);
  auto receiverMessages = receiverMessagesFuture.get();

  {
    std::unique_lock<std::mutex> scheduleLock(scheduleMutex_);
    toGenerate_.pop_front();
    cv_.notify_one();
  }

  std::vector<std::pair<__m128i, __m128i>> rcotMessages(size);

  for (size_t i = 0; i < size; i++) {
    rcotMessages[i] =
        std::make_pair(sender0Messages.at(i), receiverMessages.at(i));
  }

  return rcotMessages;
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
 *
 * h is defined as a piecewise function depending on key and size
 * h(key, n) = AES_HASH(0, key) & ((1 << n) - 1) if n <= 128
 * h(key, n) = AES_PRG(key, n) if n > 128
 */
template <bool isComposite>
std::vector<TwoPartyTupleGenerator::TupleType<isComposite>>
TwoPartyTupleGenerator::expandRCOTResults(
    std::vector<__m128i> sender0Messages,
    std::vector<__m128i> receiverMessages,
    size_t requestedTupleSize) {
  std::vector<__m128i> sender1Messages(sender0Messages.size());
  std::vector<bool> choiceBits(receiverMessages.size());

  for (size_t i = 0; i < sender0Messages.size(); i++) {
    // k1 = k0 + delta1 / l1 = l0 + delta2
    sender1Messages.at(i) = _mm_xor_si128(sender0Messages.at(i), delta_);
    // r = lsb(lr) / p = lsb(kp)
    choiceBits.at(i) = util::getLsb(receiverMessages.at(i));
  }

  std::vector<TwoPartyTupleGenerator::TupleType<isComposite>> result(
      sender0Messages.size());
  if constexpr (!isComposite) {
    // H(k0) / H(l0)
    hashFromAes_.inPlaceHash(sender0Messages);
    // H(k1) / H(l1)
    hashFromAes_.inPlaceHash(sender1Messages);
    // H(lr) / H(kp)
    hashFromAes_.inPlaceHash(receiverMessages);
    for (size_t i = 0; i < sender0Messages.size(); i++) {
      // a1 = H(k0) ^ H(k1) / a2 = H(l0) ^ H(l1)
      auto a = util::getLsb(sender0Messages.at(i)) ^
          util::getLsb(sender1Messages.at(i));
      // b1 = r / b2 = p
      auto b = choiceBits.at(i);
      // c1 = (H(k0) ^ H(k1)) & r ^ H(k0) + H(lr)
      //    = H(kr) + H(lr) /
      // c2 = (H(l0) ^ H(l1)) & p ^ H(l0) + H(kp)
      //    = H(lp) + H(kp)
      auto c = (a && b) ^ util::getLsb(sender0Messages.at(i)) ^
          util::getLsb(receiverMessages.at(i));

      result[i] = BooleanTuple(a, b, c);
    }
  } else {
    if (requestedTupleSize <= kCompositeTupleExpansionThreshold) {
      // H(k0) / H(l0)
      hashFromAes_.inPlaceHash(sender0Messages);
      // H(k1) / H(l1)
      hashFromAes_.inPlaceHash(sender1Messages);
      // H(lr) / H(kp)
      hashFromAes_.inPlaceHash(receiverMessages);

      for (size_t i = 0; i < sender0Messages.size(); i++) {
        // a1 = r / a2 = p
        bool a = choiceBits.at(i);
        // b1 = H(k0) ^ H(k1) / b2 = H(l0) ^ H(l1)
        __m128i b = _mm_xor_si128(sender0Messages.at(i), sender1Messages.at(i));
        // c1 = (H(k0) ^ H(k1)) & r ^ H(k0) + H(lr)
        //    = H(kr) + H(lr) /
        // c2 = (H(l0) ^ H(l1)) & p ^ H(l0) + H(kp)
        //    = H(lp) + H(kp)
        __m128i c = a
            ? _mm_xor_si128(
                  _mm_xor_si128(b, sender0Messages.at(i)),
                  receiverMessages.at(i))
            : _mm_xor_si128(sender0Messages.at(i), receiverMessages.at(i));

        std::vector<bool> bBits(requestedTupleSize);
        std::vector<bool> cBits(requestedTupleSize);
        util::extractLnbToVector(b, bBits);
        util::extractLnbToVector(c, cBits);
        result[i] = CompositeBooleanTuple(a, bBits, cBits);
      }
    } else {
      for (size_t i = 0; i < sender0Messages.size(); i++) {
        std::vector<bool> sender0Gen(requestedTupleSize);
        std::vector<bool> sender1Gen(requestedTupleSize);
        std::vector<bool> receiverGen(requestedTupleSize);
        // H(k0) / H(l0)
        util::AesPrg(sender0Messages.at(i)).getRandomBitsInPlace(sender0Gen);
        // H(k1) / H(l1)
        util::AesPrg(sender1Messages.at(i)).getRandomBitsInPlace(sender1Gen);
        // H(lr) / H(kp)
        util::AesPrg(receiverMessages.at(i)).getRandomBitsInPlace(receiverGen);

        auto a = choiceBits.at(i);
        std::vector<bool> b(requestedTupleSize);
        std::vector<bool> c(requestedTupleSize);
        for (size_t j = 0; j < requestedTupleSize; j++) {
          b[j] = sender0Gen[j] ^ sender1Gen[j];
          c[j] = (b[j] && a) ^ sender0Gen[j] ^ receiverGen[j];
        }
        result[i] = CompositeBooleanTuple(a, b, c);
      }
    }
  }

  return result;
}

std::pair<uint64_t, uint64_t> TwoPartyTupleGenerator::getTrafficStatistics()
    const {
  std::pair<uint64_t, uint64_t> rst = {0, 0};

  auto senderStats = senderRcot_->getTrafficStatistics();
  auto receiverStats = receiverRcot_->getTrafficStatistics();
  rst.first += senderStats.first + receiverStats.first;
  rst.second += senderStats.second + receiverStats.second;

  return rst;
}

} // namespace fbpcf::engine::tuple_generator
