/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <array>
#include <memory>
#include "fbpcf/mpc_std_lib/aes_circuit/IAesCircuit.h"
#include "fbpcf/mpc_std_lib/aes_circuit/IAesCircuitCtr.h"

namespace fbpcf::mpc_std_lib::aes_circuit {

/*
 * This is the implementation of AES circuit in Counter mode
 */
template <typename BitType>
class AesCircuitCtr final : public IAesCircuitCtr<BitType> {
 public:
  explicit AesCircuitCtr(std::unique_ptr<IAesCircuit<BitType>> AesCircuit)
      : AesCircuit_{std::move(AesCircuit)} {}

 private:
  virtual std::vector<BitType> encrypt_impl(
      const std::vector<BitType>& plaintext,
      const std::vector<BitType>& expandedEncKey,
      const std::vector<BitType>& mask) const override;

  std::unique_ptr<IAesCircuit<BitType>> AesCircuit_;
};

} // namespace fbpcf::mpc_std_lib::aes_circuit

#include "fbpcf/mpc_std_lib/aes_circuit/AesCircuitCtr_impl.h"
