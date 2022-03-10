/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ExtenderBasedRandomCorrelatedObliviousTransfer.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer {

ExtenderBasedRandomCorrelatedObliviousTransfer::
    ExtenderBasedRandomCorrelatedObliviousTransfer(
        util::Role role,
        std::unique_ptr<ferret::IRcotExtender> rcotExtender)
    : role_(role), rcotExtender_(std::move(rcotExtender)) {
  baseRcotSize_ = rcotExtender_->getBaseCotSize();
}

std::vector<__m128i> ExtenderBasedRandomCorrelatedObliviousTransfer::rcot(
    int64_t size) {
  std::vector<__m128i> rst;

  int index = 0;
  while (index < size) {
    if (otIndex_ >= rcotResults_.size()) {
      extendRcot();
    }
    auto insertSize =
        std::min<int64_t>(size - index, rcotResults_.size() - otIndex_);

    rst.insert(
        rst.end(),
        std::make_move_iterator(rcotResults_.begin() + otIndex_),
        std::make_move_iterator(rcotResults_.begin() + otIndex_ + insertSize));
    otIndex_ += insertSize;
    index += insertSize;
  }
  return rst;
}

void ExtenderBasedRandomCorrelatedObliviousTransfer::extendRcot() {
  assert(baseRcotResults_.size() == baseRcotSize_);

  switch (role_) {
    case util::Role::sender:
      rcotResults_ =
          rcotExtender_->senderExtendRcot(std::move(baseRcotResults_));
      break;
    case util::Role::receiver:
      rcotResults_ =
          rcotExtender_->receiverExtendRcot(std::move(baseRcotResults_));
      break;
  }

  // otherwise the extension won't make sense at all.
  assert(rcotResults_.size() > baseRcotSize_);

  baseRcotResults_ = std::vector<__m128i>(
      rcotResults_.end() - baseRcotSize_, rcotResults_.end());

  rcotResults_.erase(rcotResults_.end() - baseRcotSize_, rcotResults_.end());

  otIndex_ = 0;
}

} // namespace fbpcf::engine::tuple_generator::oblivious_transfer
