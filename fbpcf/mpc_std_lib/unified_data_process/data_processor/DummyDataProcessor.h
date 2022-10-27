/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_std_lib/unified_data_process/data_processor/IDataProcessor.h"

namespace fbpcf::mpc_std_lib::unified_data_process::data_processor::insecure {

/**
 * This is an insecure implementation. This object is only meant to be used as a
 * placeholder for testing.
 */
template <int schedulerId>
class DummyDataProcessor final : public IDataProcessor<schedulerId> {
 public:
  explicit DummyDataProcessor(
      int32_t myId,
      int32_t partnerId,
      std::unique_ptr<engine::communication::IPartyCommunicationAgent> agent)
      : myId_(myId), partnerId_(partnerId), agent_(std::move(agent)) {}

  /**
   * @inherit doc
   */
  typename IDataProcessor<schedulerId>::SecString processMyData(
      const std::vector<std::vector<unsigned char>>& plaintextData,
      size_t outputSize) override;

  /**
   * @inherit doc
   */
  typename IDataProcessor<schedulerId>::SecString processPeersData(
      size_t dataSize,
      const std::vector<int32_t>& indexes,
      size_t dataWidth) override;

 private:
  int32_t myId_;
  int32_t partnerId_;
  std::unique_ptr<engine::communication::IPartyCommunicationAgent> agent_;
};

} // namespace
  // fbpcf::mpc_std_lib::unified_data_process::data_processor::insecure

#include "fbpcf/mpc_std_lib/unified_data_process/data_processor/DummyDataProcessor_impl.h"
