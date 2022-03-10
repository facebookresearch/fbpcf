/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <emmintrin.h>
#include <functional>
#include <memory>
#include <vector>
#include "fbpcf/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/IRandomCorrelatedObliviousTransfer.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer {

class IRandomCorrelatedObliviousTransferFactory {
 public:
  virtual ~IRandomCorrelatedObliviousTransferFactory() = default;

  /**
   * Construct sender
   */
  virtual std::unique_ptr<IRandomCorrelatedObliviousTransfer> create(
      __m128i delta,
      std::unique_ptr<communication::IPartyCommunicationAgent> agent) = 0;

  /**
   * Construct receiver
   */
  virtual std::unique_ptr<IRandomCorrelatedObliviousTransfer> create(
      std::unique_ptr<communication::IPartyCommunicationAgent> agent) = 0;
};

} // namespace fbpcf::engine::tuple_generator::oblivious_transfer
