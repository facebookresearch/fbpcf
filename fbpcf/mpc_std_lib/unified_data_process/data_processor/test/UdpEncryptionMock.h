/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "fbpcf/mpc_std_lib/unified_data_process/data_processor/IUdpEncryption.h"

namespace fbpcf::mpc_std_lib::unified_data_process::data_processor {

using namespace ::testing;

class UdpEncryptionMock final : public IUdpEncryption {
 public:
  MOCK_METHOD(void, prepareToProcessMyData, (size_t));

  MOCK_METHOD(
      void,
      processMyData,
      (const std::vector<std::vector<unsigned char>>&,
       const std::vector<uint64_t>& indexes));

  MOCK_METHOD(std::vector<__m128i>, getExpandedKey, ());

  MOCK_METHOD(
      void,
      prepareToProcessPeerData,
      (size_t, const std::vector<uint64_t>&));

  MOCK_METHOD(void, processPeerData, (size_t));

  MOCK_METHOD(EncryptionResults, getProcessedData, ());
};

} // namespace fbpcf::mpc_std_lib::unified_data_process::data_processor
