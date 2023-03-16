/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emmintrin.h>
#include <cstddef>
#include <vector>

namespace fbpcf::mpc_std_lib::unified_data_process::data_processor {

/**
 * This is merely an interface to accommodate mocking for test.
 **/
class IUdpEncryption {
 public:
  virtual ~IUdpEncryption() = default;

  virtual void prepareToProcessMyData(size_t myDataWidth) = 0;

  /**
   * Process my data via UDP encryption. This API should be called in coordinate
   * with "ProcessPeerData" on peer's side. If this API is ever called, calling
   * "getExpandedKey" to retrieve the expanded key for decryption later.
   */
  virtual void processMyData(
      const std::vector<std::vector<unsigned char>>& plaintextData) = 0;

  virtual std::vector<__m128i> getExpandedKey() = 0;

  virtual void prepareToProcessPeerData(
      size_t peerDataWidth,
      const std::vector<int32_t>& indexes) = 0;

  /*
   * process peer data via UDP encryption. This API should be called in
   * coordinate with "ProcessMyData" on peer's side. This API is ever
   * called, calling "getProcessedData" to retrive the cherry-picked
   * encryption later.
   */
  virtual void processPeerData(size_t dataSize) = 0;

  struct EncryptionResults {
    std::vector<std::vector<unsigned char>> ciphertexts;
    std::vector<__m128i> nonces;
    std::vector<int32_t> indexes;
  };

  // temporary, avoiding break fbpcs.
  using EncryptionResuts = EncryptionResults;

  // returning the ciphertext, nonce, and index of cherry-picked rows
  virtual EncryptionResults getProcessedData() = 0;
};

} // namespace fbpcf::mpc_std_lib::unified_data_process::data_processor
