/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <assert.h>

#include "fbpcf/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/DummyBidirectionObliviousTransfer.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/IBidirectionObliviousTransferFactory.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::insecure {

template <class T>
class DummyBidirectionObliviousTransferFactory final
    : public IBidirectionObliviousTransferFactory<T> {
 public:
  explicit DummyBidirectionObliviousTransferFactory(
      communication::IPartyCommunicationAgentFactory& factory)
      : factory_(factory) {}

  std::unique_ptr<IBidirectionObliviousTransfer<T>> create(int id) override {
    return std::make_unique<DummyBidirectionObliviousTransfer<T>>(
        factory_.create(id, "Dummy_Bidirection_OT_traffic"));
  }

 private:
  communication::IPartyCommunicationAgentFactory& factory_;
};

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::insecure
