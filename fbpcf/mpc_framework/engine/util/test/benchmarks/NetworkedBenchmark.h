/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <future>

#include <folly/Benchmark.h>

namespace fbpcf::mpc_framework::engine::util {

/**
 * A parent class that can be used for writing benchmarks for components that
 * operate over the network.
 */
class NetworkedBenchmark {
 public:
  virtual ~NetworkedBenchmark() = default;

  void runBenchmark(folly::UserCounters& counters) {
    BENCHMARK_SUSPEND {
      setup();
    }

    auto senderTask = std::async([this]() { runSender(); });
    auto receiverTask = std::async([this]() { runReceiver(); });

    senderTask.get();
    receiverTask.get();

    BENCHMARK_SUSPEND {
      auto [sent, received] = getTrafficStatistics();
      counters["transmitted_bytes"] = sent + received;
    }
  }

 protected:
  virtual void setup() = 0;
  virtual void runSender() = 0;
  virtual void runReceiver() = 0;

  virtual std::pair<uint64_t, uint64_t> getTrafficStatistics() = 0;
};

} // namespace fbpcf::mpc_framework::engine::util
