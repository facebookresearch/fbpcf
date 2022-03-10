/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgent.h"
#include "fbpcf/mpc_framework/mpc_std_lib/oram/IDifferenceCalculator.h"

namespace fbpcf::mpc_std_lib::oram {

template <typename T>
class IDifferenceCalculatorFactory {
 public:
  virtual ~IDifferenceCalculatorFactory() = default;

  virtual std::unique_ptr<IDifferenceCalculator<T>> create() = 0;
};

} // namespace fbpcf::mpc_std_lib::oram
