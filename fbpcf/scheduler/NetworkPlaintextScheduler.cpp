/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cstdint>
#include <stdexcept>
#include <vector>

#include <fbpcf/scheduler/PlaintextScheduler.h>
#include "fbpcf/scheduler/NetworkPlaintextScheduler.h"

namespace fbpcf::scheduler {

NetworkPlaintextScheduler::NetworkPlaintextScheduler(
    int myId,
    std::map<
        int,
        std::unique_ptr<engine::communication::IPartyCommunicationAgent>>
        agentMap,
    std::unique_ptr<IWireKeeper> wireKeeper)
    : PlaintextScheduler{std::move(wireKeeper)},
      myId_{myId},
      agentMap_{std::move(agentMap)} {}

IScheduler::WireId<IScheduler::Boolean>
NetworkPlaintextScheduler::privateBooleanInput(bool v, int partyId) {
  freeGates_++;
  if (partyId == myId_) {
    // Send my input to all other parties
    for (auto& iter : agentMap_) {
      iter.second->sendSingleT<bool>(v);
    }
    return wireKeeper_->allocateBooleanValue(v);
  }
  auto otherV = agentMap_.at(partyId)->receiveSingleT<bool>();
  return wireKeeper_->allocateBooleanValue(otherV);
}

IScheduler::WireId<IScheduler::Boolean>
NetworkPlaintextScheduler::privateBooleanInputBatch(
    const std::vector<bool>& v,
    int partyId) {
  freeGates_ += v.size();
  if (partyId == myId_) {
    // Send my input to all other parties
    for (auto& iter : agentMap_) {
      iter.second->sendBool(v);
    }
    return wireKeeper_->allocateBatchBooleanValue(v);
  }
  auto otherV = agentMap_.at(partyId)->receiveBool(v.size());
  return wireKeeper_->allocateBatchBooleanValue(otherV);
}

IScheduler::WireId<IScheduler::Boolean>
NetworkPlaintextScheduler::recoverBooleanWire(bool v) {
  freeGates_++;

  bool result = v;
  bool receivedShare;

  // XOR the shares from all parties to recover the true value
  for (auto& iter : agentMap_) {
    if (iter.first < myId_) {
      iter.second->sendSingleT<bool>(v);
      receivedShare = iter.second->receiveSingleT<bool>();
    } else {
      receivedShare = iter.second->receiveSingleT<bool>();
      iter.second->sendSingleT<bool>(v);
    }
    result ^= receivedShare;
  }

  return wireKeeper_->allocateBooleanValue(result);
}

IScheduler::WireId<IScheduler::Boolean>
NetworkPlaintextScheduler::recoverBooleanWireBatch(const std::vector<bool>& v) {
  freeGates_ += v.size();

  std::vector<bool> result = v;
  std::vector<bool> receivedShares;

  // XOR the shares from all parties to recover the true value
  for (auto& iter : agentMap_) {
    if (iter.first < myId_) {
      iter.second->sendBool(v);
      receivedShares = iter.second->receiveBool(v.size());
    } else {
      receivedShares = iter.second->receiveBool(v.size());
      iter.second->sendBool(v);
    }
    for (size_t i = 0; i < result.size(); i++) {
      result[i] = result.at(i) ^ receivedShares.at(i);
    }
  }

  return wireKeeper_->allocateBatchBooleanValue(result);
}

bool NetworkPlaintextScheduler::extractBooleanSecretShare(
    WireId<IScheduler::Boolean> id) {
  // Party 0 gets the actual value.
  // Other parties get false, so all parties' shares XOR to the actual value.
  if (myId_ == 0) {
    return wireKeeper_->getBooleanValue(id);
  } else {
    return false;
  }
}

std::vector<bool> NetworkPlaintextScheduler::extractBooleanSecretShareBatch(
    WireId<IScheduler::Boolean> id) {
  // Party 0 gets the actual value.
  // Other parties get false, so all parties' shares XOR to the actual value.
  auto result = wireKeeper_->getBatchBooleanValue(id);
  if (myId_ == 0) {
    return result;
  } else {
    return std::vector<bool>(result.size(), false);
  }
}

IScheduler::WireId<IScheduler::Arithmetic>
NetworkPlaintextScheduler::privateIntegerInput(uint64_t v, int partyId) {
  freeGates_++;
  if (partyId == myId_) {
    // Send my input to all other parties
    for (auto& iter : agentMap_) {
      iter.second->sendInt64({v});
    }
    return wireKeeper_->allocateIntegerValue(v);
  }
  auto otherV = agentMap_.at(partyId)->receiveInt64(1)[0];
  return wireKeeper_->allocateIntegerValue(otherV);
}

IScheduler::WireId<IScheduler::Arithmetic>
NetworkPlaintextScheduler::privateIntegerInputBatch(
    const std::vector<uint64_t>& v,
    int partyId) {
  freeGates_ += v.size();
  if (partyId == myId_) {
    // Send my input to all other parties
    for (auto& iter : agentMap_) {
      iter.second->sendInt64(v);
    }
    return wireKeeper_->allocateBatchIntegerValue(v);
  }
  auto otherV = agentMap_.at(partyId)->receiveInt64(v.size());
  return wireKeeper_->allocateBatchIntegerValue(otherV);
}

IScheduler::WireId<IScheduler::Arithmetic>
NetworkPlaintextScheduler::recoverIntegerWire(uint64_t v) {
  freeGates_++;

  uint64_t result = v;
  uint64_t receivedShare;

  // XOR the shares from all parties to recover the true value
  for (auto& iter : agentMap_) {
    if (iter.first < myId_) {
      iter.second->sendInt64({v});
      receivedShare = iter.second->receiveInt64(1)[0];
    } else {
      receivedShare = iter.second->receiveInt64(1)[0];
      iter.second->sendInt64({v});
    }
    result += receivedShare;
  }

  return wireKeeper_->allocateIntegerValue(result);
}

IScheduler::WireId<IScheduler::Arithmetic>
NetworkPlaintextScheduler::recoverIntegerWireBatch(
    const std::vector<uint64_t>& v) {
  freeGates_ += v.size();

  std::vector<uint64_t> result = v;
  std::vector<uint64_t> receivedShares;

  // XOR the shares from all parties to recover the true value
  for (auto& iter : agentMap_) {
    if (iter.first < myId_) {
      iter.second->sendInt64(v);
      receivedShares = iter.second->receiveInt64(v.size());
    } else {
      receivedShares = iter.second->receiveInt64(v.size());
      iter.second->sendInt64(v);
    }
    for (size_t i = 0; i < result.size(); i++) {
      result[i] = result.at(i) + receivedShares.at(i);
    }
  }

  return wireKeeper_->allocateBatchIntegerValue(result);
}

uint64_t NetworkPlaintextScheduler::extractIntegerSecretShare(
    WireId<IScheduler::Arithmetic> id) {
  // Party 0 gets the actual value.
  // Other parties get false, so all parties' shares XOR to the actual value.
  if (myId_ == 0) {
    return wireKeeper_->getIntegerValue(id);
  } else {
    return 0;
  }
}

std::vector<uint64_t> NetworkPlaintextScheduler::extractIntegerSecretShareBatch(
    WireId<IScheduler::Arithmetic> id) {
  // Party 0 gets the actual value.
  // Other parties get false, so all parties' shares XOR to the actual value.
  auto result = wireKeeper_->getBatchIntegerValue(id);
  if (myId_ == 0) {
    return result;
  } else {
    return std::vector<uint64_t>(result.size(), 0);
  }
}

} // namespace fbpcf::scheduler
