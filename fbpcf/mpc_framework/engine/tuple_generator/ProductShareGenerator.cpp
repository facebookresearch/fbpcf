/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/mpc_framework/engine/tuple_generator/ProductShareGenerator.h"
#include <assert.h>
#include <vector>

namespace fbpcf::engine::tuple_generator {

/**
 * product share generation algorithm:
 * The two parties P1, P2 hold bits a1 b1 and a2 b2 respectively. Two OT
 * instances are executed while the two parties take turns to play the role of
 * sender/receiver.
 * To run the first OT instance, P1 picks a random bit t and use t, t+a1 as the
 * ot sender's input, P2 use b2 as the ot receiver's input and receives t+a1b2.
 * Thus P1 and P2 obtain the share of a1b2 (P1 has t, P2 has t+a1b2).
 * Similarly, the two parties can obtain shares of a2b1.
 * By combining the result, the two parties can obtain the share of a1b2+a2b1
 * These two OTs can be executed in parallel, effectively resulting in a
 * bidirection OT.
 */
std::vector<bool> ProductShareGenerator::generateBooleanProductShares(
    const std::vector<bool>& left,
    const std::vector<bool>& right) {
  if (left.size() != right.size()) {
    throw std::runtime_error("Inconsistent length in inputs");
  }
  auto input0 = prg_->getRandomBits(left.size());
  assert(input0.size() == left.size());
  std::vector<bool> input1(left.size());

  for (int i = 0; i < left.size(); i++) {
    input1[i] = input0[i] ^ left[i];
  }

  auto result =
      bidirectionObliviousTransfer_->biDirectionOT(input0, input1, right);
  for (int i = 0; i < result.size(); i++) {
    result[i] = result[i] ^ input0[i];
  }
  return result;
}

} // namespace fbpcf::engine::tuple_generator
