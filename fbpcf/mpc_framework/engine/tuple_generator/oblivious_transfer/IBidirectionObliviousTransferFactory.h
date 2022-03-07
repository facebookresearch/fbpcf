/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/IBidirectionObliviousTransfer.h"

namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer {

template <class T>
class IBidirectionObliviousTransferFactory {
 public:
  virtual ~IBidirectionObliviousTransferFactory() = default;

  /**
   * Create an oblivious transfer with a particular party.
   * Some implementation may need party id to decide parties' roles in the
   * underlying protocol.
   */
  virtual std::unique_ptr<IBidirectionObliviousTransfer<T>> create(int id) = 0;
};

} // namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer
