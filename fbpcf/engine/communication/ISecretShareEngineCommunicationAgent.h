/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <emmintrin.h>
#include <cstdint>
#include <map>
#include <vector>

namespace fbpcf::engine::communication {

/**
 * This class abstract all the communication patterns in a secret-share-based
 * engine such that the engine itself doesn't need to directly send/receive
 * messages from other parties.
 */

class ISecretShareEngineCommunicationAgent {
 public:
  virtual ~ISecretShareEngineCommunicationAgent() = default;

  /**
    Each party freshly picks a random PRG key for one of his/her peers and share
    with them.
    @param keys the map of party id to key picked for that party;
    @param return the map of party id to key received from that party;
  */
  virtual std::map<int, __m128i> exchangeKeys(
      const std::map<int, __m128i>& keys) = 0;

  /**
   * Jointly open a vector of secrets to every party.
   * @param secretShares my share of the secrets
   * @return the revealed secrets
   */
  virtual std::vector<bool> openSecretsToAll(
      const std::vector<bool>& secretShares) = 0;

  /**
   * Jointly open a vector of secrets to every party.
   * @param secretShares my share of the secrets
   * @return the revealed secrets
   */
  virtual std::vector<uint64_t> openSecretsToAll(
      const std::vector<uint64_t>& secretShares) = 0;

  /**
   * Jointly open a vector of secrets to a particular party.
   * @param id the the party to revcvevive the secrets.
   * @param secretShares my share of the secrets
   * @return the revealed secrets if this party is designed to received them;
   * otherwise an array of dummy values
   */
  virtual std::vector<bool> openSecretsToParty(
      int id,
      const std::vector<bool>& secretShares) = 0;

  /**
   * Jointly open a vector of secrets to a particular party.
   * @param id the the party to revcvevive the secrets.
   * @param secretShares my share of the secrets
   * @return the revealed secrets if this party is designed to received them;
   * otherwise an array of dummy values
   */
  virtual std::vector<uint64_t> openSecretsToParty(
      int id,
      const std::vector<uint64_t>& secretShares) = 0;

  /**
   * Get the total amount of traffic transmitted.
   * @return a pair of (sent, received) data in bytes.
   */
  virtual std::pair<uint64_t, uint64_t> getTrafficStatistics() const = 0;
};

} // namespace fbpcf::engine::communication
