/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

namespace fbpcf::mpc_std_lib::compactor {

/*
 * A compactor will compactify a set of n items based on assigned binary class
 * labels to select all 1s items. If it is allowed to reveal the number of 1s
 * items, r, our compaction outputs compactified items of size r. Otherwise, it
 * just runs a compaction algorithm and returns
 * the resulting compactified items of size n (same as the input size) without
 * discarding any items.
 */
/**
 * A type T corresponds to a set of values (i.e., a batch) to be
 * compactified, e.g. std::vector
 * A type LabelT corresponds to a binary label, e.g., bool, Bit
 */
template <typename T, typename LabelT>
class ICompactor {
 public:
  virtual ~ICompactor() = default;

  /**
   * Perform a compaction on a set of batches based on specified binary labels.
   * @param batches: a set of batches, where every batch is composed of
   * n values to be compactified
   * @param labels: a binary vector of size n where 1 indicates the
   * corresponding items in the input are considered as necessary data;
   * otherwise they are considered as unimportant data
   * @param shouldRevealSize: whether it is okay to reveal the size of 1s items
   * @return the resulting compactified metadata and labels
   */

  virtual std::tuple<std::vector<T>, std::vector<LabelT>> compaction(
      const std::vector<T>& batches,
      const std::vector<LabelT>& labels,
      bool shouldRevealSize) const = 0;
};

} // namespace fbpcf::mpc_std_lib::compactor
