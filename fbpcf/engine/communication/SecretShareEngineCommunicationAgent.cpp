/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <emmintrin.h>
#include <string.h>
#include <stdexcept>

#include "fbpcf/engine/communication/SecretShareEngineCommunicationAgent.h"

namespace fbpcf::engine::communication {
std::map<int, __m128i> SecretShareEngineCommunicationAgent::exchangeKeys(
    const std::map<int, __m128i>& keys) {
  std::map<int, __m128i> rst;
  for (auto& iter : keys) {
    agentMap_[iter.first]->sendSingleT<__m128i>(iter.second);
    rst.emplace(iter.first, agentMap_[iter.first]->receiveSingleT<__m128i>());
  }
  return rst;
}

std::vector<bool> SecretShareEngineCommunicationAgent::openSecretsToAll(
    const std::vector<bool>& secretShares) {
  std::vector<bool> rst = secretShares;
  std::vector<bool> receivedShares;

  // exchange the share with all the peers
  for (auto& iter : agentMap_) {
    if (iter.first < myId_) {
      iter.second->sendBool(secretShares);
      receivedShares = iter.second->receiveBool(secretShares.size());
    } else {
      receivedShares = iter.second->receiveBool(secretShares.size());
      iter.second->sendBool(secretShares);
    }
    for (size_t i = 0; i < rst.size(); i++) {
      rst[i] = rst[i] ^ receivedShares[i];
    }
  }
  return rst;
}

std::vector<uint64_t> SecretShareEngineCommunicationAgent::openSecretsToAll(
    const std::vector<uint64_t>& secretShares) {
  std::vector<uint64_t> rst = secretShares;
  std::vector<uint64_t> receivedShares;

  // exchange the share with all the peers
  for (auto& iter : agentMap_) {
    if (iter.first < myId_) {
      iter.second->sendInt64(secretShares);
      receivedShares = iter.second->receiveInt64(secretShares.size());
    } else {
      receivedShares = iter.second->receiveInt64(secretShares.size());
      iter.second->sendInt64(secretShares);
    }
    for (size_t i = 0; i < rst.size(); i++) {
      rst[i] = rst[i] + receivedShares[i];
    }
  }
  return rst;
}

std::vector<bool> SecretShareEngineCommunicationAgent::openSecretsToParty(
    int id,
    const std::vector<bool>& secretShares) {
  if (id == myId_) {
    std::vector<bool> rst = secretShares;
    for (auto& iter : agentMap_) {
      auto receivedShares = iter.second->receiveBool(secretShares.size());
      for (size_t i = 0; i < rst.size(); i++) {
        rst[i] = rst[i] ^ receivedShares[i];
      }
    }
    return rst;
  } else {
    agentMap_.at(id)->sendBool(secretShares);
    return std::vector<bool>(secretShares.size());
  }
}

std::vector<uint64_t> SecretShareEngineCommunicationAgent::openSecretsToParty(
    int id,
    const std::vector<uint64_t>& secretShares) {
  if (id == myId_) {
    std::vector<uint64_t> rst = secretShares;
    for (auto& iter : agentMap_) {
      auto receivedShares = iter.second->receiveInt64(secretShares.size());
      for (size_t i = 0; i < rst.size(); i++) {
        rst[i] = rst.at(i) + receivedShares.at(i);
      }
    }
    return rst;
  } else {
    agentMap_.at(id)->sendInt64(secretShares);
    return std::vector<uint64_t>(secretShares.size());
  }
}

std::pair<uint64_t, uint64_t>
SecretShareEngineCommunicationAgent::getTrafficStatistics() const {
  uint64_t sent = 0;
  uint64_t received = 0;
  for (auto& item : agentMap_) {
    auto traffic = item.second->getTrafficStatistics();
    sent += traffic.first;
    received += traffic.second;
  }
  return {sent, received};
}

} // namespace fbpcf::engine::communication
