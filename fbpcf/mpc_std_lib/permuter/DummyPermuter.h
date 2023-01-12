/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "fbpcf/mpc_std_lib/permuter/IPermuter.h"

#include "fbpcf/mpc_std_lib/util/util.h"

namespace fbpcf::mpc_std_lib::permuter::insecure {

/**
 * This permuter doesn't do anything but simply output the input. It is only
 * meant to be used as a placeholder in tests.
 **/
template <typename T, int schedulerId>
class DummyPermuter final
    : public IPermuter<typename util::SecBatchType<T, schedulerId>::type> {
 public:
  using SecBatchType = typename util::SecBatchType<T, schedulerId>::type;
  DummyPermuter(int myId, int partnerId) : myId_(myId), partnerId_(partnerId) {}

  SecBatchType permute_impl(const SecBatchType& src, size_t size)
      const override {
    std::vector<SecBatchType> unbatched =
        util::MpcAdapters<T, schedulerId>::unbatching(
            src, std::make_shared<std::vector<uint32_t>>(size, 1));
    std::vector<std::vector<T>> placeHolders(size);
    for (int i = 0; i < size; i++) {
      placeHolders[i] = util::MpcAdapters<T, schedulerId>::openToParty(
          unbatched[i], partnerId_);
    }

    std::vector<SecBatchType> permuted(size);
    for (size_t i = 0; i < size; i++) {
      permuted[i] = util::MpcAdapters<T, schedulerId>::processSecretInputs(
          placeHolders[i], partnerId_);
    }

    auto front = permuted[0];
    permuted.erase(permuted.begin());
    return util::MpcAdapters<T, schedulerId>::batchingWith(front, permuted);
  }

  SecBatchType permute_impl(
      const SecBatchType& src,
      size_t size,
      const std::vector<uint32_t>& order) const override {
    std::vector<SecBatchType> unbatched =
        util::MpcAdapters<T, schedulerId>::unbatching(
            src, std::make_shared<std::vector<uint32_t>>(size, 1));
    std::vector<std::vector<T>> plaintext(size);
    for (int i = 0; i < size; i++) {
      plaintext[i] =
          util::MpcAdapters<T, schedulerId>::openToParty(unbatched[i], myId_);
    }

    std::vector<SecBatchType> permuted(size);
    for (size_t i = 0; i < size; i++) {
      permuted[i] = util::MpcAdapters<T, schedulerId>::processSecretInputs(
          plaintext.at(order.at(i)), myId_);
    }

    auto front = permuted[0];
    permuted.erase(permuted.begin());
    return util::MpcAdapters<T, schedulerId>::batchingWith(front, permuted);
  }

 private:
  int myId_;
  int partnerId_;
};

} // namespace fbpcf::mpc_std_lib::permuter::insecure
