/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <vector>

#include "fbpcf/mpc_framework/engine/ISecretShareEngine.h"
#include "fbpcf/mpc_framework/engine/communication/ISecretShareEngineCommunicationAgent.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/ITupleGenerator.h"
#include "fbpcf/mpc_framework/engine/util/IPrgFactory.h"

namespace fbpcf::engine {

class SecretShareEngine final : public ISecretShareEngine {
 public:
  SecretShareEngine(
      std::unique_ptr<tuple_generator::ITupleGenerator> tupleGenerator,
      std::unique_ptr<communication::ISecretShareEngineCommunicationAgent>
          communicationAgent,
      std::unique_ptr<util::IPrgFactory> prgFactory,
      int myId,
      int numberOfParty);

  /**
   * @inherit doc
   */
  bool setInput(int id, std::optional<bool> v) override;

  /**
   * @inherit doc
   */
  std::vector<bool> setBatchInput(int id, const std::vector<bool>& v) override;

  /**
   * @inherit doc
   */
  bool computeSymmetricXOR(bool left, bool right) const override;

  /**
   * @inherit doc
   */
  std::vector<bool> computeBatchSymmetricXOR(
      const std::vector<bool>& left,
      const std::vector<bool>& right) const override;

  /**
   * @inherit doc
   */
  bool computeAsymmetricXOR(bool left, bool right) const override;

  /**
   * @inherit doc
   */
  std::vector<bool> computeBatchAsymmetricXOR(
      const std::vector<bool>& left,
      const std::vector<bool>& right) const override;

  /**
   * @inherit doc
   */
  bool computeSymmetricNOT(bool input) const override;

  /**
   * @inherit doc
   */
  std::vector<bool> computeBatchSymmetricNOT(
      const std::vector<bool>& input) const override;

  /**
   * @inherit doc
   */
  bool computeAsymmetricNOT(bool input) const override;

  /**
   * @inherit doc
   */
  std::vector<bool> computeBatchAsymmetricNOT(
      const std::vector<bool>& input) const override;

  /**
   * @inherit doc
   */
  bool computeFreeAND(bool left, bool right) const override;

  /**
   * @inherit doc
   */
  std::vector<bool> computeBatchFreeAND(
      const std::vector<bool>& left,
      const std::vector<bool>& right) const override;

  /**
   * @inherit doc
   */
  uint32_t scheduleAND(bool left, bool right) override;

  /**
   * @inherit doc
   */
  void executeScheduledAND() override;

  /**
   * @inherit doc
   */
  bool getANDExecutionResult(uint32_t index) const override;

  /**
   * @inherit doc
   */
  uint32_t scheduleBatchAND(
      const std::vector<bool>& left,
      const std::vector<bool>& right) override;

  /**
   * @inherit doc
   */
  const std::vector<bool>& getBatchANDExecutionResult(
      uint32_t index) const override;

  /**
   * @inherit doc
   */
  std::vector<bool> computeBatchAND(
      const std::vector<bool>& left,
      const std::vector<bool>& right) override;

  /**
   * @inherit doc
   */
  std::vector<bool> revealToParty(int id, const std::vector<bool>& output)
      const override;

  /**
   * @inherit doc
   */
  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    auto onlineCost = communicationAgent_->getTrafficStatistics();
    auto offlineCost = tupleGenerator_->getTrafficStatistics();
    return {
        onlineCost.first + offlineCost.first,
        onlineCost.second + offlineCost.second};
  }

 private:
  std::vector<std::vector<bool>> computeANDsWithVectorInputs(
      std::vector<std::reference_wrapper<const std::vector<bool>>> left,
      std::vector<std::reference_wrapper<const std::vector<bool>>> right);

  class ScheduledAND {
   public:
    ScheduledAND(bool left, bool right)
        : value_{static_cast<unsigned char>((left << 1) ^ right)} {}

    bool getLeft() const {
      return value_ >> 1;
    }

    bool getRight() const {
      return value_ & 1;
    }

   private:
    unsigned char value_;
  };

  class ScheduledBatchAND {
   public:
    ScheduledBatchAND(
        const std::vector<bool>& left,
        const std::vector<bool>& right)
        : left_(left), right_(right) {}

    std::vector<bool>& getLeft() {
      return left_;
    }

    std::vector<bool>& getRight() {
      return right_;
    }

   private:
    std::vector<bool> left_;
    std::vector<bool> right_;
  };

  std::unique_ptr<tuple_generator::ITupleGenerator> tupleGenerator_;
  std::unique_ptr<communication::ISecretShareEngineCommunicationAgent>
      communicationAgent_;
  std::unique_ptr<util::IPrgFactory> prgFactory_;

  int myId_;
  int numberOfParty_;

  // A pair of prg is kept for each peer party. We need a pair so every party
  // can decide the randomness used for masking his/her private inputs: the
  // first prg in the pair is to mask this party's input; the second one is to
  // mask peer's input.
  // This seed should be exclusively used to mask inputs and nothing else.
  std::map<
      int,
      std::pair<std::unique_ptr<util::IPrg>, std::unique_ptr<util::IPrg>>>
      inputPrgs_;

  // only for demonstration purpose, needs to replace with a more efficient
  // memory arena to speed up
  std::vector<ScheduledAND> scheduledANDGates_;

  // use executionResults_.at(0) to store non-batching execution results, rest
  // of them will be batch execution results
  std::vector<std::vector<bool>> executionResults_;

  std::vector<ScheduledBatchAND> scheduledBatchANDGates_;
};

} // namespace fbpcf::engine
