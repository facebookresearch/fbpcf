/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <assert.h>

#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/DummyRcotExtender.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::ferret::insecure {

int DummyRcotExtender::senderInit(
    __m128i /*delta*/,
    int64_t extendedSize,
    int64_t baseSize,
    int64_t /*weight*/) {
  role_ = util::Role::sender;
  extendedSize_ = extendedSize;
  baseSize_ = baseSize;
  return baseSize_;
}

int DummyRcotExtender::receiverInit(
    int64_t extendedSize,
    int64_t baseSize,
    int64_t /*weight*/) {
  role_ = util::Role::receiver;
  extendedSize_ = extendedSize;
  baseSize_ = baseSize;
  return baseSize_;
}

std::vector<__m128i> DummyRcotExtender::senderExtendRcot(
    std::vector<__m128i>&& baseRcot) {
  assert(role_ == util::Role::sender);
  assert(baseRcot.size() == baseSize_);

  std::vector<__m128i> rst(extendedSize_);
  for (int i = 0; i < extendedSize_; i++) {
    rst[i] = baseRcot[i % baseSize_];
  }
  return rst;
}

std::vector<__m128i> DummyRcotExtender::receiverExtendRcot(
    std::vector<__m128i>&& baseRcot) {
  assert(role_ == util::Role::receiver);
  assert(baseRcot.size() == baseSize_);

  std::vector<__m128i> rst(extendedSize_);
  for (int i = 0; i < extendedSize_; i++) {
    rst[i] = baseRcot[i % baseSize_];
  }
  return rst;
}

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::ferret::insecure
