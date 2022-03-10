/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/DummyBaseObliviousTransfer.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::insecure {

std::pair<std::vector<__m128i>, std::vector<__m128i>>
DummyBaseObliviousTransfer::send(
    size_t size) { // this is a dummy implementation
  std::vector<__m128i> m0(size);
  std::vector<__m128i> m1(size);
  for (size_t i = 0; i < size; i++) {
    m0[i] = _mm_set_epi64x(i, 0);
    m1[i] = _mm_set_epi64x(i, 1);
  }
  return {std::move(m0), std::move(m1)};
}

std::vector<__m128i> DummyBaseObliviousTransfer::receive(
    const std::vector<bool>& choice) {
  // this is a dummy implementation
  std::vector<__m128i> m(choice.size());
  for (size_t i = 0; i < choice.size(); i++) {
    m[i] = _mm_set_epi64x(i, choice.at(i));
  }
  return m;
}

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::insecure
