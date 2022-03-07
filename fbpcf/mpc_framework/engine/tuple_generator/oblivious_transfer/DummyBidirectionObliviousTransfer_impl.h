/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer::
    insecure {

template <class T>
std::vector<T> DummyBidirectionObliviousTransfer<T>::biDirectionOT(
    const std::vector<T>& input0,
    const std::vector<T>& input1,
    const std::vector<bool>& choice) {
  if ((input0.size() != input1.size()) || (input1.size() != choice.size())) {
    throw std::runtime_error("Inconsistent length in inputs");
  }
  std::vector<T> buffer[2];
  buffer[0] = std::vector<T>(choice.size());
  buffer[1] = std::vector<T>(choice.size());

  for (int i = 0; i < choice.size(); i++) {
    buffer[0][i] = input0[i];
    buffer[1][i] = input1[i];
  }

  agent_->sendT<T>(buffer[0]);
  agent_->sendT<T>(buffer[1]);

  buffer[0] = agent_->receiveT<T>(choice.size());
  buffer[1] = agent_->receiveT<T>(choice.size());

  std::vector<T> rst(choice.size());

  for (int i = 0; i < choice.size(); i++) {
    rst[i] = buffer[choice[i]][i];
  }
  return rst;
}

} // namespace
  // fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer::insecure
