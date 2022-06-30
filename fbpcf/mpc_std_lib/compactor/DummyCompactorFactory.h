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

template <typename T, typename LabelT>
class DummyCompactorFactory final : public ICompactorFactory<T, LabelT> {
 public:
  explicit DummyCompactorFactory(int myId, int partnerId)
      : myId_(myId), partnerId_(partnerId) {}

  std::unique_ptr<ICompactor<T, LabelT>> create() override {
    return std::make_unique<DummyCompactor<T, LabelT>>(myId_, partnerId_);
  }

 private:
  int myId_;
  int partnerId_;
};

} // namespace fbpcf::mpc_std_lib::compactor::insecure
