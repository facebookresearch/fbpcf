/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cstddef>
#include <iterator>
#include <stdexcept>

#include "fbpcf/mpc_framework/engine/SecretShareEngine.h"
#include "fbpcf/mpc_framework/engine/util/util.h"

namespace fbpcf::engine {
SecretShareEngine::SecretShareEngine(
    std::unique_ptr<tuple_generator::ITupleGenerator> tupleGenerator,
    std::unique_ptr<communication::ISecretShareEngineCommunicationAgent>
        communicationAgent,
    std::unique_ptr<util::IPrgFactory> prgFactory,
    int myId,
    int numberOfParty)
    : tupleGenerator_{std::move(tupleGenerator)},
      communicationAgent_{std::move(communicationAgent)},
      prgFactory_{std::move(prgFactory)},
      myId_{myId},
      numberOfParty_{numberOfParty} {
  std::map<int, __m128i> randomPrgKey;
  for (int i = 0; i < numberOfParty_; i++) {
    if (i != myId_) {
      randomPrgKey.emplace(i, util::getRandomM128iFromSystemNoise());
    }
  }
  auto inputPrgSeed = communicationAgent_->exchangeKeys(randomPrgKey);
  for (auto& item : inputPrgSeed) {
    inputPrgs_.emplace(
        item.first,
        std::make_pair(
            prgFactory_->create(randomPrgKey.at(item.first)),
            prgFactory_->create(item.second)));
  }
}

bool SecretShareEngine::setInput(int id, std::optional<bool> v) {
  if (id == myId_) {
    if (!v.has_value()) {
      throw std::invalid_argument("needs to provide input value");
    }
    bool rst = v.value();
    for (auto& item : inputPrgs_) {
      rst ^= item.second.first->getRandomBits(1)[0];
    }
    return rst;
  } else {
    assert(inputPrgs_.find(id) != inputPrgs_.end());
    return inputPrgs_.at(id).second->getRandomBits(1)[0];
  }
}

std::vector<bool> SecretShareEngine::setBatchInput(
    int id,
    const std::vector<bool>& v) {
  if (id == myId_) {
    auto size = v.size();
    std::vector<bool> rst(size);
    if (rst.size() == 0) {
      throw std::invalid_argument("empty input!");
    }
    for (int i = 0; i < size; i++) {
      rst[i] = v[i];
    }
    for (auto& item : inputPrgs_) {
      auto mask = item.second.first->getRandomBits(size);
      for (int i = 0; i < size; i++) {
        rst[i] = rst[i] ^ mask[i];
      }
    }
    return rst;
  } else {
    assert(inputPrgs_.find(id) != inputPrgs_.end());
    return inputPrgs_.at(id).second->getRandomBits(v.size());
  }
}

bool SecretShareEngine::computeSymmetricXOR(bool left, bool right) const {
  return left ^ right;
}

std::vector<bool> SecretShareEngine::computeBatchSymmetricXOR(
    const std::vector<bool>& left,
    const std::vector<bool>& right) const {
  if (left.size() != right.size()) {
    throw std::invalid_argument("The input sizes are not the same.");
  }
  if (left.size() == 0) {
    return std::vector<bool>();
  }
  std::vector<bool> rst(left.size());
  for (int i = 0; i < left.size(); i++) {
    rst[i] = left[i] ^ right[i];
  }
  return rst;
}

bool SecretShareEngine::computeAsymmetricXOR(bool left, bool right) const {
  if (myId_ == 0) {
    return left ^ right;
  } else {
    return left;
  }
}

std::vector<bool> SecretShareEngine::computeBatchAsymmetricXOR(
    const std::vector<bool>& left,
    const std::vector<bool>& right) const {
  if (left.size() != right.size()) {
    throw std::invalid_argument("The input sizes are not the same.");
  }
  if (left.size() == 0) {
    return std::vector<bool>();
  }
  if (myId_ == 0) {
    std::vector<bool> rst(left.size());
    for (int i = 0; i < left.size(); i++) {
      rst[i] = left[i] ^ right[i];
    }
    return rst;
  } else {
    return left;
  }
}

bool SecretShareEngine::computeSymmetricNOT(bool input) const {
  return !input;
}

std::vector<bool> SecretShareEngine::computeBatchSymmetricNOT(
    const std::vector<bool>& input) const {
  if (input.size() == 0) {
    return std::vector<bool>();
  }
  std::vector<bool> rst(input.size());
  for (int i = 0; i < rst.size(); i++) {
    rst[i] = !input[i];
  }
  return rst;
}

bool SecretShareEngine::computeAsymmetricNOT(bool input) const {
  if (myId_ == 0) {
    return !input;
  } else {
    return input;
  }
}

std::vector<bool> SecretShareEngine::computeBatchAsymmetricNOT(
    const std::vector<bool>& input) const {
  if (input.size() == 0) {
    return std::vector<bool>();
  }
  if (myId_ != 0) {
    return input;
  }
  std::vector<bool> rst(input.size());
  for (int i = 0; i < input.size(); i++) {
    rst[i] = !input[i];
  }
  return rst;
}

bool SecretShareEngine::computeFreeAND(bool left, bool right) const {
  return left & right;
}

std::vector<bool> SecretShareEngine::computeBatchFreeAND(
    const std::vector<bool>& left,
    const std::vector<bool>& right) const {
  if (left.size() != right.size()) {
    throw std::invalid_argument("The input sizes are not the same.");
  }
  if (left.size() == 0) {
    return std::vector<bool>();
  }
  std::vector<bool> rst(left.size());
  for (int i = 0; i < left.size(); i++) {
    rst[i] = left[i] & right[i];
  }
  return rst;
}

uint32_t SecretShareEngine::scheduleAND(bool left, bool right) {
  scheduledANDGates_.push_back(ScheduledAND(left, right));
  return scheduledANDGates_.size() - 1;
}

void SecretShareEngine::executeScheduledAND() {
  std::vector<bool> left(scheduledANDGates_.size());
  std::vector<bool> right(scheduledANDGates_.size());
  for (int i = 0; i < scheduledANDGates_.size(); i++) {
    left[i] = scheduledANDGates_[i].getLeft();
    right[i] = scheduledANDGates_[i].getRight();
  }

  std::vector<std::reference_wrapper<const std::vector<bool>>> leftBatch(
      1, std::reference_wrapper<const std::vector<bool>>(left));
  std::vector<std::reference_wrapper<const std::vector<bool>>> rightBatch(
      1, std::reference_wrapper<const std::vector<bool>>(right));

  for (int i = 0; i < scheduledBatchANDGates_.size(); i++) {
    leftBatch.push_back(std::reference_wrapper<const std::vector<bool>>(
        scheduledBatchANDGates_.at(i).getLeft()));
    rightBatch.push_back(std::reference_wrapper<const std::vector<bool>>(
        scheduledBatchANDGates_.at(i).getRight()));
  }

  executionResults_ = computeANDsWithVectorInputs(leftBatch, rightBatch);

  scheduledANDGates_ = std::vector<ScheduledAND>();
  scheduledBatchANDGates_ = std::vector<ScheduledBatchAND>();
}

bool SecretShareEngine::getANDExecutionResult(uint32_t index) const {
  return executionResults_.at(0).at(index);
}

uint32_t SecretShareEngine::scheduleBatchAND(
    const std::vector<bool>& left,
    const std::vector<bool>& right) {
  scheduledBatchANDGates_.push_back(ScheduledBatchAND(left, right));
  return scheduledBatchANDGates_.size() - 1;
}

const std::vector<bool>& SecretShareEngine::getBatchANDExecutionResult(
    uint32_t index) const {
  return executionResults_.at(index + 1);
}

std::vector<bool> SecretShareEngine::computeBatchAND(
    const std::vector<bool>& left,
    const std::vector<bool>& right) {
  return computeANDsWithVectorInputs(
             {std::reference_wrapper<const std::vector<bool>>(left)},
             {std::reference_wrapper<const std::vector<bool>>(right)})
      .at(0);
}

std::vector<std::vector<bool>> SecretShareEngine::computeANDsWithVectorInputs(
    std::vector<std::reference_wrapper<const std::vector<bool>>> left,
    std::vector<std::reference_wrapper<const std::vector<bool>>> right) {
  auto count = left.size();
  if (count != right.size()) {
    throw std::invalid_argument("The input sizes are not the same.");
  }
  if (count == 0) {
    return std::vector<std::vector<bool>>();
  }
  size_t size = 0;
  for (size_t i = 0; i < count; i++) {
    if (left.at(i).get().size() != right.at(i).get().size()) {
      throw std::invalid_argument("The input sizes are not the same.");
    }
    size += left.at(i).get().size();
  }

  auto tuples = tupleGenerator_->getBooleanTuple(size);
  std::vector<bool> secretsToOpen(size * 2);

  size_t index = 0;
  for (size_t i = 0; i < count; i++) {
    for (size_t j = 0; j < left.at(i).get().size(); j++) {
      secretsToOpen[index * 2] =
          left.at(i).get().at(j) ^ tuples.at(index).getA();
      secretsToOpen[index * 2 + 1] =
          right.at(i).get().at(j) ^ tuples.at(index).getB();
      index++;
    }
  }
  auto openedSecrets =
      communicationAgent_->openSecretsToAll(std::move(secretsToOpen));

  if (openedSecrets.size() != size * 2) {
    throw std::runtime_error("unexpected number of opened secrets");
  }

  std::vector<std::vector<bool>> rst(count);
  index = 0;
  for (size_t i = 0; i < count; i++) {
    rst[i] = std::vector<bool>(left.at(i).get().size());
    for (size_t j = 0; j < left.at(i).get().size(); j++) {
      rst[i][j] = tuples.at(index).getC() ^
          (openedSecrets.at(2 * index) & tuples.at(index).getB()) ^
          (openedSecrets.at(2 * index + 1) & tuples.at(index).getA());
      if (myId_ == 0) {
        rst[i][j] = rst[i][j] ^
            (openedSecrets.at(2 * index) & openedSecrets.at(2 * index + 1));
      }
      index++;
    }
  }

  return rst;
}

std::vector<bool> SecretShareEngine::revealToParty(
    int id,
    const std::vector<bool>& output) const {
  return communicationAgent_->openSecretsToParty(id, output);
}

} // namespace fbpcf::engine
