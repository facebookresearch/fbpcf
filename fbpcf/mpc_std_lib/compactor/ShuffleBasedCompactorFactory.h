/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/engine/util/util.h"
#include "fbpcf/mpc_std_lib/compactor/ICompactorFactory.h"
#include "fbpcf/mpc_std_lib/compactor/ShuffleBasedCompactor.h"
#include "fbpcf/mpc_std_lib/shuffler/IShufflerFactory.h"
#include "fbpcf/mpc_std_lib/util/util.h"

namespace fbpcf::mpc_std_lib::compactor {

template <typename T, typename LabelT, int schedulerId>
class ShuffleBasedCompactorFactory final
    : public ICompactorFactory<
          typename util::SecBatchType<T, schedulerId>::type,
          typename util::SecBatchType<LabelT, schedulerId>::type> {
 public:
  using SecBatchType =
      typename util::SecBatchType<std::pair<T, LabelT>, schedulerId>::type;
  using SecBatchSrcType = typename util::SecBatchType<T, schedulerId>::type;
  using SecBatchLabelType =
      typename util::SecBatchType<LabelT, schedulerId>::type;
  ShuffleBasedCompactorFactory(
      int myId,
      int partnerId,
      std::unique_ptr<shuffler::IShufflerFactory<SecBatchType>> shufflerFactory)
      : myId_(myId),
        partnerId_(partnerId),
        shufflerFactory_(std::move(shufflerFactory)) {}

  std::unique_ptr<ICompactor<SecBatchSrcType, SecBatchLabelType>> create()
      override {
    return std::make_unique<ShuffleBasedCompactor<T, LabelT, schedulerId>>(
        myId_, partnerId_, shufflerFactory_->create());
  }

 private:
  int myId_;
  int partnerId_;
  std::unique_ptr<shuffler::IShufflerFactory<SecBatchType>> shufflerFactory_;
};

} // namespace fbpcf::mpc_std_lib::compactor
