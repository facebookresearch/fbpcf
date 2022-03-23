/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/mpc_std_lib/util/util.h"

namespace fbpcf::mpc_std_lib::permuter {

/*
 * A permuter will permute a number of values, and output them in the permuted
 * order. Our permuter is decoupled from the concrete types. As long as a type
 * can perform certain operations/has certain helper functions that depends on
 * concrete implementation (the user may need to implement this method), it
 * should be supported by our permuter.
 */
/**
 * This type T corresponds to a batch of secret-shared values.
 */
template <typename T>
class IPermuter {
 public:
  virtual ~IPermuter() = default;

  /**
   * permute a batch of secret values, the other party will provide the order.
   * @param src the batch of values to permute
   * @param size the size of the batch
   * @return the permuted values in batch
   */
  virtual T permute(const T& src, size_t size) const = 0;

  /**
   * permute a batch of secret values to the provided order.
   * @param src the batch of values to permute
   * @param size the size of the batch
   * @param order the order to permute
   * @return the permuted values in batch
   */
  virtual T permute(
      const T& src,
      size_t size,
      const std::vector<uint32_t>& order) const = 0;
};

} // namespace fbpcf::mpc_std_lib::permuter
