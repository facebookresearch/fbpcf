/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/mpc_std_lib/walr_multiplication/IWalrMatrixMultiplication.h"
namespace fbpcf::mpc_std_lib::walr {

template <int schedulerId>
class IWalrMatrixMultiplicationFactory {
 public:
  virtual ~IWalrMatrixMultiplicationFactory() = default;
  virtual std::unique_ptr<IWalrMatrixMultiplication<schedulerId>> create() = 0;
};

} // namespace fbpcf::mpc_std_lib::walr
