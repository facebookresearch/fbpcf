/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <emmintrin.h>
#include <emp-tool/emp-tool.h>
#include <vector>

#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgent.h"

namespace fbpcf::engine::util {
// this object wrapps around our own network object and provides the same API as
// emp-tool::io

class EmpNetworkAdapter {
 public:
  explicit EmpNetworkAdapter(communication::IPartyCommunicationAgent& agent)
      : agent_(agent) {}

  void send_data(const void* data, int nByte) {
    std::vector<unsigned char> buffer(nByte);
    memcpy(buffer.data(), data, nByte);
    agent_.send(buffer);
  }

  void recv_data(void* data, int nByte) {
    auto buffer = agent_.receive(nByte);
    memcpy(data, buffer.data(), nByte);
  }

  void send_block(const __m128i* data, int nBlock) {
    send_data(data, nBlock * sizeof(__m128i));
  }

  void recv_block(__m128i* data, int nBlock) {
    recv_data(data, nBlock * sizeof(__m128i));
  }

  void send_pt(emp::Point* A, int num_pts = 1) {
    for (int i = 0; i < num_pts; ++i) {
      size_t len = A[i].size();
      A[i].group->resize_scratch(len);
      unsigned char* tmp = A[i].group->scratch;
      send_data(&len, sizeof(size_t));
      A[i].to_bin(tmp, len);
      send_data(tmp, len);
    }
  }

  void recv_pt(emp::Group* g, emp::Point* A, int num_pts = 1) {
    size_t len = 0;
    for (int i = 0; i < num_pts; ++i) {
      recv_data(&len, sizeof(size_t));
      g->resize_scratch(len);
      unsigned char* tmp = g->scratch;
      recv_data(tmp, len);
      A[i].from_bin(g, tmp, len);
    }
  }

  void flush() {}

  std::pair<uint64_t, uint64_t> getTrafficStatistics() const {
    // returning {0, 0} because this object doesn't own agent_
    return {0, 0};
  }

 private:
  communication::IPartyCommunicationAgent& agent_;
};

} // namespace fbpcf::engine::util
