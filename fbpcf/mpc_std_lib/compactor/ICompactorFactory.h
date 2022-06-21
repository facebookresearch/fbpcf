/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/mpc_std_lib/compactor/ICompactor.h"

namespace fbpcf::mpc_std_lib::compactor {

template <typename T, typename LabelT>
class ICompactorFactory {
 public:
  virtual ~ICompactorFactory() = default;
  virtual std::unique_ptr<ICompactor<T, LabelT>> create() = 0;
};

} // namespace fbpcf::mpc_std_lib::compactor
