/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fbpcf/engine/util/util.h>
#include "fbpcf/engine/util/IPrg.h"
#include "fbpcf/mpc_std_lib/permuter/IPermuter.h"
#include "fbpcf/mpc_std_lib/shuffler/IShuffler.h"

namespace fbpcf::mpc_std_lib::shuffler {

/**
 * This shuffler leverages a permuter to shuffle a batch of values
 **/
template <typename T>
class PermuteBasedShuffler final : public IShuffler<T> {
 public:
  PermuteBasedShuffler(
      int myId,
      int partnerId,
      std::unique_ptr<permuter::IPermuter<T>> permuter,
      std::unique_ptr<engine::util::IPrg> prg)
      : myId_(myId),
        partnerId_(partnerId),
        permuter_(std::move(permuter)),
        prg_(std::move(prg)) {}

  T shuffle(const T& src, uint32_t size) const override {
    auto myRandomPermutation = generateRandomPermutation(size);
    if (myId_ < partnerId_) {
      auto tmp = permuter_->permute(src, size, myRandomPermutation);
      auto rst = permuter_->permute(std::move(tmp), size);
      return rst;
    } else {
      auto tmp = permuter_->permute(src, size);
      auto rst = permuter_->permute(std::move(tmp), size, myRandomPermutation);
      return rst;
    }
  }

 private:
  std::vector<uint32_t> generateRandomPermutation(uint32_t size) const {
    std::vector<uint32_t> rst(size);
    for (size_t i = 0; i < size; i++) {
      rst[i] = i;
    }
    for (size_t i = size; i > 1; i--) {
      auto randomBytes = prg_->getRandomBytes(8);
      auto tmp = reinterpret_cast<uint64_t*>(randomBytes.data());
      auto position = (*tmp) % i;
      std::swap(rst[position], rst[i - 1]);
    }
    return rst;
  }

  int myId_;
  int partnerId_;
  std::unique_ptr<permuter::IPermuter<T>> permuter_;
  std::unique_ptr<engine::util::IPrg> prg_;
};

} // namespace fbpcf::mpc_std_lib::shuffler
