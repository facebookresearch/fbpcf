/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/scheduler/EagerScheduler.h"
#include <stdexcept>
#include <string>

namespace fbpcf::scheduler {

EagerScheduler::EagerScheduler(
    std::unique_ptr<engine::ISecretShareEngine> engine,
    std::unique_ptr<IWireKeeper> wireKeeper)
    : engine_{std::move(engine)}, wireKeeper_{std::move(wireKeeper)} {}

IScheduler::WireId<IScheduler::Boolean> EagerScheduler::privateBooleanInput(
    bool v,
    int partyId) {
  freeGates_++;
  return wireKeeper_->allocateBooleanValue(engine_->setInput(partyId, v));
}

IScheduler::WireId<IScheduler::Boolean>
EagerScheduler::privateBooleanInputBatch(
    const std::vector<bool>& v,
    int partyId) {
  freeGates_ += v.size();
  return wireKeeper_->allocateBatchBooleanValue(
      engine_->setBatchInput(partyId, v));
}

IScheduler::WireId<IScheduler::Boolean> EagerScheduler::publicBooleanInput(
    bool v) {
  freeGates_++;
  return wireKeeper_->allocateBooleanValue(v);
}

IScheduler::WireId<IScheduler::Boolean> EagerScheduler::publicBooleanInputBatch(
    const std::vector<bool>& v) {
  freeGates_ += v.size();
  return wireKeeper_->allocateBatchBooleanValue(v);
}

IScheduler::WireId<IScheduler::Boolean> EagerScheduler::recoverBooleanWire(
    bool v) {
  freeGates_++;
  return wireKeeper_->allocateBooleanValue(v);
}

IScheduler::WireId<IScheduler::Boolean> EagerScheduler::recoverBooleanWireBatch(
    const std::vector<bool>& v) {
  freeGates_ += v.size();
  return wireKeeper_->allocateBatchBooleanValue(v);
}

IScheduler::WireId<IScheduler::Boolean> EagerScheduler::openBooleanValueToParty(
    WireId<IScheduler::Boolean> src,
    int partyId) {
  std::vector<bool> secretShares{wireKeeper_->getBooleanValue(src)};
  nonFreeGates_ += secretShares.size();
  auto revealedSecrets = engine_->revealToParty(partyId, secretShares);

  if (revealedSecrets.size() == 1) {
    return wireKeeper_->allocateBooleanValue(revealedSecrets.at(0));
  } else {
    throw std::runtime_error("Unexpected number of revealed secrets");
  }
}

IScheduler::WireId<IScheduler::Boolean>
EagerScheduler::openBooleanValueToPartyBatch(
    WireId<IScheduler::Boolean> src,
    int partyId) {
  auto secretShares = wireKeeper_->getBatchBooleanValue(src);
  nonFreeGates_ += secretShares.size();
  auto revealedSecrets = engine_->revealToParty(partyId, secretShares);

  if (revealedSecrets.size() == secretShares.size()) {
    return wireKeeper_->allocateBatchBooleanValue(revealedSecrets);
  } else {
    throw std::runtime_error(
        "Unexpected number of revealed secrets " +
        std::to_string(revealedSecrets.size()));
  }
}

bool EagerScheduler::extractBooleanSecretShare(WireId<IScheduler::Boolean> id) {
  return wireKeeper_->getBooleanValue(id);
}

std::vector<bool> EagerScheduler::extractBooleanSecretShareBatch(
    WireId<IScheduler::Boolean> id) {
  return wireKeeper_->getBatchBooleanValue(id);
}

bool EagerScheduler::getBooleanValue(WireId<IScheduler::Boolean> id) {
  return wireKeeper_->getBooleanValue(id);
}

std::vector<bool> EagerScheduler::getBooleanValueBatch(
    WireId<IScheduler::Boolean> id) {
  return wireKeeper_->getBatchBooleanValue(id);
}

IScheduler::WireId<IScheduler::Boolean> EagerScheduler::privateAndPrivate(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  nonFreeGates_++;
  auto index = engine_->scheduleAND(
      wireKeeper_->getBooleanValue(left), wireKeeper_->getBooleanValue(right));
  engine_->executeScheduledAND();
  return wireKeeper_->allocateBooleanValue(
      engine_->getANDExecutionResult(index));
}

IScheduler::WireId<IScheduler::Boolean> EagerScheduler::privateAndPrivateBatch(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  auto leftValue = wireKeeper_->getBatchBooleanValue(left);
  auto rightValue = wireKeeper_->getBatchBooleanValue(right);
  nonFreeGates_ += leftValue.size();
  return wireKeeper_->allocateBatchBooleanValue(
      engine_->computeBatchAND(leftValue, rightValue));
}

IScheduler::WireId<IScheduler::Boolean> EagerScheduler::privateAndPublic(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  freeGates_++;
  return wireKeeper_->allocateBooleanValue(engine_->computeFreeAND(
      wireKeeper_->getBooleanValue(left), wireKeeper_->getBooleanValue(right)));
}

IScheduler::WireId<IScheduler::Boolean> EagerScheduler::privateAndPublicBatch(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  auto leftValue = wireKeeper_->getBatchBooleanValue(left);
  auto rightValue = wireKeeper_->getBatchBooleanValue(right);
  freeGates_ += leftValue.size();
  return wireKeeper_->allocateBatchBooleanValue(
      engine_->computeBatchFreeAND(leftValue, rightValue));
}

IScheduler::WireId<IScheduler::Boolean> EagerScheduler::publicAndPublic(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  freeGates_++;
  return wireKeeper_->allocateBooleanValue(engine_->computeFreeAND(
      wireKeeper_->getBooleanValue(left), wireKeeper_->getBooleanValue(right)));
}

IScheduler::WireId<IScheduler::Boolean> EagerScheduler::publicAndPublicBatch(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  auto leftValue = wireKeeper_->getBatchBooleanValue(left);
  auto rightValue = wireKeeper_->getBatchBooleanValue(right);
  freeGates_ += leftValue.size();
  return wireKeeper_->allocateBatchBooleanValue(
      engine_->computeBatchFreeAND(leftValue, rightValue));
}

std::vector<IScheduler::WireId<IScheduler::Boolean>>
EagerScheduler::privateAndPrivateComposite(
    IScheduler::WireId<IScheduler::Boolean> left,
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) {
  throw std::runtime_error("Not implemented");
}

std::vector<IScheduler::WireId<IScheduler::Boolean>>
EagerScheduler::privateAndPrivateCompositeBatch(
    IScheduler::WireId<IScheduler::Boolean> left,
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) {
  throw std::runtime_error("Not implemented");
}

std::vector<IScheduler::WireId<IScheduler::Boolean>>
EagerScheduler::privateAndPublicComposite(
    IScheduler::WireId<IScheduler::Boolean> left,
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) {
  throw std::runtime_error("Not implemented");
}

std::vector<IScheduler::WireId<IScheduler::Boolean>>
EagerScheduler::privateAndPublicCompositeBatch(
    IScheduler::WireId<IScheduler::Boolean> left,
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) {
  throw std::runtime_error("Not implemented");
}

std::vector<IScheduler::WireId<IScheduler::Boolean>>
EagerScheduler::publicAndPublicComposite(
    IScheduler::WireId<IScheduler::Boolean> left,
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) {
  throw std::runtime_error("Not implemented");
}

std::vector<IScheduler::WireId<IScheduler::Boolean>>
EagerScheduler::publicAndPublicCompositeBatch(
    IScheduler::WireId<IScheduler::Boolean> left,
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) {
  throw std::runtime_error("Not implemented");
}

IScheduler::WireId<IScheduler::Boolean> EagerScheduler::privateXorPrivate(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  freeGates_++;
  return wireKeeper_->allocateBooleanValue(engine_->computeSymmetricXOR(
      wireKeeper_->getBooleanValue(left), wireKeeper_->getBooleanValue(right)));
}

IScheduler::WireId<IScheduler::Boolean> EagerScheduler::privateXorPrivateBatch(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  auto leftValue = wireKeeper_->getBatchBooleanValue(left);
  auto rightValue = wireKeeper_->getBatchBooleanValue(right);
  freeGates_ += leftValue.size();
  return wireKeeper_->allocateBatchBooleanValue(
      engine_->computeBatchSymmetricXOR(leftValue, rightValue));
}

IScheduler::WireId<IScheduler::Boolean> EagerScheduler::privateXorPublic(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  freeGates_++;
  return wireKeeper_->allocateBooleanValue(engine_->computeAsymmetricXOR(
      wireKeeper_->getBooleanValue(left), wireKeeper_->getBooleanValue(right)));
}

IScheduler::WireId<IScheduler::Boolean> EagerScheduler::privateXorPublicBatch(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  auto leftValue = wireKeeper_->getBatchBooleanValue(left);
  auto rightValue = wireKeeper_->getBatchBooleanValue(right);
  freeGates_ += leftValue.size();
  return wireKeeper_->allocateBatchBooleanValue(
      engine_->computeBatchAsymmetricXOR(leftValue, rightValue));
}

IScheduler::WireId<IScheduler::Boolean> EagerScheduler::publicXorPublic(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  freeGates_++;
  return wireKeeper_->allocateBooleanValue(engine_->computeSymmetricXOR(
      wireKeeper_->getBooleanValue(left), wireKeeper_->getBooleanValue(right)));
}

IScheduler::WireId<IScheduler::Boolean> EagerScheduler::publicXorPublicBatch(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  auto leftValue = wireKeeper_->getBatchBooleanValue(left);
  auto rightValue = wireKeeper_->getBatchBooleanValue(right);
  freeGates_ += leftValue.size();
  return wireKeeper_->allocateBatchBooleanValue(
      engine_->computeBatchSymmetricXOR(leftValue, rightValue));
}

IScheduler::WireId<IScheduler::Boolean> EagerScheduler::notPrivate(
    WireId<IScheduler::Boolean> src) {
  freeGates_++;
  return wireKeeper_->allocateBooleanValue(
      engine_->computeAsymmetricNOT(wireKeeper_->getBooleanValue(src)));
}

IScheduler::WireId<IScheduler::Boolean> EagerScheduler::notPrivateBatch(
    WireId<IScheduler::Boolean> src) {
  auto values = wireKeeper_->getBatchBooleanValue(src);
  freeGates_ += values.size();
  return wireKeeper_->allocateBatchBooleanValue(
      engine_->computeBatchAsymmetricNOT(values));
}

IScheduler::WireId<IScheduler::Boolean> EagerScheduler::notPublic(
    WireId<IScheduler::Boolean> src) {
  freeGates_++;
  return wireKeeper_->allocateBooleanValue(
      engine_->computeSymmetricNOT(wireKeeper_->getBooleanValue(src)));
}

IScheduler::WireId<IScheduler::Boolean> EagerScheduler::notPublicBatch(
    WireId<IScheduler::Boolean> src) {
  auto values = wireKeeper_->getBatchBooleanValue(src);
  freeGates_ += values.size();
  return wireKeeper_->allocateBatchBooleanValue(
      engine_->computeBatchSymmetricNOT(values));
}

void EagerScheduler::increaseReferenceCount(WireId<IScheduler::Boolean> id) {
  wireKeeper_->increaseReferenceCount(id);
}

void EagerScheduler::increaseReferenceCountBatch(
    WireId<IScheduler::Boolean> id) {
  wireKeeper_->increaseBatchReferenceCount(id);
}

void EagerScheduler::decreaseReferenceCount(WireId<IScheduler::Boolean> id) {
  wireKeeper_->decreaseReferenceCount(id);
}

void EagerScheduler::decreaseReferenceCountBatch(
    WireId<IScheduler::Boolean> id) {
  wireKeeper_->decreaseBatchReferenceCount(id);
}

std::pair<uint64_t, uint64_t> EagerScheduler::getTrafficStatistics() const {
  return engine_->getTrafficStatistics();
}

} // namespace fbpcf::scheduler
