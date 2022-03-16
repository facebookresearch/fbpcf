/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <future>

#include <folly/Benchmark.h>

namespace fbpcf::engine::util {

/**
 * A parent class that can be used for writing benchmarks for components that
 * operate over the network.
 */
class NetworkedBenchmark {
 public:
  virtual ~NetworkedBenchmark() = default;

  void runBenchmark(folly::UserCounters& counters) {
    uint64_t initTransmittedBytes;
    BENCHMARK_SUSPEND {
      setup();

      auto start = std::chrono::high_resolution_clock::now();

      auto initSenderTask = std::async([this]() { initSender(); });
      auto initReceiverTask = std::async([this]() { initReceiver(); });

      initSenderTask.get();
      initReceiverTask.get();

      auto end = std::chrono::high_resolution_clock::now();
      counters["init_time_usec"] =
          std::chrono::duration_cast<std::chrono::microseconds>(end - start)
              .count();

      auto [sent, received] = getTrafficStatistics();
      initTransmittedBytes = sent + received;
      counters["init_transmitted_bytes"] = initTransmittedBytes;
    }

    auto senderTask = std::async([this]() { runSender(); });
    auto receiverTask = std::async([this]() { runReceiver(); });

    senderTask.get();
    receiverTask.get();

    BENCHMARK_SUSPEND {
      auto [sent, received] = getTrafficStatistics();
      counters["transmitted_bytes"] = sent + received - initTransmittedBytes;
    }
  }

 protected:
  virtual void setup() = 0;

  virtual void initSender() = 0;
  virtual void initReceiver() = 0;

  virtual void runSender() = 0;
  virtual void runReceiver() = 0;

  virtual std::pair<uint64_t, uint64_t> getTrafficStatistics() = 0;
};

} // namespace fbpcf::engine::util
