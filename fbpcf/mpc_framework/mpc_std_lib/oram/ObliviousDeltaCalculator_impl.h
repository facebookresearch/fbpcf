/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

namespace fbpcf::mpc_framework::mpc_std_lib::oram {

template <int schedulerId>
std::tuple<std::vector<__m128i>, std::vector<bool>, std::vector<bool>>
ObliviousDeltaCalculator<schedulerId>::calculateDelta(
    const std::vector<__m128i>& delta0Shares,
    const std::vector<__m128i>& delta1Shares,
    const std::vector<bool>& alphaShares) const {
  auto delta0SharesBool = util::convertToBits(delta0Shares);
  auto delta1SharesBool = util::convertToBits(delta1Shares);
  std::vector<frontend::Bit<true, schedulerId, true>> secDelta0(
      128 /* there are 128 bits in __m128i */);
  std::vector<frontend::Bit<true, schedulerId, true>> secDelta1(
      128 /* there are 128 bits in __m128i */);
  std::vector<frontend::Bit<true, schedulerId, true>> secDelta(
      128 /* there are 128 bits in __m128i */);

  frontend::Bit<true, schedulerId, true> secAlpha(
      (typename frontend::Bit<true, schedulerId, true>::ExtractedBit(
          alphaShares)));

  for (size_t i = 0; i < kBitsInM128i; i++) {
    secDelta0[i] = frontend::Bit<true, schedulerId, true>(
        typename frontend::Bit<true, schedulerId, true>::ExtractedBit(
            delta0SharesBool.at(i)));
    secDelta1[i] = frontend::Bit<true, schedulerId, true>(
        typename frontend::Bit<true, schedulerId, true>::ExtractedBit(
            delta1SharesBool.at(i)));
    secDelta[i] =
        secDelta1.at(i) ^ ((secDelta0.at(i) ^ secDelta1.at(i)) & secAlpha);
  }
  auto secT0 = secDelta0.at(0) ^ !secAlpha;
  auto secT1 = secDelta1.at(0) ^ secAlpha;

  auto t0Party0 = secT0.openToParty(party0Id_);
  auto t0Party1 = secT0.openToParty(party1Id_);

  auto t1Party0 = secT1.openToParty(party0Id_);
  auto t1Party1 = secT1.openToParty(party1Id_);

  std::vector<frontend::Bit<false, schedulerId, true>> deltaBoolParty0(
      kBitsInM128i);
  std::vector<frontend::Bit<false, schedulerId, true>> deltaBoolParty1(
      kBitsInM128i);

  std::vector<std::vector<bool>> deltaBool(kBitsInM128i);

  for (size_t i = 0; i < kBitsInM128i; i++) {
    deltaBoolParty0[i] = secDelta.at(i).openToParty(party0Id_);
  }
  for (size_t i = 0; i < kBitsInM128i; i++) {
    deltaBoolParty1[i] = secDelta.at(i).openToParty(party1Id_);
  }

  for (size_t i = 0; i < kBitsInM128i; i++) {
    deltaBool[i] = amIParty0_ ? deltaBoolParty0.at(i).getValue()
                              : deltaBoolParty1.at(i).getValue();
  }
  std::vector<__m128i> delta = util::convertFromBits(deltaBool);
  auto t0 = amIParty0_ ? t0Party0.getValue() : t0Party1.getValue();
  auto t1 = amIParty0_ ? t1Party0.getValue() : t1Party1.getValue();

  return {delta, t0, t1};
}

} // namespace fbpcf::mpc_framework::mpc_std_lib::oram
