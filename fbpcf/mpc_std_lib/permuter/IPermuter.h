/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdexcept>
#include "folly/Format.h"

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
  T permute(const T& src, size_t size) {
    return permute_impl(src, size);
  }

  /**
   * permute a batch of secret values to the provided order.
   * @param src the batch of values to permute
   * @param size the size of the batch
   * @param order the order to permute
   * @return the permuted values in batch
   */
  virtual T
  permute(const T& src, size_t size, const std::vector<uint32_t>& order) {
    if (order.size() != size) {
      throw std::runtime_error(folly::sformat(
          "Permuter called with invalid size. Got size {} but only {} elements in order",
          size,
          order.size()));
    }

    std::vector<bool> seen(order.size(), false);

    for (uint32_t i = 0; i < order.size(); i++) {
      uint32_t permutationIndex = order[i];
      if (permutationIndex < 0 || permutationIndex >= order.size()) {
        throw std::runtime_error(folly::sformat(
            "Permutation index out of bounds at index {} (Got {})",
            i,
            permutationIndex));
      }
      if (seen[permutationIndex]) {
        throw std::runtime_error(folly::sformat(
            "Permutation index seen a second time at index {} (Target index was {})",
            i,
            permutationIndex));
      }

      seen[permutationIndex] = true;
    }

    return permute_impl(src, size, order);
  }

 private:
  virtual T permute_impl(const T& src, size_t size) const = 0;

  virtual T permute_impl(
      const T& src,
      size_t size,
      const std::vector<uint32_t>& order) const = 0;
};

} // namespace fbpcf::mpc_std_lib::permuter
