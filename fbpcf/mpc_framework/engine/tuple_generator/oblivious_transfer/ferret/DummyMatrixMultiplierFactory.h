/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/DummyMatrixMultiplier.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/IMatrixMultiplierFactory.h"

namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer::
    ferret::insecure {

class DummyMatrixMultiplierFactory final : public IMatrixMultiplierFactory {
 public:
  std::unique_ptr<IMatrixMultiplier> create() override {
    return std::make_unique<DummyMatrixMultiplier>();
  }
};

} // namespace
  // fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer::ferret::insecure
