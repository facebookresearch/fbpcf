/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/IMatrixMultiplier.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::ferret {

class IMatrixMultiplierFactory {
 public:
  virtual ~IMatrixMultiplierFactory() = default;
  virtual std::unique_ptr<IMatrixMultiplier> create() = 0;
};

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::ferret
