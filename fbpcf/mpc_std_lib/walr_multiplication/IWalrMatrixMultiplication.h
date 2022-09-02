/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <vector>

#include "fbpcf/frontend/Bit.h"

namespace fbpcf::mpc_std_lib::walr {

template <int schedulerId>
class IWalrMatrixMultiplication {
  using FixedPointType = uint64_t;

 public:
  virtual ~IWalrMatrixMultiplication() = default;
  /**
   * The API for the caller with features and label shares.
   * @param features: the feature matrix. Each element is a column vector of the
   * feature matrix.
   * @param labels: the label vector represented as a batch of Bits,
   * consisting of only (secret) boolean labels.
   * To make the shape compatible, one must have features.size() ==
   * labels.getBatchSize().
   * @return the product of feature matrix and the label vector.
   */
  virtual std::vector<double> matrixVectorMultiplication(
      const std::vector<std::vector<double>>& features,
      const frontend::Bit<true, schedulerId, true>& labels) const = 0;

  /**
   * The API for the caller with only label shares.
   * @param labels: the label vector consisting of only (secret) boolean labels.
   * @param dpNoise: the dp noise that would be imposed on the output.
   */
  virtual void matrixVectorMultiplication(
      const frontend::Bit<true, schedulerId, true>& labels,
      const std::vector<double>& dpNoise) const = 0;
};

} // namespace fbpcf::mpc_std_lib::walr
