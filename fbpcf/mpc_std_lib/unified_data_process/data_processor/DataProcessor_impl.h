/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emmintrin.h>
#include <fbpcf/engine/util/util.h>
#include "fbpcf/engine/util/aes.h"
#include "fbpcf/mpc_std_lib/aes_circuit/AesCircuitCtr.h"
#include "fbpcf/mpc_std_lib/unified_data_process/data_processor/DataProcessor.h"
#include "fbpcf/mpc_std_lib/unified_data_process/data_processor/UdpDecryption.h"
#include "fbpcf/mpc_std_lib/unified_data_process/data_processor/UdpUtil.h"

namespace fbpcf::mpc_std_lib::unified_data_process::data_processor {

template <int schedulerId>
typename IDataProcessor<schedulerId>::SecString
DataProcessor<schedulerId>::processMyData(
    const std::vector<std::vector<unsigned char>>& plaintextData,
    size_t outputSize) {
  size_t dataSize = plaintextData.size();
  size_t dataWidth = plaintextData.at(0).size();

  encrypter_.prepareToProcessMyData(dataWidth);
  encrypter_.processMyData(plaintextData);

  // 1b. (peer)receive encryted data from peer
  // 2b. (peer)pick desired ciphertext blocks
  // 3a. share key
  std::vector<__m128i> expandedKeyVectorM128i = encrypter_.getExpandedKey();

  UdpDecryption<schedulerId> decrypter(myId_, partnerId_);
  return decrypter.decryptMyData(expandedKeyVectorM128i, dataWidth, outputSize);
}

template <int schedulerId>
typename IDataProcessor<schedulerId>::SecString
DataProcessor<schedulerId>::processPeersData(
    size_t dataSize,
    const std::vector<uint64_t>& indexes,
    size_t dataWidth) {
  encrypter_.prepareToProcessPeerData(dataWidth, indexes);
  encrypter_.processPeerData(dataSize);

  auto [intersection, nonces, _] = encrypter_.getProcessedData();

  UdpDecryption<schedulerId> decrypter(myId_, partnerId_);
  return decrypter.decryptPeerData(intersection, nonces, indexes);
}

} // namespace fbpcf::mpc_std_lib::unified_data_process::data_processor
