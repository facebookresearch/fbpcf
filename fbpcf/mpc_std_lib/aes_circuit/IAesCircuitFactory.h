/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include "fbpcf/mpc_std_lib/aes_circuit/IAesCircuit.h"

namespace fbpcf::mpc_std_lib::aes_circuit {

template <typename BitType>
class IAesCircuitFactory {
 public:
  virtual ~IAesCircuitFactory() = default;
  virtual std::unique_ptr<IAesCircuit<BitType>> create() = 0;

  enum CircuitType {
    Dummy,
    Secure,
  };

  virtual CircuitType getCircuitType() const = 0;
};

} // namespace fbpcf::mpc_std_lib::aes_circuit
