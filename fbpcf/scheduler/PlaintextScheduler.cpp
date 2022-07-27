/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cstddef>
#include <stdexcept>
#include <vector>

#include <fbpcf/scheduler/IScheduler.h>
#include "fbpcf/scheduler/PlaintextScheduler.h"

namespace fbpcf::scheduler {

PlaintextScheduler::PlaintextScheduler(std::unique_ptr<IWireKeeper> wireKeeper)
    : wireKeeper_{std::move(wireKeeper)} {}

IScheduler::WireId<IScheduler::Boolean> PlaintextScheduler::privateBooleanInput(
    bool v,
    int /*partyId*/) {
  freeGates_++;
  return wireKeeper_->allocateBooleanValue(v);
}

IScheduler::WireId<IScheduler::Boolean>
PlaintextScheduler::privateBooleanInputBatch(
    const std::vector<bool>& v,
    int /*partyId*/) {
  freeGates_ += v.size();
  return wireKeeper_->allocateBatchBooleanValue(v);
}

IScheduler::WireId<IScheduler::Boolean> PlaintextScheduler::publicBooleanInput(
    bool v) {
  freeGates_++;
  return wireKeeper_->allocateBooleanValue(v);
}

IScheduler::WireId<IScheduler::Boolean>
PlaintextScheduler::publicBooleanInputBatch(const std::vector<bool>& v) {
  freeGates_ += v.size();
  return wireKeeper_->allocateBatchBooleanValue(v);
}

IScheduler::WireId<IScheduler::Boolean> PlaintextScheduler::recoverBooleanWire(
    bool v) {
  freeGates_++;
  return wireKeeper_->allocateBooleanValue(v);
}

IScheduler::WireId<IScheduler::Boolean>
PlaintextScheduler::recoverBooleanWireBatch(const std::vector<bool>& v) {
  freeGates_ += v.size();
  return wireKeeper_->allocateBatchBooleanValue(v);
}

IScheduler::WireId<IScheduler::Boolean>
PlaintextScheduler::openBooleanValueToParty(
    WireId<IScheduler::Boolean> src,
    int /*partyId*/) {
  nonFreeGates_++;
  return wireKeeper_->allocateBooleanValue(wireKeeper_->getBooleanValue(src));
}

IScheduler::WireId<IScheduler::Boolean>
PlaintextScheduler::openBooleanValueToPartyBatch(
    WireId<IScheduler::Boolean> src,
    int /*partyId*/) {
  auto& v = wireKeeper_->getBatchBooleanValue(src);
  nonFreeGates_ += v.size();
  return wireKeeper_->allocateBatchBooleanValue(v);
}

bool PlaintextScheduler::extractBooleanSecretShare(
    WireId<IScheduler::Boolean> id) {
  return wireKeeper_->getBooleanValue(id);
}

std::vector<bool> PlaintextScheduler::extractBooleanSecretShareBatch(
    WireId<IScheduler::Boolean> id) {
  return wireKeeper_->getBatchBooleanValue(id);
}

bool PlaintextScheduler::getBooleanValue(WireId<IScheduler::Boolean> id) {
  return wireKeeper_->getBooleanValue(id);
}

std::vector<bool> PlaintextScheduler::getBooleanValueBatch(
    WireId<IScheduler::Boolean> id) {
  return wireKeeper_->getBatchBooleanValue(id);
}

IScheduler::WireId<IScheduler::Arithmetic>
PlaintextScheduler::privateIntegerInput(uint64_t v, int /*partyId*/) {
  freeGates_++;
  return wireKeeper_->allocateIntegerValue(v);
}

IScheduler::WireId<IScheduler::Arithmetic>
PlaintextScheduler::privateIntegerInputBatch(
    const std::vector<uint64_t>& v,
    int /*partyId*/) {
  freeGates_ += v.size();
  return wireKeeper_->allocateBatchIntegerValue(v);
}

IScheduler::WireId<IScheduler::Arithmetic>
PlaintextScheduler::publicIntegerInput(uint64_t v) {
  freeGates_++;
  return wireKeeper_->allocateIntegerValue(v);
}

IScheduler::WireId<IScheduler::Arithmetic>
PlaintextScheduler::publicIntegerInputBatch(const std::vector<uint64_t>& v) {
  freeGates_ += v.size();
  return wireKeeper_->allocateBatchIntegerValue(v);
}

IScheduler::WireId<IScheduler::Arithmetic>
PlaintextScheduler::recoverIntegerWire(uint64_t v) {
  freeGates_++;
  return wireKeeper_->allocateIntegerValue(v);
}

IScheduler::WireId<IScheduler::Arithmetic>
PlaintextScheduler::recoverIntegerWireBatch(const std::vector<uint64_t>& v) {
  freeGates_ += v.size();
  return wireKeeper_->allocateBatchIntegerValue(v);
}

IScheduler::WireId<IScheduler::Arithmetic>
PlaintextScheduler::openIntegerValueToParty(
    WireId<IScheduler::Arithmetic> src,
    int /*partyId*/) {
  nonFreeGates_++;
  return wireKeeper_->allocateIntegerValue(wireKeeper_->getIntegerValue(src));
}

IScheduler::WireId<IScheduler::Arithmetic>
PlaintextScheduler::openIntegerValueToPartyBatch(
    WireId<IScheduler::Arithmetic> src,
    int /*partyId*/) {
  auto& v = wireKeeper_->getBatchIntegerValue(src);
  nonFreeGates_ += v.size();
  return wireKeeper_->allocateBatchIntegerValue(v);
}

uint64_t PlaintextScheduler::extractIntegerSecretShare(
    WireId<IScheduler::Arithmetic> id) {
  return wireKeeper_->getIntegerValue(id);
}

std::vector<uint64_t> PlaintextScheduler::extractIntegerSecretShareBatch(
    WireId<IScheduler::Arithmetic> id) {
  return wireKeeper_->getBatchIntegerValue(id);
}

uint64_t PlaintextScheduler::getIntegerValue(
    WireId<IScheduler::Arithmetic> id) {
  return wireKeeper_->getIntegerValue(id);
}

std::vector<uint64_t> PlaintextScheduler::getIntegerValueBatch(
    WireId<IScheduler::Arithmetic> id) {
  return wireKeeper_->getBatchIntegerValue(id);
}

IScheduler::WireId<IScheduler::Boolean> PlaintextScheduler::privateAndPrivate(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  nonFreeGates_++;
  return wireKeeper_->allocateBooleanValue(
      wireKeeper_->getBooleanValue(left) & wireKeeper_->getBooleanValue(right));
}

IScheduler::WireId<IScheduler::Boolean>
PlaintextScheduler::privateAndPrivateBatch(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  auto& leftValue = wireKeeper_->getBatchBooleanValue(left);
  auto& rightValue = wireKeeper_->getBatchBooleanValue(right);
  if (leftValue.size() != rightValue.size()) {
    throw std::invalid_argument("invalid inputs!");
  }
  nonFreeGates_ += leftValue.size();
  std::vector<bool> rst(leftValue.size());
  for (size_t i = 0; i < leftValue.size(); i++) {
    rst[i] = leftValue[i] & rightValue[i];
  }
  return wireKeeper_->allocateBatchBooleanValue(rst);
}

IScheduler::WireId<IScheduler::Boolean> PlaintextScheduler::privateAndPublic(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  freeGates_++;
  return wireKeeper_->allocateBooleanValue(
      wireKeeper_->getBooleanValue(left) & wireKeeper_->getBooleanValue(right));
}

IScheduler::WireId<IScheduler::Boolean>
PlaintextScheduler::privateAndPublicBatch(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  auto& leftValue = wireKeeper_->getBatchBooleanValue(left);
  auto& rightValue = wireKeeper_->getBatchBooleanValue(right);
  if (leftValue.size() != rightValue.size()) {
    throw std::invalid_argument("invalid inputs!");
  }
  freeGates_ += leftValue.size();
  std::vector<bool> rst(leftValue.size());
  for (size_t i = 0; i < leftValue.size(); i++) {
    rst[i] = leftValue.at(i) & rightValue.at(i);
  }
  return wireKeeper_->allocateBatchBooleanValue(rst);
}

IScheduler::WireId<IScheduler::Boolean> PlaintextScheduler::publicAndPublic(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  return privateAndPublic(left, right);
}

IScheduler::WireId<IScheduler::Boolean>
PlaintextScheduler::publicAndPublicBatch(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  return privateAndPublicBatch(left, right);
}

std::vector<IScheduler::WireId<IScheduler::Boolean>>
PlaintextScheduler::privateAndPrivateComposite(
    IScheduler::WireId<IScheduler::Boolean> left,
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) {
  return computeCompositeAND(left, rights, nonFreeGates_);
}

std::vector<IScheduler::WireId<IScheduler::Boolean>>
PlaintextScheduler::privateAndPrivateCompositeBatch(
    IScheduler::WireId<IScheduler::Boolean> left,
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) {
  return validateAndComputeBatchCompositeAND(left, rights, nonFreeGates_);
}

std::vector<IScheduler::WireId<IScheduler::Boolean>>
PlaintextScheduler::privateAndPublicComposite(
    IScheduler::WireId<IScheduler::Boolean> left,
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) {
  return computeCompositeAND(left, rights, freeGates_);
}

std::vector<IScheduler::WireId<IScheduler::Boolean>>
PlaintextScheduler::privateAndPublicCompositeBatch(
    IScheduler::WireId<IScheduler::Boolean> left,
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) {
  return validateAndComputeBatchCompositeAND(left, rights, freeGates_);
}

std::vector<IScheduler::WireId<IScheduler::Boolean>>
PlaintextScheduler::publicAndPublicComposite(
    IScheduler::WireId<IScheduler::Boolean> left,
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) {
  return computeCompositeAND(left, rights, freeGates_);
}

std::vector<IScheduler::WireId<IScheduler::Boolean>>
PlaintextScheduler::publicAndPublicCompositeBatch(
    IScheduler::WireId<IScheduler::Boolean> left,
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rights) {
  return validateAndComputeBatchCompositeAND(left, rights, freeGates_);
}

IScheduler::WireId<IScheduler::Boolean> PlaintextScheduler::privateXorPrivate(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  freeGates_++;
  return wireKeeper_->allocateBooleanValue(
      wireKeeper_->getBooleanValue(left) ^ wireKeeper_->getBooleanValue(right));
}

IScheduler::WireId<IScheduler::Boolean>
PlaintextScheduler::privateXorPrivateBatch(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  auto& leftValue = wireKeeper_->getBatchBooleanValue(left);
  auto& rightValue = wireKeeper_->getBatchBooleanValue(right);
  if (leftValue.size() != rightValue.size()) {
    throw std::invalid_argument("invalid inputs!");
  }
  freeGates_ += leftValue.size();
  std::vector<bool> rst(leftValue.size());
  for (size_t i = 0; i < leftValue.size(); i++) {
    rst[i] = leftValue[i] ^ rightValue[i];
  }
  return wireKeeper_->allocateBatchBooleanValue(rst);
}

IScheduler::WireId<IScheduler::Boolean> PlaintextScheduler::privateXorPublic(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  return privateXorPrivate(left, right);
}

IScheduler::WireId<IScheduler::Boolean>
PlaintextScheduler::privateXorPublicBatch(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  return privateXorPrivateBatch(left, right);
}

IScheduler::WireId<IScheduler::Boolean> PlaintextScheduler::publicXorPublic(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  return privateXorPrivate(left, right);
}

IScheduler::WireId<IScheduler::Boolean>
PlaintextScheduler::publicXorPublicBatch(
    WireId<IScheduler::Boolean> left,
    WireId<IScheduler::Boolean> right) {
  return privateXorPrivateBatch(left, right);
}

IScheduler::WireId<IScheduler::Boolean> PlaintextScheduler::notPrivate(
    WireId<IScheduler::Boolean> src) {
  freeGates_++;
  return wireKeeper_->allocateBooleanValue(!wireKeeper_->getBooleanValue(src));
}

IScheduler::WireId<IScheduler::Boolean> PlaintextScheduler::notPrivateBatch(
    WireId<IScheduler::Boolean> src) {
  auto& value = wireKeeper_->getBatchBooleanValue(src);
  freeGates_ += value.size();
  std::vector<bool> rst(value.size());
  for (size_t i = 0; i < value.size(); i++) {
    rst[i] = !value[i];
  }
  return wireKeeper_->allocateBatchBooleanValue(rst);
}

IScheduler::WireId<IScheduler::Boolean> PlaintextScheduler::notPublic(
    WireId<IScheduler::Boolean> src) {
  return notPrivate(src);
}

IScheduler::WireId<IScheduler::Boolean> PlaintextScheduler::notPublicBatch(
    WireId<IScheduler::Boolean> src) {
  return notPrivateBatch(src);
}

IScheduler::WireId<IScheduler::Arithmetic>
PlaintextScheduler::privatePlusPrivate(
    WireId<IScheduler::Arithmetic> left,
    WireId<IScheduler::Arithmetic> right) {
  freeGates_++;
  return wireKeeper_->allocateIntegerValue(
      wireKeeper_->getIntegerValue(left) + wireKeeper_->getIntegerValue(right));
}

IScheduler::WireId<IScheduler::Arithmetic>
PlaintextScheduler::privatePlusPrivateBatch(
    WireId<IScheduler::Arithmetic> left,
    WireId<IScheduler::Arithmetic> right) {
  auto& leftValue = wireKeeper_->getBatchIntegerValue(left);
  auto& rightValue = wireKeeper_->getBatchIntegerValue(right);
  if (leftValue.size() != rightValue.size()) {
    throw std::invalid_argument("invalid inputs!");
  }
  freeGates_ += leftValue.size();
  std::vector<uint64_t> rst(leftValue.size());
  for (size_t i = 0; i < leftValue.size(); i++) {
    rst[i] = leftValue[i] + rightValue[i];
  }
  return wireKeeper_->allocateBatchIntegerValue(rst);
}

IScheduler::WireId<IScheduler::Arithmetic>
PlaintextScheduler::privatePlusPublic(
    WireId<IScheduler::Arithmetic> left,
    WireId<IScheduler::Arithmetic> right) {
  return privatePlusPrivate(left, right);
}

IScheduler::WireId<IScheduler::Arithmetic>
PlaintextScheduler::privatePlusPublicBatch(
    WireId<IScheduler::Arithmetic> left,
    WireId<IScheduler::Arithmetic> right) {
  return privatePlusPrivateBatch(left, right);
}

IScheduler::WireId<IScheduler::Arithmetic> PlaintextScheduler::publicPlusPublic(
    WireId<IScheduler::Arithmetic> left,
    WireId<IScheduler::Arithmetic> right) {
  return privatePlusPrivate(left, right);
}

IScheduler::WireId<IScheduler::Arithmetic>
PlaintextScheduler::publicPlusPublicBatch(
    WireId<IScheduler::Arithmetic> left,
    WireId<IScheduler::Arithmetic> right) {
  return privatePlusPrivateBatch(left, right);
}

IScheduler::WireId<IScheduler::Arithmetic>
PlaintextScheduler::privateMultPrivate(
    WireId<IScheduler::Arithmetic> left,
    WireId<IScheduler::Arithmetic> right) {
  nonFreeGates_++;
  return wireKeeper_->allocateIntegerValue(
      wireKeeper_->getIntegerValue(left) * wireKeeper_->getIntegerValue(right));
}

IScheduler::WireId<IScheduler::Arithmetic>
PlaintextScheduler::privateMultPrivateBatch(
    WireId<IScheduler::Arithmetic> left,
    WireId<IScheduler::Arithmetic> right) {
  auto& leftValue = wireKeeper_->getBatchIntegerValue(left);
  auto& rightValue = wireKeeper_->getBatchIntegerValue(right);
  if (leftValue.size() != rightValue.size()) {
    throw std::invalid_argument("invalid inputs!");
  }
  nonFreeGates_ += leftValue.size();
  std::vector<uint64_t> rst(leftValue.size());
  for (size_t i = 0; i < leftValue.size(); i++) {
    rst[i] = leftValue[i] * rightValue[i];
  }
  return wireKeeper_->allocateBatchIntegerValue(rst);
}

IScheduler::WireId<IScheduler::Arithmetic>
PlaintextScheduler::privateMultPublic(
    WireId<IScheduler::Arithmetic> left,
    WireId<IScheduler::Arithmetic> right) {
  freeGates_++;
  return wireKeeper_->allocateIntegerValue(
      wireKeeper_->getIntegerValue(left) * wireKeeper_->getIntegerValue(right));
}

IScheduler::WireId<IScheduler::Arithmetic>
PlaintextScheduler::privateMultPublicBatch(
    WireId<IScheduler::Arithmetic> left,
    WireId<IScheduler::Arithmetic> right) {
  auto& leftValue = wireKeeper_->getBatchIntegerValue(left);
  auto& rightValue = wireKeeper_->getBatchIntegerValue(right);
  if (leftValue.size() != rightValue.size()) {
    throw std::invalid_argument("invalid inputs!");
  }
  freeGates_ += leftValue.size();
  std::vector<uint64_t> rst(leftValue.size());
  for (size_t i = 0; i < leftValue.size(); i++) {
    rst[i] = leftValue[i] * rightValue[i];
  }
  return wireKeeper_->allocateBatchIntegerValue(rst);
}

IScheduler::WireId<IScheduler::Arithmetic> PlaintextScheduler::publicMultPublic(
    WireId<IScheduler::Arithmetic> left,
    WireId<IScheduler::Arithmetic> right) {
  return privateMultPublic(left, right);
}

IScheduler::WireId<IScheduler::Arithmetic>
PlaintextScheduler::publicMultPublicBatch(
    WireId<IScheduler::Arithmetic> left,
    WireId<IScheduler::Arithmetic> right) {
  return privateMultPublicBatch(left, right);
}

IScheduler::WireId<IScheduler::Arithmetic> PlaintextScheduler::negPrivate(
    WireId<IScheduler::Arithmetic> src) {
  freeGates_++;
  return wireKeeper_->allocateIntegerValue(-wireKeeper_->getIntegerValue(src));
}

IScheduler::WireId<IScheduler::Arithmetic> PlaintextScheduler::negPrivateBatch(
    WireId<IScheduler::Arithmetic> src) {
  auto& value = wireKeeper_->getBatchIntegerValue(src);
  freeGates_ += value.size();
  std::vector<uint64_t> rst(value.size());
  for (size_t i = 0; i < value.size(); i++) {
    rst[i] = -value[i];
  }
  return wireKeeper_->allocateBatchIntegerValue(rst);
}

IScheduler::WireId<IScheduler::Arithmetic> PlaintextScheduler::negPublic(
    WireId<IScheduler::Arithmetic> src) {
  return negPrivate(src);
}

IScheduler::WireId<IScheduler::Arithmetic> PlaintextScheduler::negPublicBatch(
    WireId<IScheduler::Arithmetic> src) {
  return negPrivateBatch(src);
}

void PlaintextScheduler::increaseReferenceCount(
    WireId<IScheduler::Boolean> id) {
  wireKeeper_->increaseReferenceCount(id);
}

void PlaintextScheduler::increaseReferenceCountBatch(
    WireId<IScheduler::Boolean> id) {
  wireKeeper_->increaseBatchReferenceCount(id);
}

void PlaintextScheduler::decreaseReferenceCount(
    WireId<IScheduler::Boolean> id) {
  wireKeeper_->decreaseReferenceCount(id);
}

void PlaintextScheduler::decreaseReferenceCountBatch(
    WireId<IScheduler::Boolean> id) {
  wireKeeper_->decreaseBatchReferenceCount(id);
}

void PlaintextScheduler::increaseReferenceCount(
    WireId<IScheduler::Arithmetic> id) {
  wireKeeper_->increaseReferenceCount(id);
}

void PlaintextScheduler::increaseReferenceCountBatch(
    WireId<IScheduler::Arithmetic> id) {
  wireKeeper_->increaseBatchReferenceCount(id);
}

void PlaintextScheduler::decreaseReferenceCount(
    WireId<IScheduler::Arithmetic> id) {
  wireKeeper_->decreaseReferenceCount(id);
}

void PlaintextScheduler::decreaseReferenceCountBatch(
    WireId<IScheduler::Arithmetic> id) {
  wireKeeper_->decreaseBatchReferenceCount(id);
}

std::vector<IScheduler::WireId<IScheduler::Boolean>>
PlaintextScheduler::computeCompositeAND(
    IScheduler::WireId<IScheduler::Boolean> left,
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rights,
    uint64_t& gateCounter) {
  gateCounter += rights.size();
  bool leftValue = wireKeeper_->getBooleanValue(left);
  std::vector<IScheduler::WireId<IScheduler::Boolean>> rst;
  for (auto rightWire : rights) {
    rst.push_back(wireKeeper_->allocateBooleanValue(
        leftValue & wireKeeper_->getBooleanValue(rightWire)));
  }

  return rst;
}

// band a number of batches into one batch.
IScheduler::WireId<IScheduler::Boolean> PlaintextScheduler::batchingUp(
    std::vector<WireId<Boolean>> src) {
  size_t batchSize = 0;
  for (auto& item : src) {
    batchSize += wireKeeper_->getBatchBooleanValue(item).size();
  }
  std::vector<bool> vector(batchSize, 0);
  size_t index = 0;
  for (auto& item : src) {
    auto& batch = wireKeeper_->getBatchBooleanValue(item);
    for (auto v : batch) {
      vector[index++] = v;
    }
  }
  return wireKeeper_->allocateBatchBooleanValue(vector);
}

// decompose a batch of values into several smaller batches.
std::vector<IScheduler::WireId<IScheduler::Boolean>>
PlaintextScheduler::unbatching(
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

std::vector<IScheduler::WireId<IScheduler::Boolean>>
PlaintextScheduler::validateAndComputeBatchCompositeAND(
    IScheduler::WireId<IScheduler::Boolean> left,
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rights,
    uint64_t& gateCounter) {
  auto leftValue = wireKeeper_->getBatchBooleanValue(left);
  std::vector<IScheduler::WireId<IScheduler::Boolean>> returnWires;
  for (auto rightWire : rights) {
    auto& rightValue = wireKeeper_->getBatchBooleanValue(rightWire);
    if (leftValue.size() != rightValue.size()) {
      throw std::invalid_argument("Batch inputs have differing sizes");
    }
    std::vector<bool> rst;
    for (size_t i = 0; i < leftValue.size(); i++) {
      rst.push_back(leftValue[i] & rightValue[i]);
    }
    returnWires.push_back(wireKeeper_->allocateBatchBooleanValue(rst));
  }
  gateCounter += leftValue.size() * rights.size();
  return returnWires;
}
} // namespace fbpcf::scheduler
