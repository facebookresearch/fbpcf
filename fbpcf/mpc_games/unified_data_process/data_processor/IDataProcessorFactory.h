/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include "fbpcf/mpc_games/unified_data_process/data_processor/IDataProcessor.h"

namespace fbpcf::mpc_games::udp::data_processor {

template <int schedulerId>
class IDataProcessorFactory {
 public:
  virtual ~IDataProcessorFactory() = default;
  virtual std::unique_ptr<IDataProcessor<schedulerId>> create() = 0;
};

} // namespace fbpcf::mpc_games::udp::data_processor
