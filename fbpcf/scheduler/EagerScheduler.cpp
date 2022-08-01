/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/scheduler/EagerScheduler.h"
#include <stdexcept>
#include <string>
#include "fbpcf/util/MetricCollector.h"

namespace fbpcf::scheduler {

EagerScheduler::EagerScheduler(
    std::unique_ptr<engine::ISecretShareEngine> engine,
    std::unique_ptr<IWireKeeper> wireKeeper,
    std::shared_ptr<util::MetricCollector> collector)
    : engine_{std::move(engine)},
      wireKeeper_{std::move(wireKeeper)},
      collector_{collector} {}

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
      engine_->computeBatchANDImmediately(leftValue, rightValue));
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
  nonFreeGates_ += rights.size();
  std::vector<bool> rightValues(rights.size());
  for (size_t i = 0; i < rights.size(); i++) {
    rightValues[i] = wireKeeper_->getBooleanValue(rights[i]);
  }
  auto index = engine_->scheduleCompositeAND(
      wireKeeper_->getBooleanValue(left), rightValues);
  engine_->executeScheduledAND();
  auto result = engine_->getCompositeANDExecutionResult(index);
  std::vector<IScheduler::WireId<IScheduler::Boolean>> outputWires(
      result.size());
  for (size_t i = 0; i < rights.size(); i++) {
    outputWires[i] = wireKeeper_->allocateBooleanValue(result[i]);
  }
  return outputWires;
}

std::vector<IScheduler::WireId<IScheduler::Boolean>>
EagerScheduler::privateAndPrivateCompositeBatch(
    IScheduler::WireId<IScheduler::Boolean> left,
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) {
  auto leftValues = wireKeeper_->getBatchBooleanValue(left);
  nonFreeGates_ += leftValues.size() * rights.size();
  std::vector<std::vector<bool>> rightValues;
  for (size_t i = 0; i < rights.size(); i++) {
    rightValues.push_back(wireKeeper_->getBatchBooleanValue(rights[i]));
  }

  auto index = engine_->scheduleBatchCompositeAND(leftValues, rightValues);
  engine_->executeScheduledAND();
  auto result = engine_->getBatchCompositeANDExecutionResult(index);
  std::vector<IScheduler::WireId<IScheduler::Boolean>> outputWires(
      result.size());
  for (size_t i = 0; i < result.size(); i++) {
    outputWires[i] = wireKeeper_->allocateBatchBooleanValue(result[i]);
  }
  return outputWires;
}

std::vector<IScheduler::WireId<IScheduler::Boolean>>
EagerScheduler::privateAndPublicComposite(
    IScheduler::WireId<IScheduler::Boolean> left,
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) {
  freeGates_ += rights.size();
  std::vector<IScheduler::WireId<IScheduler::Boolean>> outputWires(
      rights.size());
  for (size_t i = 0; i < rights.size(); i++) {
    outputWires[i] = wireKeeper_->allocateBooleanValue(engine_->computeFreeAND(
        wireKeeper_->getBooleanValue(left),
        wireKeeper_->getBooleanValue(rights[i])));
  }
  return outputWires;
}

std::vector<IScheduler::WireId<IScheduler::Boolean>>
EagerScheduler::privateAndPublicCompositeBatch(
    IScheduler::WireId<IScheduler::Boolean> left,
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) {
  auto leftValues = wireKeeper_->getBatchBooleanValue(left);
  freeGates_ += leftValues.size() * rights.size();
  std::vector<IScheduler::WireId<IScheduler::Boolean>> outputWires(
      rights.size());
  for (size_t i = 0; i < rights.size(); i++) {
    outputWires[i] =
        wireKeeper_->allocateBatchBooleanValue((engine_->computeBatchFreeAND(
            leftValues, wireKeeper_->getBatchBooleanValue(rights[i]))));
  }
  return outputWires;
}

std::vector<IScheduler::WireId<IScheduler::Boolean>>
EagerScheduler::publicAndPublicComposite(
    IScheduler::WireId<IScheduler::Boolean> left,
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) {
  return privateAndPublicComposite(left, rights);
}

std::vector<IScheduler::WireId<IScheduler::Boolean>>
EagerScheduler::publicAndPublicCompositeBatch(
    IScheduler::WireId<IScheduler::Boolean> left,
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) {
  return privateAndPublicCompositeBatch(left, rights);
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

// band a number of batches into one batch.
IScheduler::WireId<IScheduler::Boolean> EagerScheduler::batchingUp(
    std::vector<WireId<Boolean>> src) {
  size_t batchSize = 0;
  for (auto& item : src) {
    batchSize += wireKeeper_->getBatchBooleanValue(item).size();
  }
  std::vector<bool> vector(batchSize, 0);
  size_t index = 0;
  for (auto& item : src) {
    auto& batch = wireKeeper_->getBatchBooleanValue(item);
    for (size_t i = 0; i < batch.size(); i++) {
      vector[index++] = batch.at(i);
    }
  }
  return wireKeeper_->allocateBatchBooleanValue(vector);
}

// decompose a batch of values into several smaller batches.
std::vector<IScheduler::WireId<IScheduler::Boolean>> EagerScheduler::unbatching(
    WireId<Boolean> src,
    std::shared_ptr<std::vector<uint32_t>> unbatchingStrategy) {
  auto batch = wireKeeper_->getBatchBooleanValue(src);
  size_t index = 0;
  std::vector<IScheduler::WireId<IScheduler::Boolean>> rst(
      unbatchingStrategy->size());
  for (size_t i = 0; i < rst.size(); i++) {
    std::vector<bool> v(unbatchingStrategy->at(i));
    if (index + v.size() > batch.size()) {
      throw std::runtime_error(
          "Failed to unbatch, you are unbatching to more values than the input has.");
    }
    for (size_t j = 0; j < v.size(); j++) {
      v[j] = batch.at(index++);
    }
    rst[i] = wireKeeper_->allocateBatchBooleanValue(v);
  }
  return rst;
}

std::pair<uint64_t, uint64_t> EagerScheduler::getTrafficStatistics() const {
  return engine_->getTrafficStatistics();
}

} // namespace fbpcf::scheduler
