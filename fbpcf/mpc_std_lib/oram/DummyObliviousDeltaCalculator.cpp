/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/mpc_std_lib/oram/DummyObliviousDeltaCalculator.h"
#include "fbpcf/engine/util/util.h"

namespace fbpcf::mpc_std_lib::oram::insecure {
std::tuple<std::vector<__m128i>, std::vector<bool>, std::vector<bool>>
DummyObliviousDeltaCalculator::calculateDelta(
    const std::vector<__m128i>& delta0Shares,
    const std::vector<__m128i>& delta1Shares,
    const std::vector<bool>& alphaShares) const {
  size_t size = delta0Shares.size();
  if ((delta1Shares.size() != size) || (alphaShares.size() != size)) {
    throw std::invalid_argument("Inconsistent size.");
  }
  agent_->sendT<__m128i>(delta0Shares);
  agent_->sendT<__m128i>(delta1Shares);
  agent_->sendBool(alphaShares);
  auto otherDelta0 = agent_->receiveT<__m128i>(size);
  auto otherDelta1 = agent_->receiveT<__m128i>(size);
  auto otherAlpha = agent_->receiveBool(size);
  std::vector<__m128i> delta(size);
  std::vector<bool> t0(size);
  std::vector<bool> t1(size);

  for (size_t i = 0; i < size; i++) {
    otherDelta0[i] = _mm_xor_si128(otherDelta0.at(i), delta0Shares.at(i));
    otherDelta1[i] = _mm_xor_si128(otherDelta1.at(i), delta1Shares.at(i));
    otherAlpha[i] = otherAlpha.at(i) ^ alphaShares.at(i);
    delta[i] = otherAlpha.at(i) ? otherDelta0.at(i) : otherDelta1.at(i);
    t0[i] = engine::util::getLsb(otherDelta0.at(i)) ^ !otherAlpha.at(i);
    t1[i] = engine::util::getLsb(otherDelta1.at(i)) ^ otherAlpha.at(i);
  }
  return {delta, t0, t1};
}

} // namespace fbpcf::mpc_std_lib::oram::insecure
