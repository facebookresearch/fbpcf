/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include "fbpcf/mpc_framework/mpc_std_lib/oram/ISinglePointArrayGenerator.h"

namespace fbpcf::mpc_framework::mpc_std_lib::oram {

class ISinglePointArrayGeneratorFactory {
 public:
  virtual ~ISinglePointArrayGeneratorFactory() = default;

  virtual std::unique_ptr<ISinglePointArrayGenerator> create() = 0;
};

} // namespace fbpcf::mpc_framework::mpc_std_lib::oram
