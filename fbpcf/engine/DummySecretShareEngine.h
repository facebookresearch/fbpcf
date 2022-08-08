/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fmt/format.h>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "fbpcf/engine/ISecretShareEngine.h"

namespace fbpcf::engine {

/**
 * This is merely a dummy engine - it won't actually conduct any computation.
 * The results it returns will match the expected output size, but the content
 * can be nonsense. It should only be used as a placeholder.
 */
class DummySecretShareEngine final : public ISecretShareEngine {
 public:
  explicit DummySecretShareEngine(int myId) : myId_(myId) {}

  /**
   * @inherit doc
   */
  bool setInput(int id, std::optional<bool> v) override {
    if (id == myId_ && (!v.has_value())) {
      throw std::invalid_argument("needs to provide input value");
    }
    return true;
  }

  /**
   * @inherit doc
   */
  std::vector<bool> setBatchInput(int id, const std::vector<bool>& v) override {
    if (id == myId_ && v.size() == 0) {
      throw std::invalid_argument("empty input!");
    }
    return v;
  }

  /**
   * @inherit doc
   */
  bool computeSymmetricXOR(
      [[maybe_unused]] bool left,
      [[maybe_unused]] bool right) const override {
    return true;
  }

  /**
   * @inherit doc
   */
  std::vector<bool> computeBatchSymmetricXOR(
      const std::vector<bool>& left,
      const std::vector<bool>& right) const override {
    if (left.size() != right.size()) {
      throw std::invalid_argument("The input sizes are not the same.");
    }
    return left;
  }

  /**
   * @inherit doc
   */
  bool computeAsymmetricXOR(
      [[maybe_unused]] bool left,
      [[maybe_unused]] bool right) const override {
    return true;
  }

  /**
   * @inherit doc
   */
  std::vector<bool> computeBatchAsymmetricXOR(
      const std::vector<bool>& left,
      const std::vector<bool>& right) const override {
    if (left.size() != right.size()) {
      throw std::invalid_argument("The input sizes are not the same.");
    }
    return left;
  }

  /**
   * @inherit doc
   */
  bool computeSymmetricNOT([[maybe_unused]] bool input) const override {
    return true;
  }

  /**
   * @inherit doc
   */
  std::vector<bool> computeBatchSymmetricNOT(
      const std::vector<bool>& input) const override {
    return input;
  }

  /**
   * @inherit doc
   */
  bool computeAsymmetricNOT([[maybe_unused]] bool input) const override {
    return true;
  }

  /**
   * @inherit doc
   */
  std::vector<bool> computeBatchAsymmetricNOT(
      const std::vector<bool>& input) const override {
    return input;
  }

  //======== Below are free AND computation API's: ========

  /**
   * @inherit doc
   */
  bool computeFreeAND([[maybe_unused]] bool left, [[maybe_unused]] bool right)
      const override {
    return true;
  }

  /**
   * @inherit doc
   */
  std::vector<bool> computeBatchFreeAND(
      const std::vector<bool>& left,
      const std::vector<bool>& right) const override {
    if (left.size() != right.size()) {
      throw std::invalid_argument("The input sizes are not the same.");
    }
    return left;
  }

  //======== Below are API's to schedule non-free AND's: ========

  /**
   * @inherit doc
   */
  uint32_t scheduleAND([[maybe_unused]] bool left, [[maybe_unused]] bool right)
      override {
    return 0;
  }

  /**
   * @inherit doc
   */
  uint32_t scheduleBatchAND(
      const std::vector<bool>& left,
      const std::vector<bool>& right) override {
    if (left.size() != right.size()) {
      throw std::runtime_error("Batch AND's must have the same length");
    }
    dummyBatchANDResults_.push_back(left);
    return dummyBatchANDResults_.size() - 1;
  }

  /**
   * @inherit doc
   */
  uint32_t scheduleCompositeAND(
      [[maybe_unused]] bool left,
      std::vector<bool> rights) override {
    dummyCompositeANDResults_.push_back(rights);
    return dummyCompositeANDResults_.size() - 1;
  }

  /**
   * @inherit doc
   */
  uint32_t scheduleBatchCompositeAND(
      const std::vector<bool>& left,
      const std::vector<std::vector<bool>>& rights) override {
    auto batchSize = left.size();
    for (auto rightValue : rights) {
      if (rightValue.size() != batchSize) {
        throw std::runtime_error(fmt::format(
            "Batch composite AND must have left.size() = rights[i].size() for all i. (Got {:d} != {:d})",
            left.size(),
            rightValue.size()));
      }
    }
    dummyCompositeBatchANDResults_.push_back(rights);
    return dummyCompositeBatchANDResults_.size() - 1;
  }

  //======== Below are API's to execute non free AND's: ========

  /**
   * @inherit doc
   */
  std::vector<bool> computeBatchANDImmediately(
      const std::vector<bool>& left,
      const std::vector<bool>& right) override {
    if (left.size() != right.size()) {
      throw std::runtime_error("Left and right must have equal length");
    }
    return left;
  }

  //======== Below are API's to retrieve non-free AND results: ========

  /**
   * @inherit doc
   */
  bool getANDExecutionResult([[maybe_unused]] uint32_t index) const override {
    return true;
  }

  /**
   * @inherit doc
   */
  const std::vector<bool>& getBatchANDExecutionResult(
      uint32_t index) const override {
    return dummyBatchANDResults_.at(index);
  }

  /**
   * @inherit doc
   */
  const std::vector<bool>& getCompositeANDExecutionResult(
      uint32_t index) const override {
    return dummyCompositeANDResults_.at(index);
  }

  /**
   * @inherit doc
   */
  const std::vector<std::vector<bool>>& getBatchCompositeANDExecutionResult(
      uint32_t index) const override {
    return dummyCompositeBatchANDResults_.at(index);
  }

  /**
   * @inherit doc
   */
  std::vector<bool> revealToParty(
      [[maybe_unused]] int id,
      const std::vector<bool>& output) const override {
    return output;
  }

  /**
   * @inherit doc
   */
  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    return {0, 0};
  }

  /**
   * @inherit doc
   */
  uint64_t setIntegerInput(int id, std::optional<uint64_t> v) override {
    if (id == myId_ && (!v.has_value())) {
      throw std::invalid_argument("needs to provide input value");
    }
    return 0;
  }

  /**
   * @inherit doc
   */
  std::vector<uint64_t> setBatchIntegerInput(
      int id,
      const std::vector<uint64_t>& v) override {
    if (id == myId_ && v.size() == 0) {
      throw std::invalid_argument("empty input!");
    }
    return v;
  }

  /**
   * @inherit doc
   */
  uint64_t computeSymmetricPlus(
      [[maybe_unused]] uint64_t left,
      [[maybe_unused]] uint64_t right) const override {
    return 0;
  }

  /**
   * @inherit doc
   */
  std::vector<uint64_t> computeBatchSymmetricPlus(
      const std::vector<uint64_t>& left,
      const std::vector<uint64_t>& right) const override {
    if (left.size() != right.size()) {
      throw std::invalid_argument("The input sizes are not the same.");
    }
    return left;
  }

  /**
   * @inherit doc
   */
  uint64_t computeAsymmetricPlus(
      [[maybe_unused]] uint64_t left,
      [[maybe_unused]] uint64_t right) const override {
    return 0;
  }

  /**
   * @inherit doc
   */
  std::vector<uint64_t> computeBatchAsymmetricPlus(
      const std::vector<uint64_t>& left,
      const std::vector<uint64_t>& right) const override {
    if (left.size() != right.size()) {
      throw std::invalid_argument("The input sizes are not the same.");
    }
    return left;
  }

  /**
   * @inherit doc
   */
  uint64_t computeSymmetricNeg([[maybe_unused]] uint64_t input) const override {
    return 0;
  }

  /**
   * @inherit doc
   */
  std::vector<uint64_t> computeBatchSymmetricNeg(
      const std::vector<uint64_t>& input) const override {
    return input;
  }

  //======== Below are free Mult computation API's: ========

  /**
   * @inherit doc
   */
  uint64_t computeFreeMult(
      [[maybe_unused]] uint64_t left,
      [[maybe_unused]] uint64_t right) const override {
    return 0;
  }

  /**
   * @inherit doc
   */
  std::vector<uint64_t> computeBatchFreeMult(
      const std::vector<uint64_t>& left,
      const std::vector<uint64_t>& right) const override {
    if (left.size() != right.size()) {
      throw std::invalid_argument("The input sizes are not the same.");
    }
    return left;
  }

  //======== Below are API's to schedule non-free Mult's: ========

  /**
   * @inherit doc
   */
  uint32_t scheduleMult(
      [[maybe_unused]] uint64_t left,
      [[maybe_unused]] uint64_t right) override {
    return 0;
  }

  /**
   * @inherit doc
   */
  uint32_t scheduleBatchMult(
      const std::vector<uint64_t>& left,
      const std::vector<uint64_t>& right) override {
    if (left.size() != right.size()) {
      throw std::runtime_error("Batch Mult's must have the same length");
    }
    dummyBatchMultResults_.push_back(left);
    return dummyBatchMultResults_.size() - 1;
  }

  //======== Below are API's to execute non free Mult's: ========

  /**
   * @inherit doc
   */
  std::vector<uint64_t> computeBatchMultImmediately(
      const std::vector<uint64_t>& left,
      const std::vector<uint64_t>& right) override {
    if (left.size() != right.size()) {
      throw std::runtime_error("Left and right must have equal length");
    }
    return left;
  }

  //======== Below are API's to retrieve non-free Mult results: ========

  /**
   * @inherit doc
   */
  uint64_t getMultExecutionResult(
      [[maybe_unused]] uint32_t index) const override {
    return 0;
  }

  /**
   * @inherit doc
   */
  const std::vector<uint64_t>& getBatchMultExecutionResult(
      uint32_t index) const override {
    return dummyBatchMultResults_.at(index);
  }

  /**
   * @inherit doc
   */
  std::vector<uint64_t> revealToParty(
      int /* id*/,
      const std::vector<uint64_t>& output) const override {
    return output;
  }

  //======== Below are API's to execute non free AND's and Mult's: ========
  /**
   * @inherit doc
   */
  void executeScheduledOperations() override {}

 private:
  std::vector<std::vector<bool>> dummyBatchANDResults_;
  std::vector<std::vector<bool>> dummyCompositeANDResults_;
  std::vector<std::vector<std::vector<bool>>> dummyCompositeBatchANDResults_;

  std::vector<std::vector<uint64_t>> dummyBatchMultResults_;

  int myId_;
};

} // namespace fbpcf::engine
