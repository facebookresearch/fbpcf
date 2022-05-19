/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <openssl/ec.h>
#include <openssl/evp.h>
#include <functional>

#include "fbpcf/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/IBaseObliviousTransfer.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer {

/**
 * This is a NP(NP stands for the authors' names - Naor and Pinkas) base
 * oblivious transfer object. This is an implementation for paper 'Efficient
 * Oblivious Transfer Protocols' by Moni Naor and Benny Pinkas. Link to the
 * paper:
 * https://github.com/isislovecruft/library--/blob/master/cryptography%20%26%20mathematics/oblivious%20transfer/Efficient%20Oblivious%20Transfer%20Protocols%20(2001)%20-%20Naor%2C%20Pinkas.pdf
 */
class NpBaseObliviousTransfer : public IBaseObliviousTransfer {
 public:
  explicit NpBaseObliviousTransfer(
      std::unique_ptr<communication::IPartyCommunicationAgent> agent);

  /**
   * @inherit doc
   */
  std::pair<std::vector<__m128i>, std::vector<__m128i>> send(
      size_t size) override;

  /**
   * @inherit doc
   */
  std::vector<__m128i> receive(const std::vector<bool>& choice) override;

  /**
   * @inherit doc
   */
  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    return agent_->getTrafficStatistics();
  }

  /**
   * @inherit doc
   */
  std::unique_ptr<communication::IPartyCommunicationAgent>
  extractCommunicationAgent() override {
    return std::move(agent_);
  }

 protected:
  using PointPointer =
      std::unique_ptr<EC_POINT, std::function<void(EC_POINT*)>>;

  void sendPoint(const EC_POINT& point) const;
  PointPointer receivePoint() const;

  PointPointer generateRandomPoint() const;

  __m128i hashPoint(const EC_POINT& point, uint64_t nonce) const;

  std::unique_ptr<communication::IPartyCommunicationAgent> agent_;

  std::unique_ptr<EC_GROUP, std::function<void(EC_GROUP*)>> group_;
  std::unique_ptr<BIGNUM, std::function<void(BIGNUM*)>> order_;
};

} // namespace fbpcf::engine::tuple_generator::oblivious_transfer
