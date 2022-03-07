/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/mpc_games/unified_data_process/adapter/IAdapter.h"

namespace fbpcf::mpc_games::udp::adapter {

class IAdapterFactory {
 public:
  virtual ~IAdapterFactory() = default;
  virtual std::unique_ptr<IAdapter> create() = 0;
};

} // namespace fbpcf::mpc_games::udp::adapter
