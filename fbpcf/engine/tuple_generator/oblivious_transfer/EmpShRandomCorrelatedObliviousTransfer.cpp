/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <emmintrin.h>

#include "fbpcf/engine/tuple_generator/oblivious_transfer/EmpShRandomCorrelatedObliviousTransfer.h"
#include "fbpcf/engine/util/EmpNetworkAdapter.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer {

EmpShRandomCorrelatedObliviousTransfer::EmpShRandomCorrelatedObliviousTransfer(
    __m128i delta,
    std::unique_ptr<communication::IPartyCommunicationAgent> agent)
    : agent_(std::move(agent)) {
  io_ = std::make_unique<util::EmpNetworkAdapter>(*agent_);

  shot_ = std::make_unique<emp::IKNP<util::EmpNetworkAdapter>>(io_.get());
  bool delta_bool[128];
  block_to_bool(delta_bool, delta);
  shot_->setup_send(delta_bool);

  role_ = util::Role::sender;
}

EmpShRandomCorrelatedObliviousTransfer::EmpShRandomCorrelatedObliviousTransfer(
    std::unique_ptr<communication::IPartyCommunicationAgent> agent,
    std::unique_ptr<util::IPrg> prg)
    : agent_(std::move(agent)), prg_(std::move(prg)) {
  io_ = std::make_unique<util::EmpNetworkAdapter>(*agent_);

  shot_ = std::make_unique<emp::IKNP<util::EmpNetworkAdapter>>(io_.get());
  shot_->setup_recv();

  role_ = util::Role::receiver;
}

std::vector<__m128i> EmpShRandomCorrelatedObliviousTransfer::rcot(
    int64_t size) {
  std::vector<__m128i> rst(size);
  switch (role_) {
    case util::Role::sender: {
      shot_->send_cot(rst.data(), size);
      for (size_t i = 0; i < rst.size(); i++) {
        util::setLsbTo0(rst[i]);
      }
      return rst;
    }
    case util::Role::receiver: {
      auto choice = prg_->getRandomBits(size);
      bool* b = new bool[size];
      for (int i = 0; i < size; i++) {
        b[i] = choice[i];
      }
      shot_->recv_cot(rst.data(), b, size);
      for (size_t i = 0; i < rst.size(); i++) {
        if (b[i] == 0) {
          util::setLsbTo0(rst[i]);
        } else {
          util::setLsbTo1(rst[i]);
        }
      }
      delete[] b;
      return rst;
    }
  }
}

} // namespace fbpcf::engine::tuple_generator::oblivious_transfer
