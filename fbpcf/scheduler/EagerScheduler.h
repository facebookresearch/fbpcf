/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/engine/ISecretShareEngine.h"
#include "fbpcf/scheduler/IScheduler.h"
#include "fbpcf/scheduler/IWireKeeper.h"
#include "fbpcf/util/MetricCollector.h"

namespace fbpcf::scheduler {

/**
 * An "eager" scheduler immediately carries out all computations upon
 * request. It is cryptographically secure if the underlying secret
 * sharing engine is.
 */
class EagerScheduler final : public IScheduler {
 public:
  explicit EagerScheduler(
      std::unique_ptr<engine::ISecretShareEngine> engine,
      std::unique_ptr<IWireKeeper> wireKeeper,
      std::shared_ptr<util::MetricCollector> collector =
          std::make_shared<util::MetricCollector>("eager_scheduler"));

  //======== Below are input processing APIs: ========

  /**
   * @inherit doc
   */
  WireId<IScheduler::Boolean> privateBooleanInput(bool v, int partyId) override;

  /**
   * @inherit doc
   */
  WireId<IScheduler::Boolean> privateBooleanInputBatch(
      const std::vector<bool>& v,
      int partyId) override;

  /**
   * @inherit doc
   */
  WireId<IScheduler::Boolean> publicBooleanInput(bool v) override;

  /**
   * @inherit doc
   */
  WireId<IScheduler::Boolean> publicBooleanInputBatch(
      const std::vector<bool>& v) override;

  /**
   * @inherit doc
   */
  WireId<IScheduler::Boolean> recoverBooleanWire(bool v) override;

  /**
   * @inherit doc
   */
  WireId<IScheduler::Boolean> recoverBooleanWireBatch(
      const std::vector<bool>& v) override;

  //======== Below are output processing APIs: ========
  /**
   * @inherit doc
   */
  WireId<IScheduler::Boolean> openBooleanValueToParty(
      WireId<IScheduler::Boolean> src,
      int partyId) override;

  /**
   * @inherit doc
   */
  WireId<IScheduler::Boolean> openBooleanValueToPartyBatch(
      WireId<IScheduler::Boolean> src,
      int partyId) override;

  /**
   * @inherit doc
   */
  bool extractBooleanSecretShare(WireId<IScheduler::Boolean> id) override;

  /**
   * @inherit doc
   */
  std::vector<bool> extractBooleanSecretShareBatch(
      WireId<IScheduler::Boolean> id) override;

  /**
   * @inherit doc
   */
  bool getBooleanValue(WireId<IScheduler::Boolean> id) override;

  /**
   * @inherit doc
   */
  std::vector<bool> getBooleanValueBatch(
      WireId<IScheduler::Boolean> id) override;

  //======== Below are computation APIs: ========

  // ------ AND gates ------

  /**
   * @inherit doc
   */
  WireId<IScheduler::Boolean> privateAndPrivate(
      WireId<IScheduler::Boolean> left,
      WireId<IScheduler::Boolean> right) override;

  /**
   * @inherit doc
   */
  WireId<IScheduler::Boolean> privateAndPrivateBatch(
      WireId<IScheduler::Boolean> left,
      WireId<IScheduler::Boolean> right) override;

  /**
   * @inherit doc
   */
  WireId<IScheduler::Boolean> privateAndPublic(
      WireId<IScheduler::Boolean> left,
      WireId<IScheduler::Boolean> right) override;

  /**
   * @inherit doc
   */
  WireId<IScheduler::Boolean> privateAndPublicBatch(
      WireId<IScheduler::Boolean> left,
      WireId<IScheduler::Boolean> right) override;

  /**
   * @inherit doc
   */
  WireId<IScheduler::Boolean> publicAndPublic(
      WireId<IScheduler::Boolean> left,
      WireId<IScheduler::Boolean> right) override;

  /**
   * @inherit doc
   */
  WireId<IScheduler::Boolean> publicAndPublicBatch(
      WireId<IScheduler::Boolean> left,
      WireId<IScheduler::Boolean> right) override;

  // ------ Composite AND gates ------

  /**
   * @inherit doc
   */
  std::vector<WireId<Boolean>> privateAndPrivateComposite(
      WireId<Boolean> left,
      std::vector<WireId<Boolean>> rights) override;

  /**
   * @inherit doc
   */
  std::vector<WireId<Boolean>> privateAndPrivateCompositeBatch(
      WireId<Boolean> left,
      std::vector<WireId<Boolean>> rights) override;

  /**
   * @inherit doc
   */
  std::vector<WireId<Boolean>> privateAndPublicComposite(
      WireId<Boolean> left,
      std::vector<WireId<Boolean>> rights) override;

  /**
   * @inherit doc
   */
  std::vector<WireId<Boolean>> privateAndPublicCompositeBatch(
      WireId<Boolean> left,
      std::vector<WireId<Boolean>> rights) override;

  /**
   * @inherit doc
   */
  std::vector<WireId<Boolean>> publicAndPublicComposite(
      WireId<Boolean> left,
      std::vector<WireId<Boolean>> rights) override;

  /**
   * @inherit doc
   */
  std::vector<WireId<Boolean>> publicAndPublicCompositeBatch(
      WireId<Boolean> left,
      std::vector<WireId<Boolean>> rights) override;

  // ------ XOR gates ------

  /**
   * @inherit doc
   */
  WireId<IScheduler::Boolean> privateXorPrivate(
      WireId<IScheduler::Boolean> left,
      WireId<IScheduler::Boolean> right) override;

  /**
   * @inherit doc
   */
  WireId<IScheduler::Boolean> privateXorPrivateBatch(
      WireId<IScheduler::Boolean> left,
      WireId<IScheduler::Boolean> right) override;

  /**
   * @inherit doc
   */
  WireId<IScheduler::Boolean> privateXorPublic(
      WireId<IScheduler::Boolean> left,
      WireId<IScheduler::Boolean> right) override;

  /**
   * @inherit doc
   */
  WireId<IScheduler::Boolean> privateXorPublicBatch(
      WireId<IScheduler::Boolean> left,
      WireId<IScheduler::Boolean> right) override;

  /**
   * @inherit doc
   */
  WireId<IScheduler::Boolean> publicXorPublic(
      WireId<IScheduler::Boolean> left,
      WireId<IScheduler::Boolean> right) override;

  /**
   * @inherit doc
   */
  WireId<IScheduler::Boolean> publicXorPublicBatch(
      WireId<IScheduler::Boolean> left,
      WireId<IScheduler::Boolean> right) override;

  // ------ Not gates ------

  /**
   * @inherit doc
   */
  WireId<IScheduler::Boolean> notPrivate(
      WireId<IScheduler::Boolean> src) override;

  /**
   * @inherit doc
   */
  WireId<IScheduler::Boolean> notPrivateBatch(
      WireId<IScheduler::Boolean> src) override;

  /**
   * @inherit doc
   */
  WireId<IScheduler::Boolean> notPublic(
      WireId<IScheduler::Boolean> src) override;

  /**
   * @inherit doc
   */
  WireId<IScheduler::Boolean> notPublicBatch(
      WireId<IScheduler::Boolean> src) override;

  //======== Below are wire management APIs: ========

  /**
   * @inherit doc
   */
  void increaseReferenceCount(WireId<IScheduler::Boolean> src) override;

  /**
   * @inherit doc
   */
  void increaseReferenceCountBatch(WireId<IScheduler::Boolean> src) override;

  /**
   * @inherit doc
   */
  void decreaseReferenceCount(WireId<IScheduler::Boolean> id) override;

  /**
   * @inherit doc
   */
  void decreaseReferenceCountBatch(WireId<IScheduler::Boolean> id) override;

  //======== Below are rebatching APIs: ========

  // band a number of batches into one batch.
  WireId<Boolean> batchingUp(std::vector<WireId<Boolean>> src) override;

  // decompose a batch of values into several smaller batches.
  std::vector<WireId<Boolean>> unbatching(
      WireId<Boolean> src,
      std::shared_ptr<std::vector<uint32_t>> unbatchingStrategy) override;

  //======== Below are miscellaneous APIs: ========

  /**
   * @inherit doc
   */
  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override;

  /**
   * @inherit doc
   */
  std::pair<uint64_t, uint64_t> getWireStatistics() const override {
    return wireKeeper_->getWireStatistics();
  }

 private:
  std::unique_ptr<engine::ISecretShareEngine> engine_;
  std::unique_ptr<IWireKeeper> wireKeeper_;
  std::shared_ptr<util::MetricCollector> collector_;
};

} // namespace fbpcf::scheduler
