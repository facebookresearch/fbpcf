/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fbpcf/engine/util/util.h>
#include "fbpcf/mpc_std_lib/compactor/ICompactor.h"
#include "fbpcf/mpc_std_lib/shuffler/IShuffler.h"
#include "fbpcf/mpc_std_lib/util/util.h"

namespace fbpcf::mpc_std_lib::compactor {

/*
 * This compactor first shuffles given metadata and labels. Then, by revealing
 * the resulting shuffled labels, it selects all items whose labels are assigned
 * to 1. We utilize a shuffler API to shuffle the data.
 */

/*
 * We assume that there are two parties. In our implementation, a party with a
 * smaller id is assigned to party0 and the other party is assigned to party1.
 */
template <typename T, typename LabelT, int schedulerId>
class ShuffleBasedCompactor final
    : public ICompactor<
          typename util::SecBatchType<T, schedulerId>::type,
          typename util::SecBatchType<LabelT, schedulerId>::type> {
 public:
  using SecBatchType =
      typename util::SecBatchType<std::pair<T, LabelT>, schedulerId>::type;
  using SecBatchSrcType = typename util::SecBatchType<T, schedulerId>::type;
  using SecBatchLabelType =
      typename util::SecBatchType<LabelT, schedulerId>::type;
  ShuffleBasedCompactor(
      int myId,
      int partnerId,
      std::unique_ptr<shuffler::IShuffler<SecBatchType>> shuffler)
      : myId_(myId), partnerId_(partnerId), shuffler_(std::move(shuffler)) {}

  std::pair<SecBatchSrcType, SecBatchLabelType> compaction(
      const SecBatchSrcType& src,
      const SecBatchLabelType& label,
      size_t size,
      bool shouldRevealSize) const {
    if (!shouldRevealSize) {
      throw std::runtime_error("shouldRevealSize should be true");
    }

    // shuffle the data which includes both src and label.
    auto shuffled = shuffler_->shuffle({src, label}, size);
    auto shuffledSrc = shuffled.first;
    auto shuffledLabel = shuffled.second;

    // reveal Label to both party;
    auto party0 =
        (myId_ < partnerId_) ? myId_ : partnerId_; // a party with a smaller id
    auto party1 =
        (myId_ < partnerId_) ? partnerId_ : myId_; // a party with a larger id

    auto revealedLabel0 = util::MpcAdapters<LabelT, schedulerId>::openToParty(
        shuffledLabel, party0); // reveal to party0
    auto revealedLabel1 = util::MpcAdapters<LabelT, schedulerId>::openToParty(
        shuffledLabel, party1); // reveal to party1
    auto plaintextLabel = (myId_ == party0) ? revealedLabel0 : revealedLabel1;

    // unbatch the batch type shuffledSrc and shuffledLabel into a vector format
    // to select elements that are labeled as 1.
    auto unbatchSize = std::make_shared<std::vector<uint32_t>>(size);
    for (size_t i = 0; i < size; i++) {
      (*unbatchSize)[i] = 1;
    }
    auto shuffledSrcBatches = util::MpcAdapters<T, schedulerId>::unbatching(
        shuffledSrc, unbatchSize); // unbatch
    auto shuffledLabelBatches =
        util::MpcAdapters<LabelT, schedulerId>::unbatching(
            shuffledLabel, unbatchSize); // unbatch

    // select all 1s items.
    std::vector<SecBatchSrcType> compactifiedSrc;
    std::vector<SecBatchLabelType> compactifiedLabel;
    for (size_t i = 0; i < size; i++) {
      if (plaintextLabel.at(i)) {
        compactifiedSrc.push_back(shuffledSrcBatches.at(i));
        compactifiedLabel.push_back(shuffledLabelBatches.at(i));
      }
    }

    // rebatch
    auto rstLabel = util::MpcAdapters<LabelT, schedulerId>::batchingWith(
        compactifiedLabel.at(0),
        std::vector<SecBatchLabelType>(
            compactifiedLabel.begin() + 1, compactifiedLabel.end()));
    auto rstSrc = util::MpcAdapters<T, schedulerId>::batchingWith(
        compactifiedSrc.at(0),
        std::vector<SecBatchSrcType>(
            compactifiedSrc.begin() + 1, compactifiedSrc.end()));
    return {rstSrc, rstLabel};
  }

 private:
  int myId_;
  int partnerId_;
  std::unique_ptr<shuffler::IShuffler<SecBatchType>> shuffler_;
};
} // namespace fbpcf::mpc_std_lib::compactor
