/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/mpc_std_lib/aes_circuit/AesCircuit.h"
#include "fbpcf/mpc_std_lib/aes_circuit/AesCircuitCtr.h"
#include "fbpcf/mpc_std_lib/aes_circuit/IAesCircuitCtrFactory.h"
#include "fbpcf/mpc_std_lib/aes_circuit/IAesCircuitFactory.h"

namespace fbpcf::mpc_std_lib::aes_circuit {

template <typename BitType>
class AesCircuitCtrFactory : IAesCircuitCtrFactory<BitType> {
 public:
  std::unique_ptr<IAesCircuitCtr<BitType>> create() override {
    return std::make_unique<AesCircuitCtr<BitType>>(
        std::make_unique<AesCircuit<BitType>>());
  }

  typename IAesCircuitFactory<BitType>::CircuitType getCircuitType()
      const override {
    return IAesCircuitFactory<BitType>::CircuitType::Secure;
  }
};

} // namespace fbpcf::mpc_std_lib::aes_circuit
