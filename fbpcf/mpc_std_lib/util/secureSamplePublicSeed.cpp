/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/mpc_std_lib/util/secureSamplePublicSeed.h"

namespace fbpcf::mpc_std_lib::util {

__m128i secureSamplePublicSeed(
    bool amISendingFirst,
    engine::communication::IPartyCommunicationAgent& agent) {
  if (amISendingFirst) {
    std::vector<__m128i> myShareWithSalt(2);
    myShareWithSalt.at(0) = engine::util::getRandomM128iFromSystemNoise();
    myShareWithSalt.at(1) = engine::util::getRandomM128iFromSystemNoise();

    std::vector<unsigned char> myDigest(SHA256_DIGEST_LENGTH);

    SHA256(
        reinterpret_cast<const unsigned char*>(myShareWithSalt.data()),
        sizeof(__m128i) * 2,
        myDigest.data());

    agent.send(myDigest);
    auto peerShare = agent.receiveSingleT<__m128i>();
    agent.sendT(myShareWithSalt);

    return _mm_xor_si128(myShareWithSalt.at(0), peerShare);

  } else {
    auto myShare = engine::util::getRandomM128iFromSystemNoise();

    std::vector<unsigned char> peerClaimedDigest(SHA256_DIGEST_LENGTH);
    peerClaimedDigest = agent.receive(SHA256_DIGEST_LENGTH);

    agent.sendSingleT(myShare);

    auto peerShareAndSalt = agent.receiveT<__m128i>(2);

    std::vector<unsigned char> peerActualDigest(SHA256_DIGEST_LENGTH);
    SHA256(
        reinterpret_cast<const unsigned char*>(peerShareAndSalt.data()),
        sizeof(__m128i) * 2,
        peerActualDigest.data());
    for (size_t i = 0; i < SHA256_DIGEST_LENGTH; i++) {
      if (peerClaimedDigest.at(i) != peerActualDigest.at(i)) {
        throw std::runtime_error(
            "Peer's share's hash doesn't match their claim.");
      }
    }
    return _mm_xor_si128(myShare, peerShareAndSalt.at(0));
  }
}

} // namespace fbpcf::mpc_std_lib::util
