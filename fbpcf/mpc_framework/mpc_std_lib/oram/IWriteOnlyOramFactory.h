/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include "fbpcf/mpc_framework/mpc_std_lib/oram/IWriteOnlyOram.h"

namespace fbpcf::mpc_framework::mpc_std_lib::oram {

template <typename T>
class IWriteOnlyOramFactory {
 public:
  virtual ~IWriteOnlyOramFactory() = default;

  virtual std::unique_ptr<IWriteOnlyOram<T>> create(size_t size) = 0;

  /**
   * Get maximum batch size that can be used without running out of memory.
   */
  virtual uint32_t getMaxBatchSize(size_t size, uint8_t concurrency) = 0;
};

} // namespace fbpcf::mpc_framework::mpc_std_lib::oram
