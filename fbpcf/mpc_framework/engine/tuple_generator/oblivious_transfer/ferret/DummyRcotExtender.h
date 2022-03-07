/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/IRcotExtender.h"

namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer::
    ferret::insecure {

/**
 * This is  a dummy Rcot extender. It won't do anything but simply copy-paste
 * the inputs.
 */
class DummyRcotExtender final : public IRcotExtender {
 public:
  /**
   * @inherit doc
   */
  int senderInit(
      __m128i delta,
      int64_t extendedSize,
      int64_t baseSize,
      int64_t weight) override;

  /**
   * @inherit doc
   */
  int receiverInit(int64_t extendedSize, int64_t baseSize, int64_t weight)
      override;

  /**
   * @inherit doc
   */
  void setCommunicationAgent(
      std::unique_ptr<communication::IPartyCommunicationAgent> /*agent*/)
      override {}

  /**
   * @inherit doc
   */
  int getBaseCotSize() const {
    return baseSize_;
  }

  /**
   * @inherit doc
   */
  std::vector<__m128i> senderExtendRcot(
      std::vector<__m128i>&& baseRcot) override;

  /**
   * @inherit doc
   */
  std::vector<__m128i> receiverExtendRcot(
      std::vector<__m128i>&& baseRcot) override;

  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    return {0, 0};
  }

 private:
  util::Role role_;
  int64_t extendedSize_;
  int64_t baseSize_;
};

} // namespace
  // fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer::ferret::insecure
