/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/scheduler/LazyScheduler.h"

#include <cstdint>
#include <exception>
#include <map>
#include <stdexcept>
#include <string>

#include <fbpcf/scheduler/gate_keeper/GateKeeper.h>
#include <fbpcf/scheduler/gate_keeper/INormalGate.h>
#include "fbpcf/scheduler/IScheduler.h"
#include "fbpcf/scheduler/gate_keeper/IGate.h"
#include "fbpcf/scheduler/gate_keeper/INormalGate.h"

namespace fbpcf::scheduler {

LazyScheduler::LazyScheduler(
    std::unique_ptr<engine::ISecretShareEngine> engine,
    std::shared_ptr<IWireKeeper> wireKeeper,
    std::unique_ptr<IGateKeeper> gateKeeper,
    std::shared_ptr<util::MetricCollector> collector)
    : engine_{std::move(engine)},
      wireKeeper_{std::move(wireKeeper)},
      gateKeeper_{std::move(gateKeeper)},
      collector_{collector} {}

IScheduler::WireId<IScheduler::Boolean> LazyScheduler::privateBooleanInput(
    bool v,
    int partyId) {
  auto id = gateKeeper_->inputGate(engine_->setInput(partyId, v));
  maybeExecuteGates();
  return id;
}

IScheduler::WireId<IScheduler::Boolean> LazyScheduler::privateBooleanInputBatch(
    const std::vector<bool>& v,
    int partyId) {
  auto id = gateKeeper_->inputGateBatch(engine_->setBatchInput(partyId, v));
  maybeExecuteGates();
  return id;
}

IScheduler::WireId<IScheduler::Boolean> LazyScheduler::publicBooleanInput(
    bool v) {
  auto id = gateKeeper_->inputGate(v);
  maybeExecuteGates();
  return id;
}

IScheduler::WireId<IScheduler::Boolean> LazyScheduler::publicBooleanInputBatch(
    const std::vector<bool>& v) {
  auto id = gateKeeper_->inputGateBatch(v);
  maybeExecuteGates();
  return id;
}

IScheduler::WireId<IScheduler::Boolean> LazyScheduler::recoverBooleanWire(
    bool v) {
  auto id = gateKeeper_->inputGate(v);
  maybeExecuteGates();
  return id;
}

IScheduler::WireId<IScheduler::Boolean> LazyScheduler::recoverBooleanWireBatch(
    const std::vector<bool>& v) {
  auto id = gateKeeper_->inputGateBatch(v);
  maybeExecuteGates();
  return id;
}

IScheduler::WireId<IScheduler::Boolean> LazyScheduler::openBooleanValueToParty(
    WireId<IScheduler::Boolean> src,
    int partyId) {
  auto id = gateKeeper_->outputGate(src, partyId);
  maybeExecuteGates();
  return id;
}

IScheduler::WireId<IScheduler::Boolean>
LazyScheduler::openBooleanValueToPartyBatch(
    WireId<IScheduler::Boolean> src,
    int partyId) {
  auto id = gateKeeper_->outputGateBatch(src, partyId);
  maybeExecuteGates();
  return id;
}

bool LazyScheduler::extractBooleanSecretShare(WireId<IScheduler::Boolean> id) {
  return forceWire<false>(id);
}

std::vector<bool> LazyScheduler::extractBooleanSecretShareBatch(
    WireId<IScheduler::Boolean> id) {
  return forceWire<true>(id);
}

bool LazyScheduler::getBooleanValue(WireId<IScheduler::Boolean> id) {
  return forceWire<false>(id);
}

std::vector<bool> LazyScheduler::getBooleanValueBatch(
    WireId<IScheduler::Boolean> id) {
  return forceWire<true>(id);
}

IScheduler::WireId<IScheduler::Boolean> LazyScheduler::privateAndPrivate(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  auto id =
      gateKeeper_->normalGate(INormalGate::GateType::NonFreeAnd, left, right);
  maybeExecuteGates();
  return id;
}

IScheduler::WireId<IScheduler::Boolean> LazyScheduler::privateAndPrivateBatch(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  auto id = gateKeeper_->normalGateBatch(
      INormalGate::GateType::NonFreeAnd, left, right);
  maybeExecuteGates();
  return id;
}

IScheduler::WireId<IScheduler::Boolean> LazyScheduler::privateAndPublic(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  auto id =
      gateKeeper_->normalGate(INormalGate::GateType::FreeAnd, left, right);
  maybeExecuteGates();
  return id;
}

IScheduler::WireId<IScheduler::Boolean> LazyScheduler::privateAndPublicBatch(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  auto id =
      gateKeeper_->normalGateBatch(INormalGate::GateType::FreeAnd, left, right);
  maybeExecuteGates();
  return id;
}

IScheduler::WireId<IScheduler::Boolean> LazyScheduler::publicAndPublic(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  auto id =
      gateKeeper_->normalGate(INormalGate::GateType::FreeAnd, left, right);
  maybeExecuteGates();
  return id;
}

IScheduler::WireId<IScheduler::Boolean> LazyScheduler::publicAndPublicBatch(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  auto id =
      gateKeeper_->normalGateBatch(INormalGate::GateType::FreeAnd, left, right);
  maybeExecuteGates();
  return id;
}

std::vector<IScheduler::WireId<IScheduler::Boolean>>
LazyScheduler::privateAndPrivateComposite(
    IScheduler::WireId<IScheduler::Boolean> left,
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) {
  auto id = gateKeeper_->compositeGate(
      ICompositeGate::GateType::NonFreeAnd, left, rights);
  maybeExecuteGates();
  return id;
}

std::vector<IScheduler::WireId<IScheduler::Boolean>>
LazyScheduler::privateAndPrivateCompositeBatch(
    IScheduler::WireId<IScheduler::Boolean> left,
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) {
  auto id = gateKeeper_->compositeGateBatch(
      ICompositeGate::GateType::NonFreeAnd, left, rights);
  maybeExecuteGates();
  return id;
}

std::vector<IScheduler::WireId<IScheduler::Boolean>>
LazyScheduler::privateAndPublicComposite(
    IScheduler::WireId<IScheduler::Boolean> left,
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) {
  auto id = gateKeeper_->compositeGate(
      ICompositeGate::GateType::FreeAnd, left, rights);
  maybeExecuteGates();
  return id;
}

std::vector<IScheduler::WireId<IScheduler::Boolean>>
LazyScheduler::privateAndPublicCompositeBatch(
    IScheduler::WireId<IScheduler::Boolean> left,
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) {
  auto id = gateKeeper_->compositeGateBatch(
      ICompositeGate::GateType::FreeAnd, left, rights);
  maybeExecuteGates();
  return id;
}

std::vector<IScheduler::WireId<IScheduler::Boolean>>
LazyScheduler::publicAndPublicComposite(
    IScheduler::WireId<IScheduler::Boolean> left,
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) {
  auto id = gateKeeper_->compositeGate(
      ICompositeGate::GateType::FreeAnd, left, rights);
  maybeExecuteGates();
  return id;
}

std::vector<IScheduler::WireId<IScheduler::Boolean>>
LazyScheduler::publicAndPublicCompositeBatch(
    IScheduler::WireId<IScheduler::Boolean> left,
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) {
  auto id = gateKeeper_->compositeGateBatch(
      ICompositeGate::GateType::FreeAnd, left, rights);
  maybeExecuteGates();
  return id;
}

IScheduler::WireId<IScheduler::Boolean> LazyScheduler::privateXorPrivate(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  auto id =
      gateKeeper_->normalGate(INormalGate::GateType::SymmetricXOR, left, right);
  maybeExecuteGates();
  return id;
}

IScheduler::WireId<IScheduler::Boolean> LazyScheduler::privateXorPrivateBatch(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  auto id = gateKeeper_->normalGateBatch(
      INormalGate::GateType::SymmetricXOR, left, right);
  maybeExecuteGates();
  return id;
}

IScheduler::WireId<IScheduler::Boolean> LazyScheduler::privateXorPublic(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  auto id = gateKeeper_->normalGate(
      INormalGate::GateType::AsymmetricXOR, left, right);
  maybeExecuteGates();
  return id;
}

IScheduler::WireId<IScheduler::Boolean> LazyScheduler::privateXorPublicBatch(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  auto id = gateKeeper_->normalGateBatch(
      INormalGate::GateType::AsymmetricXOR, left, right);
  maybeExecuteGates();
  return id;
}

IScheduler::WireId<IScheduler::Boolean> LazyScheduler::publicXorPublic(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  auto id =
      gateKeeper_->normalGate(INormalGate::GateType::SymmetricXOR, left, right);
  maybeExecuteGates();
  return id;
}

IScheduler::WireId<IScheduler::Boolean> LazyScheduler::publicXorPublicBatch(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  auto id = gateKeeper_->normalGateBatch(
      INormalGate::GateType::SymmetricXOR, left, right);
  maybeExecuteGates();
  return id;
}

IScheduler::WireId<IScheduler::Boolean> LazyScheduler::notPrivate(
    WireId<IScheduler::Boolean> src) {
  auto id = gateKeeper_->normalGate(INormalGate::GateType::AsymmetricNot, src);
  maybeExecuteGates();
  return id;
}

IScheduler::WireId<IScheduler::Boolean> LazyScheduler::notPrivateBatch(
    WireId<IScheduler::Boolean> src) {
  auto id =
      gateKeeper_->normalGateBatch(INormalGate::GateType::AsymmetricNot, src);
  maybeExecuteGates();
  return id;
}

IScheduler::WireId<IScheduler::Boolean> LazyScheduler::notPublic(
    WireId<IScheduler::Boolean> src) {
  auto id = gateKeeper_->normalGate(INormalGate::GateType::SymmetricNot, src);
  maybeExecuteGates();
  return id;
}

IScheduler::WireId<IScheduler::Boolean> LazyScheduler::notPublicBatch(
    WireId<IScheduler::Boolean> src) {
  auto id =
      gateKeeper_->normalGateBatch(INormalGate::GateType::SymmetricNot, src);
  maybeExecuteGates();
  return id;
}

void LazyScheduler::increaseReferenceCount(WireId<IScheduler::Boolean> id) {
  wireKeeper_->increaseReferenceCount(id);
}

void LazyScheduler::increaseReferenceCountBatch(
    WireId<IScheduler::Boolean> id) {
  wireKeeper_->increaseBatchReferenceCount(id);
}

void LazyScheduler::decreaseReferenceCount(WireId<IScheduler::Boolean> id) {
  wireKeeper_->decreaseReferenceCount(id);
}

void LazyScheduler::decreaseReferenceCountBatch(
    WireId<IScheduler::Boolean> id) {
  wireKeeper_->decreaseBatchReferenceCount(id);
}

std::pair<uint64_t, uint64_t> LazyScheduler::getTrafficStatistics() const {
  return engine_->getTrafficStatistics();
}

// band a number of batches into one batch.
IScheduler::WireId<IScheduler::Boolean> LazyScheduler::batchingUp(
    std::vector<WireId<Boolean>> src) {
  maybeExecuteGates();
  return gateKeeper_->batchingUp(src);
}

// decompose a batch of values into several smaller batches.
std::vector<IScheduler::WireId<IScheduler::Boolean>> LazyScheduler::unbatching(
    WireId<Boolean> src,
    std::shared_ptr<std::vector<uint32_t>> unbatchingStrategy) {
  auto rst = gateKeeper_->unbatching(src, unbatchingStrategy);
  while (gateKeeper_->hasReachedBatchingLimit()) {
    executeOneLevel();
  }
  return rst;
}

template <bool usingBatch>
IGateKeeper::BoolType<usingBatch> LazyScheduler::forceWire(
    IScheduler::WireId<IScheduler::Boolean> id) {
  if constexpr (usingBatch) {
    executeTillLevel(wireKeeper_->getBatchFirstAvailableLevel(id));
    return wireKeeper_->getBatchBooleanValue(id);
  } else {
    executeTillLevel(wireKeeper_->getFirstAvailableLevel(id));
    return wireKeeper_->getBooleanValue(id);
  }
}

void LazyScheduler::maybeExecuteGates() {
  while (gateKeeper_->hasReachedBatchingLimit()) {
    executeOneLevel();
  }
}

void LazyScheduler::executeTillLevel(uint32_t level) {
  while (gateKeeper_->getFirstUnexecutedLevel() <= level) {
    executeOneLevel();
  }
}

void LazyScheduler::executeOneLevel() {
  auto level = gateKeeper_->getFirstUnexecutedLevel();
  auto gates = gateKeeper_->popFirstUnexecutedLevel();
  auto isLevelFree = IGateKeeper::isLevelFree(level);

  // Compute free or non-free gates
  std::map<int64_t, IGate::Secrets> secretSharesByParty;
  for (auto& gate : gates) {
    gate->compute(*engine_, secretSharesByParty);

    if (isLevelFree) {
      freeGates_ += gate->getNumberOfResults();
    } else {
      nonFreeGates_ += gate->getNumberOfResults();
    }
  }

  if (!isLevelFree) {
    // Execute AND gates and share secrets
    engine_->executeScheduledAND();

    std::map<int64_t, IGate::Secrets> revealedSecretsByParty;
    for (auto [party, secretShares] : secretSharesByParty) {
      revealedSecretsByParty.emplace(
          party,
          IGate::Secrets(
              engine_->revealToParty(party, secretShares.booleanSecrets),
              std::vector<uint64_t>()));
    }

    // Update non-free gates
    for (auto& nonFreeGate : gates) {
      nonFreeGate->collectScheduledResult(*engine_, revealedSecretsByParty);
    }
  }
}

} // namespace fbpcf::scheduler
