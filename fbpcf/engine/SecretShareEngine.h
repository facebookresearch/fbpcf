/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "fbpcf/engine/ISecretShareEngine.h"
#include "fbpcf/engine/communication/ISecretShareEngineCommunicationAgent.h"
#include "fbpcf/engine/tuple_generator/ITupleGenerator.h"
#include "fbpcf/engine/util/IPrgFactory.h"

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

  //======== Below are free AND computation API's: ========

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

  //======== Below are API's to schedule non-free AND's: ========

  /**
   * @inherit doc
   */
  uint32_t scheduleAND(bool left, bool right) override;

  /**
   * @inherit doc
   */
  uint32_t scheduleBatchAND(
      const std::vector<bool>& left,
      const std::vector<bool>& right) override;

  /**
   * @inherit doc
   */
  uint32_t scheduleCompositeAND(bool left, std::vector<bool> rights) override;

  /**
   * @inherit doc
   */
  uint32_t scheduleBatchCompositeAND(
      const std::vector<bool>& left,
      const std::vector<std::vector<bool>>& rights) override;

  //======== Below are API's to execute non free AND's: ========

  /**
   * @inherit doc
   */
  void executeScheduledAND() override;

  /**
   * @inherit doc
   */
  std::vector<bool> computeBatchANDImmediately(
      const std::vector<bool>& left,
      const std::vector<bool>& right) override;

  //======== Below are API's to retrieve non-free AND results: ========

  /**
   * @inherit doc
   */
  bool getANDExecutionResult(uint32_t index) const override;

  /**
   * @inherit doc
   */
  const std::vector<bool>& getBatchANDExecutionResult(
      uint32_t index) const override;

  /**
   * @inherit doc
   */
  const std::vector<bool>& getCompositeANDExecutionResult(
      uint32_t index) const override;

  /**
   * @inherit doc
   */
  const std::vector<std::vector<bool>>& getBatchCompositeANDExecutionResult(
      uint32_t index) const override;

  /**
   * @inherit doc
   */
  std::vector<bool> revealToParty(int id, const std::vector<bool>& output)
      const override;

  /**
   * @inherit doc
   */
  uint64_t setIntegerInput(int id, std::optional<uint64_t> v) override;

  /**
   * @inherit doc
   */
  std::vector<uint64_t> setBatchIntegerInput(
      int id,
      const std::vector<uint64_t>& v) override;

  /**
   * @inherit doc
   */
  std::vector<uint64_t> revealToParty(
      int id,
      const std::vector<uint64_t>& output) const override;

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
  struct ExecutionResults {
    std::vector<bool> andResults;
    std::vector<std::vector<bool>> batchANDResults;
    std::vector<std::vector<bool>> compositeANDResults;
    std::vector<std::vector<std::vector<bool>>> compositeBatchANDResults;
  };

  class ScheduledAND {
   public:
    ScheduledAND(bool left, bool right)
        : value_{static_cast<unsigned char>((left << 1) ^ right)} {}

    explicit ScheduledAND(const ScheduledAND&) = default;
    ScheduledAND(ScheduledAND&&) = default;
    ScheduledAND& operator=(const ScheduledAND&) = delete;
    ScheduledAND& operator=(ScheduledAND&&) = delete;

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

    explicit ScheduledBatchAND(const ScheduledBatchAND&) = default;
    ScheduledBatchAND(ScheduledBatchAND&&) = default;
    ScheduledBatchAND& operator=(const ScheduledBatchAND&) = delete;
    ScheduledBatchAND& operator=(ScheduledBatchAND&&) = delete;

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

  class ScheduledCompositeAND {
   public:
    ScheduledCompositeAND(bool left, std::vector<bool>& rights)
        : left_{left}, rights_{rights} {}

    explicit ScheduledCompositeAND(const ScheduledCompositeAND&) = default;
    ScheduledCompositeAND(ScheduledCompositeAND&&) = default;
    ScheduledCompositeAND& operator=(const ScheduledCompositeAND&) = delete;
    ScheduledCompositeAND& operator=(ScheduledCompositeAND&&) = delete;

    bool getLeft() const {
      return left_;
    }

    std::vector<bool>& getRights() {
      return rights_;
    }

   private:
    bool left_;
    std::vector<bool> rights_;
  };

  class ScheduledBatchCompositeAND {
   public:
    ScheduledBatchCompositeAND(
        const std::vector<bool>& left,
        const std::vector<std::vector<bool>>& rights)
        : left_(left), rights_(rights) {}

    explicit ScheduledBatchCompositeAND(const ScheduledBatchCompositeAND&) =
        default;
    ScheduledBatchCompositeAND(ScheduledBatchCompositeAND&&) = default;
    ScheduledBatchCompositeAND& operator=(const ScheduledBatchCompositeAND&) =
        delete;
    ScheduledBatchCompositeAND& operator=(ScheduledBatchCompositeAND&&) =
        delete;

    std::vector<bool>& getLeft() {
      return left_;
    }

    std::vector<std::vector<bool>>& getRights() {
      return rights_;
    }

   private:
    std::vector<bool> left_;
    std::vector<std::vector<bool>> rights_;
  };

  ExecutionResults computeAllANDsFromScheduledANDs(
      std::vector<ScheduledAND>& ands,
      std::vector<ScheduledBatchAND>& batchAnds,
      std::vector<ScheduledCompositeAND>& compositeAnds,
      std::vector<ScheduledBatchCompositeAND>& batchCompositeAnds);

  std::vector<bool> computeSecretSharesToOpen(
      std::vector<ScheduledAND>& ands,
      std::vector<ScheduledBatchAND>& batchAnds,
      std::vector<ScheduledCompositeAND>& compositeAnds,
      std::vector<ScheduledBatchCompositeAND>& batchCompositeAnds,
      std::vector<tuple_generator::ITupleGenerator::BooleanTuple>& normalTuples,
      std::map<
          size_t,
          std::vector<tuple_generator::ITupleGenerator::CompositeBooleanTuple>>&
          compositeTuples,
      size_t openedSecretCount);

  ExecutionResults computeExecutionResultsFromOpenedShares(
      std::vector<ScheduledAND>& ands,
      std::vector<ScheduledBatchAND>& batchAnds,
      std::vector<ScheduledCompositeAND>& compositeAnds,
      std::vector<ScheduledBatchCompositeAND>& batchCompositeAnds,
      std::vector<bool>& openedSecrets,
      std::vector<tuple_generator::ITupleGenerator::BooleanTuple>& normalTuples,
      std::map<
          size_t,
          std::vector<tuple_generator::ITupleGenerator::CompositeBooleanTuple>>&
          compositeTuples);

  std::vector<bool> computeSecretSharesToOpenLegacy(
      std::vector<ScheduledAND>& ands,
      std::vector<ScheduledBatchAND>& batchAnds,
      std::vector<ScheduledCompositeAND>& compositeAnds,
      std::vector<ScheduledBatchCompositeAND>& batchCompositeAnds,
      std::vector<tuple_generator::ITupleGenerator::BooleanTuple>& tuples);

  ExecutionResults computeExecutionResultsFromOpenedSharesLegacy(
      std::vector<ScheduledAND>& ands,
      std::vector<ScheduledBatchAND>& batchAnds,
      std::vector<ScheduledCompositeAND>& compositeAnds,
      std::vector<ScheduledBatchCompositeAND>& batchCompositeAnds,
      std::vector<bool>& openedSecrets,
      std::vector<tuple_generator::ITupleGenerator::BooleanTuple>& tuples);

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
  std::vector<ScheduledBatchAND> scheduledBatchANDGates_;
  std::vector<ScheduledCompositeAND> scheduledCompositeANDGates_;
  std::vector<ScheduledBatchCompositeAND> scheduledBatchCompositeANDGates_;

  ExecutionResults executionResults_;
};

} // namespace fbpcf::engine
