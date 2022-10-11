/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/mpc_std_lib/util/util.h"

namespace fbpcf::mpc_std_lib::shuffler {

/*
 * A shuffler will shuffle a number of values, and output them in
 * a shuffled order.
 * Our shuffler is decoupled from the concrete types. As long as a type can
 * perform certain operations/has certain helper functions that depends on
 * concrete implementation (the user may need to implement this method), it
 * should be supported by our shuffler.
 */
/**
 * This type T corresponds to a set of values, e.g. std::vector
 */
template <typename T>
class IShuffler {
 public:
  virtual ~IShuffler() = default;

  /**
   * shuffle a batch of secret values.
   * @param src the batch of values to shuffle
   * @param size the size of the batch
   * @return the shuffled values in batch
   */
  virtual T shuffle(const T& src, uint32_t size) const = 0;
};

} // namespace fbpcf::mpc_std_lib::shuffler
