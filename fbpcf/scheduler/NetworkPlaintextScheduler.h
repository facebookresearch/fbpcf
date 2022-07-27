/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <map>
#include <memory>

#include <fbpcf/scheduler/PlaintextScheduler.h>
#include "fbpcf/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/scheduler/IScheduler.h"
#include "fbpcf/scheduler/IWireKeeper.h"

namespace fbpcf::scheduler {

/**
 * A scheduler that carries out computations in plaintext over the network.
 * Each party's input is immediately shared with all other parties.
 * This should only be used for debugging purposes.
 */
class NetworkPlaintextScheduler final : public PlaintextScheduler {
 public:
  explicit NetworkPlaintextScheduler(
      int myId,
      std::map<
          int,
          std::unique_ptr<engine::communication::IPartyCommunicationAgent>>
          agentMap,
      std::unique_ptr<IWireKeeper> wireKeeper);

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
  WireId<IScheduler::Boolean> recoverBooleanWire(bool v) override;

  /**
   * @inherit doc
   */
  WireId<IScheduler::Boolean> recoverBooleanWireBatch(
      const std::vector<bool>& v) override;

  /**
   * @inherit doc
   */
  WireId<IScheduler::Arithmetic> privateIntegerInput(uint64_t v, int partyId)
      override;

  /**
   * @inherit doc
   */
  WireId<IScheduler::Arithmetic> privateIntegerInputBatch(
      const std::vector<uint64_t>& v,
      int partyId) override;

  /**
   * @inherit doc
   */
  WireId<IScheduler::Arithmetic> recoverIntegerWire(uint64_t v) override;

  /**
   * @inherit doc
   */
  WireId<IScheduler::Arithmetic> recoverIntegerWireBatch(
      const std::vector<uint64_t>& v) override;

  //======== Below are output processing APIs: ========

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
  uint64_t extractIntegerSecretShare(
      WireId<IScheduler::Arithmetic> id) override;

  /**
   * @inherit doc
   */
  std::vector<uint64_t> extractIntegerSecretShareBatch(
      WireId<IScheduler::Arithmetic> id) override;

  //======== Below are miscellaneous APIs: ========

  /**
   * @inherit doc
   */
  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    uint64_t sent = 0;
    uint64_t received = 0;
    for (auto& item : agentMap_) {
      auto traffic = item.second->getTrafficStatistics();
      sent += traffic.first;
      received += traffic.second;
    }
    return {sent, received};
  }

 private:
  int myId_;
  std::
      map<int, std::unique_ptr<engine::communication::IPartyCommunicationAgent>>
          agentMap_;
};

} // namespace fbpcf::scheduler
