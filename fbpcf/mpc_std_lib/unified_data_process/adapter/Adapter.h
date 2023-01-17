/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/frontend/BitString.h"
#include "fbpcf/mpc_std_lib/shuffler/IShuffler.h"
#include "fbpcf/mpc_std_lib/unified_data_process/adapter/IAdapter.h"

namespace fbpcf::mpc_std_lib::unified_data_process::adapter {

template <int schedulerId>
class Adapter final : public IAdapter {
  using SecBit = frontend::Bit<true, schedulerId, true>;
  using PubBit = frontend::Bit<false, schedulerId, true>;
  using SecString = frontend::BitString<true, schedulerId, true>;

 public:
  Adapter(
      bool amIParty0,
      int32_t party0Id,
      int32_t party1Id,
      std::unique_ptr<shuffler::IShuffler<SecString>> shuffler)
      : amIParty0_(amIParty0),
        party0Id_(party0Id),
        party1Id_(party1Id),
        shuffler_(std::move(shuffler)) {}

  std::vector<int32_t> adapt_impl(
      const std::vector<int32_t>& unionMap) const override;

 private:
  bool amIParty0_;
  int32_t party0Id_;
  int32_t party1Id_;
  std::unique_ptr<shuffler::IShuffler<SecString>> shuffler_;
};

} // namespace fbpcf::mpc_std_lib::unified_data_process::adapter

#include "fbpcf/mpc_std_lib/unified_data_process/adapter/Adapter_impl.h"
