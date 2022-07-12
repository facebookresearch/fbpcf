/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fbpcf/mpc_std_lib/util/util.h>
#include "fbpcf/mpc_std_lib/compactor/ICompactor.h"

namespace fbpcf::mpc_std_lib::compactor::insecure {

/*
 * This compactor reveals both metadata (i.e., batches) and labels and selects
 * all items whose labels are assigned to 1. It is only meant to be used as a
 * placeholder in tests.
 */

/*
 * We assume that there are two parties. In our implementation, a party with a
 * smaller id is assigned to party0 and the other party is assigned to party1.
 */

template <typename T, typename LabelT, int schedulerId>
class DummyCompactor final
    : public ICompactor<
          typename util::SecBatchType<T, schedulerId>::type,
          typename util::SecBatchType<LabelT, schedulerId>::type> {
 public:
  using SecBatchType = typename util::SecBatchType<T, schedulerId>::type;
  using SecBatchLabelType =
      typename util::SecBatchType<LabelT, schedulerId>::type;

  explicit DummyCompactor(int myId, int partnerId)
      : myId_(myId), partnerId_(partnerId) {}
  std::pair<SecBatchType, SecBatchLabelType> compaction(
      const SecBatchType& src,
      const SecBatchLabelType& label,
      size_t size,
      bool /*shouldRevealSize*/) const override {
    auto party0 =
        (myId_ < partnerId_) ? myId_ : partnerId_; // a party with a smaller id
    auto party1 =
        (myId_ < partnerId_) ? partnerId_ : myId_; // a party with a larger id

    // reveal labels and src to both parties
    auto revealedLabel0 = util::MpcAdapters<LabelT, schedulerId>::openToParty(
        label, party0); // reveal to Party0
    auto revealedLabel1 = util::MpcAdapters<LabelT, schedulerId>::openToParty(
        label, party1); // reveal to Party1
    auto plaintextLabel = (myId_ == party0) ? revealedLabel0 : revealedLabel1;

    auto revealedSrc0 = util::MpcAdapters<T, schedulerId>::openToParty(
        src, party0); // reveal to Party0
    auto revealedSrc1 = util::MpcAdapters<T, schedulerId>::openToParty(
        src, party1); // reveal to Party1
    auto plaintextSrc = (myId_ == party0) ? revealedSrc0 : revealedSrc1;

    // select items whose labels are 1.
    auto compactifiedSrc = plaintextSrc;
    auto compactifiedLabel = plaintextLabel;
    size_t outputSize = 0;
    for (size_t j = 0; j < size; j++) {
      if (plaintextLabel[j]) {
        compactifiedSrc[outputSize] = std::move(compactifiedSrc.at(j));
        compactifiedLabel[outputSize] = std::move(plaintextLabel.at(j));
        outputSize++;
      }
    }
    compactifiedSrc.resize(outputSize); // compactify
    compactifiedLabel.resize(outputSize); // compactify

    return std::make_pair(
        util::MpcAdapters<T, schedulerId>::processSecretInputs(
            compactifiedSrc, party0),
        util::MpcAdapters<LabelT, schedulerId>::processSecretInputs(
            compactifiedLabel, party0));
  }

 private:
  int myId_;
  int partnerId_;
};
} // namespace fbpcf::mpc_std_lib::compactor::insecure
