/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/engine/tuple_generator/oblivious_transfer/DummyBidirectionObliviousTransfer.h"
#include <cstdint>

namespace fbpcf::engine::tuple_generator::oblivious_transfer::insecure {

std::vector<bool> DummyBidirectionObliviousTransfer::biDirectionOT(
    const std::vector<bool>& input0,
    const std::vector<bool>& input1,
    const std::vector<bool>& choice) {
  if ((input0.size() != input1.size()) || (input1.size() != choice.size())) {
    throw std::runtime_error("Inconsistent length in inputs");
  }
  std::vector<bool> buffer[2];
  buffer[0] = std::vector<bool>(choice.size());
  buffer[1] = std::vector<bool>(choice.size());

  for (size_t i = 0; i < choice.size(); i++) {
    buffer[0][i] = input0[i];
    buffer[1][i] = input1[i];
  }

  agent_->sendT<bool>(buffer[0]);
  agent_->sendT<bool>(buffer[1]);

  buffer[0] = agent_->receiveT<bool>(choice.size());
  buffer[1] = agent_->receiveT<bool>(choice.size());

  std::vector<bool> rst(choice.size());

  for (size_t i = 0; i < choice.size(); i++) {
    rst[i] = buffer[choice[i]][i];
  }
  return rst;
}

std::vector<uint64_t> DummyBidirectionObliviousTransfer::biDirectionOT(
    const std::vector<uint64_t>& input0,
    const std::vector<uint64_t>& input1,
    const std::vector<bool>& choice) {
  if ((input0.size() != input1.size()) || (input1.size() != choice.size())) {
    throw std::runtime_error("Inconsistent length in inputs");
  }
  std::vector<uint64_t> buffer[2];
  buffer[0] = std::vector<uint64_t>(choice.size());
  buffer[1] = std::vector<uint64_t>(choice.size());

  for (size_t i = 0; i < choice.size(); i++) {
    buffer[0][i] = input0[i];
    buffer[1][i] = input1[i];
  }

  agent_->sendT<uint64_t>(buffer[0]);
  agent_->sendT<uint64_t>(buffer[1]);

  buffer[0] = agent_->receiveT<uint64_t>(choice.size());
  buffer[1] = agent_->receiveT<uint64_t>(choice.size());

  std::vector<uint64_t> rst(choice.size());

  for (size_t i = 0; i < choice.size(); i++) {
    rst[i] = buffer[choice[i]][i];
  }
  return rst;
}

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::insecure
