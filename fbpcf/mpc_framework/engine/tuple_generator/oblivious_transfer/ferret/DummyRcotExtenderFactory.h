/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/DummyRcotExtender.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/IRcotExtenderFactory.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::ferret::insecure {

class DummyRcotExtenderFactory final : public IRcotExtenderFactory {
 public:
  std::unique_ptr<IRcotExtender> create() override {
    return std::make_unique<DummyRcotExtender>();
  }
};

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::ferret::insecure
