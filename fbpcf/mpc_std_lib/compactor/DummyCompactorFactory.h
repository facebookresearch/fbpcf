/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/mpc_std_lib/compactor/DummyCompactor.h"
#include "fbpcf/mpc_std_lib/compactor/ICompactorFactory.h"

namespace fbpcf::mpc_std_lib::compactor::insecure {

template <typename T, typename LabelT, int schedulerId>
class DummyCompactorFactory final
    : public ICompactorFactory<
          typename util::SecBatchType<T, schedulerId>::type,
          typename util::SecBatchType<LabelT, schedulerId>::type> {
 public:
  using SecBatchType = typename util::SecBatchType<T, schedulerId>::type;
  using SecBatchLabelType =
      typename util::SecBatchType<LabelT, schedulerId>::type;
  explicit DummyCompactorFactory(int myId, int partnerId)
      : myId_(myId), partnerId_(partnerId) {}

  std::unique_ptr<ICompactor<SecBatchType, SecBatchLabelType>> create()
      override {
    return std::make_unique<DummyCompactor<T, LabelT, schedulerId>>(
        myId_, partnerId_);
  }

 private:
  int myId_;
  int partnerId_;
};

} // namespace fbpcf::mpc_std_lib::compactor::insecure
