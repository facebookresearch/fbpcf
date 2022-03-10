/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/IMatrixMultiplierFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/TenLocalLinearMatrixMultiplier.h"
#include "fbpcf/mpc_framework/engine/util/AesPrgFactory.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::ferret {

class TenLocalLinearMatrixMultiplierFactory final
    : public IMatrixMultiplierFactory {
 public:
  std::unique_ptr<IMatrixMultiplier> create() override {
    return std::make_unique<TenLocalLinearMatrixMultiplier>();
  }
};

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::ferret
