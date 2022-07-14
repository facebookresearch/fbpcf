/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include "fbpcf/frontend/mpcGame.h"

namespace fbpcf::edit_distance {
/**
 * Share integer, with width number of bits, from sender to receiver. The
 * integer is revealed in plaintext to the receiver.
 */
template <int schedulerId, size_t width, int sender, int receiver>
uint64_t shareIntFrom(const int myRole, uint64_t input) {
  // Sender shares input
  typename fbpcf::frontend::MpcGame<
      schedulerId>::template SecUnsignedInt<width, false>
      secInput{input, sender};
  // Reveal to receiver
  uint64_t output = secInput.openToParty(receiver).getValue();
  return (myRole == sender) ? input : output;
}

template <typename T, typename O>
T createPublicBatchConstant(O ele, size_t size) {
  std::vector<O> copies(size, ele);
  return T(copies);
}

template <typename T, typename O>
T createSecretBatchConstant(O ele, size_t size, int partyId) {
  std::vector<O> copies(size, ele);
  return T(copies, partyId);
}

} // namespace fbpcf::edit_distance
