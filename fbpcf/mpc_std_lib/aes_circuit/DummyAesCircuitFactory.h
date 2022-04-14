/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/mpc_std_lib/aes_circuit/DummyAesCircuit.h"
#include "fbpcf/mpc_std_lib/aes_circuit/IAesCircuitFactory.h"

namespace fbpcf::mpc_std_lib::aes_circuit::insecure {

template <typename BitType>
class DummyAesCircuitFactory {
 public:
  std::unique_ptr<IAesCircuit<BitType>> create() {
    return std::make_unique<insecure::DummyAesCircuit<BitType>>();
  }

  typename IAesCircuitFactory<BitType>::CircuitType getCircuitType() const {
    return IAesCircuitFactory<BitType>::CircuitType::Dummy;
  }
};

} // namespace fbpcf::mpc_std_lib::aes_circuit::insecure
