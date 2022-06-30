/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include "fbpcf/mpc_std_lib/util/util.h"
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
 * compactified, e.g. std::vector.
 * Similarly, a type LabelT corresponds to a set of binary labels (i.e., a
 * batch), e.g. std::vector.
 */
template <typename T, typename LabelT>
class ICompactor {
 public:
  virtual ~ICompactor() = default;

  /**
   * Perform a compaction on a set of secret values based on given binary
   * labels.
   * @param src: a set of n values to be compactified
   * @param label: a set of n binary labels, where 1 indicates the
   * corresponding items in the src are considered as necessary data;
   * otherwise they are considered as unimportant data
   * @param shouldRevealSize: whether it is okay to reveal the size of 1s items
   * @return the resulting compactified src and label
   */

  virtual std::pair<T, LabelT> compaction(
      const T& src,
      const LabelT& label,
      bool shouldRevealSize) const = 0;
};

} // namespace fbpcf::mpc_std_lib::compactor
