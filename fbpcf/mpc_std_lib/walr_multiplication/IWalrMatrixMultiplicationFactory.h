/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/mpc_std_lib/walr_multiplication/IWalrMatrixMultiplication.h"
#include "fbpcf/util/MetricCollector.h"

namespace fbpcf::mpc_std_lib::walr {

template <int schedulerId>
class IWalrMatrixMultiplicationFactory {
 public:
  explicit IWalrMatrixMultiplicationFactory(
      std::shared_ptr<fbpcf::util::MetricCollector> metricCollector)
      : metricCollector_(metricCollector) {}
  virtual ~IWalrMatrixMultiplicationFactory() = default;
  virtual std::unique_ptr<IWalrMatrixMultiplication<schedulerId>> create() = 0;

 protected:
  std::shared_ptr<fbpcf::util::MetricCollector> metricCollector_;
};

} // namespace fbpcf::mpc_std_lib::walr
