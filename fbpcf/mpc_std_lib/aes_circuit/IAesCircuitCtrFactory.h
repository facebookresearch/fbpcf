/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include "fbpcf/mpc_std_lib/aes_circuit/IAesCircuitCtr.h"
#include "fbpcf/mpc_std_lib/aes_circuit/IAesCircuitFactory.h"

namespace fbpcf::mpc_std_lib::aes_circuit {

template <typename BitType>
class IAesCircuitCtrFactory {
 public:
  virtual ~IAesCircuitCtrFactory() = default;

  virtual std::unique_ptr<IAesCircuitCtr<BitType>> create() = 0;

  virtual typename IAesCircuitFactory<BitType>::CircuitType getCircuitType()
      const = 0;
};

} // namespace fbpcf::mpc_std_lib::aes_circuit
