/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emmintrin.h>
#include "fbpcf/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/engine/util/util.h"
#include "fbpcf/mpc_std_lib/aes_circuit/IAesCircuitCtr.h"
#include "fbpcf/mpc_std_lib/unified_data_process/data_processor/IDataProcessor.h"
#include "fbpcf/mpc_std_lib/unified_data_process/data_processor/UdpEncryption.h"
#include "fbpcf/primitive/mac/S2v.h"
#include "fbpcf/primitive/mac/S2vFactory.h"

namespace fbpcf::mpc_std_lib::unified_data_process::data_processor {

/**
 * This is the implementation of UDP data processor.
 */
template <int schedulerId>
class DataProcessor final : public IDataProcessor<schedulerId> {
 public:
  using AesCtr =
      aes_circuit::IAesCircuitCtr<typename IDataProcessor<schedulerId>::SecBit>;

  explicit DataProcessor(
      int32_t myId,
      int32_t partnerId,
      std::unique_ptr<fbpcf::engine::communication::IPartyCommunicationAgent>
          agent,
      std::unique_ptr<AesCtr> aesCircuitCtr)
      : myId_(myId),
        partnerId_(partnerId),
        encrypter_(std::move(agent)),
        aesCircuitCtr_(std::move(aesCircuitCtr)) {}

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
  UdpEncryption encrypter_;
  std::unique_ptr<AesCtr> aesCircuitCtr_;
};

} // namespace fbpcf::mpc_std_lib::unified_data_process::data_processor

#include "fbpcf/mpc_std_lib/unified_data_process/data_processor/DataProcessor_impl.h"
