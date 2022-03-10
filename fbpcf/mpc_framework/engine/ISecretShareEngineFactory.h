/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include "fbpcf/mpc_framework/engine/ISecretShareEngine.h"

namespace fbpcf::engine {

/**
 * This factory creates secret share MPC engine
 */
class ISecretShareEngineFactory {
 public:
  virtual ~ISecretShareEngineFactory() = default;

  virtual std::unique_ptr<ISecretShareEngine> create() = 0;
};

} // namespace fbpcf::engine
