/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Benchmark.h>

#include "common/init/Init.h"

#include "fbpcf/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/engine/util/test/benchmarks/BenchmarkHelper.h"
#include "fbpcf/engine/util/test/benchmarks/NetworkedBenchmark.h"
#include "fbpcf/frontend/mpcGame.h"
#include "fbpcf/scheduler/IScheduler.h"
#include "fbpcf/scheduler/SchedulerHelper.h"

namespace fbpcf::frontend {

const bool unsafe = true;

template <class Game0, class Game1>
class FrontendBenchmark : public engine::util::NetworkedBenchmark {
 public:
  void setup() override {
    auto [agentFactory0, agentFactory1] =
        engine::util::getSocketAgentFactories();
    agentFactory0_ = std::move(agentFactory0);
    agentFactory1_ = std::move(agentFactory1);
  }

 protected:
  void initSender() override {
    sender_ = std::make_unique<Game0>(
        scheduler::createPlaintextScheduler<unsafe>(0, *agentFactory0_));
  }

  void runSender() override {
    sender_->play();
  }

  void initReceiver() override {
    receiver_ = std::make_unique<Game1>(
        scheduler::createPlaintextScheduler<unsafe>(1, *agentFactory1_));
  }

  void runReceiver() override {
    receiver_->play();
  }

  std::pair<uint64_t, uint64_t> getTrafficStatistics() override {
    return sender_->getTrafficStatistics();
  }

 private:
  std::unique_ptr<engine::communication::IPartyCommunicationAgentFactory>
      agentFactory0_;
  std::unique_ptr<engine::communication::IPartyCommunicationAgentFactory>
      agentFactory1_;

  std::unique_ptr<Game0> sender_;
  std::unique_ptr<Game1> receiver_;
};

template <int schedulerId, bool usingBatch>
class BitGame : public MpcGame<schedulerId> {
 public:
  using SecBit =
      typename frontend::MpcGame<schedulerId>::template SecBit<usingBatch>;

  explicit BitGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : MpcGame<schedulerId>(std::move(scheduler)) {}

  virtual ~BitGame() = default;

  void play() {
    SecBit b1, b2;
    if constexpr (usingBatch) {
      auto batchSize = 1000;
      b1 = SecBit(std::vector<bool>(batchSize, true), 0);
      b2 = SecBit(std::vector<bool>(batchSize, false), 1);
    } else {
      b1 = SecBit(true, 0);
      b2 = SecBit(false, 1);
    }
    for (auto i = 0; i < 100; i++) {
      b2 = operation(b1, b2);
    }
    b2.openToParty(0).getValue();
  }

 protected:
  virtual SecBit operation(SecBit b1, SecBit b2) = 0;
};

template <int schedulerId, bool usingBatch>
class BitAndGame : public BitGame<schedulerId, usingBatch> {
 public:
  explicit BitAndGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : BitGame<schedulerId, usingBatch>(std::move(scheduler)) {}

 protected:
  typename BitGame<schedulerId, usingBatch>::SecBit operation(
      typename BitGame<schedulerId, usingBatch>::SecBit b1,
      typename BitGame<schedulerId, usingBatch>::SecBit b2) override {
    return b1 & b2;
  }
};

BENCHMARK_COUNTERS(BitAndBenchmark, counters) {
  FrontendBenchmark<BitAndGame<0, false>, BitAndGame<1, false>> benchmark;
  benchmark.runBenchmark(counters);
}

BENCHMARK_COUNTERS(BitAndBatchBenchmark, counters) {
  FrontendBenchmark<BitAndGame<0, true>, BitAndGame<1, true>> benchmark;
  benchmark.runBenchmark(counters);
}

template <int schedulerId, bool usingBatch>
class BitXorGame : public BitGame<schedulerId, usingBatch> {
 public:
  explicit BitXorGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : BitGame<schedulerId, usingBatch>(std::move(scheduler)) {}

 protected:
  typename BitGame<schedulerId, usingBatch>::SecBit operation(
      typename BitGame<schedulerId, usingBatch>::SecBit b1,
      typename BitGame<schedulerId, usingBatch>::SecBit b2) override {
    return b1 ^ b2;
  }
};

BENCHMARK_COUNTERS(BitXorBenchmark, counters) {
  FrontendBenchmark<BitXorGame<0, false>, BitXorGame<1, false>> benchmark;
  benchmark.runBenchmark(counters);
}

BENCHMARK_COUNTERS(BitXorBatchBenchmark, counters) {
  FrontendBenchmark<BitXorGame<0, true>, BitXorGame<1, true>> benchmark;
  benchmark.runBenchmark(counters);
}

template <int schedulerId, bool usingBatch>
class BitOrGame : public BitGame<schedulerId, usingBatch> {
 public:
  explicit BitOrGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : BitGame<schedulerId, usingBatch>(std::move(scheduler)) {}

 protected:
  typename BitGame<schedulerId, usingBatch>::SecBit operation(
      typename BitGame<schedulerId, usingBatch>::SecBit b1,
      typename BitGame<schedulerId, usingBatch>::SecBit b2) override {
    return b1 || b2;
  }
};

BENCHMARK_COUNTERS(BitOrBenchmark, counters) {
  FrontendBenchmark<BitOrGame<0, false>, BitOrGame<1, false>> benchmark;
  benchmark.runBenchmark(counters);
}

BENCHMARK_COUNTERS(BitOrBatchBenchmark, counters) {
  FrontendBenchmark<BitOrGame<0, true>, BitOrGame<1, true>> benchmark;
  benchmark.runBenchmark(counters);
}

template <int schedulerId, bool usingBatch>
class BitNotGame : public BitGame<schedulerId, usingBatch> {
 public:
  explicit BitNotGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : BitGame<schedulerId, usingBatch>(std::move(scheduler)) {}

 protected:
  typename BitGame<schedulerId, usingBatch>::SecBit operation(
      typename BitGame<schedulerId, usingBatch>::SecBit b1,
      typename BitGame<schedulerId, usingBatch>::SecBit) override {
    return !b1;
  }
};

BENCHMARK_COUNTERS(BitNotBenchmark, counters) {
  FrontendBenchmark<BitNotGame<0, false>, BitNotGame<1, false>> benchmark;
  benchmark.runBenchmark(counters);
}

BENCHMARK_COUNTERS(BitNotBatchBenchmark, counters) {
  FrontendBenchmark<BitNotGame<0, true>, BitNotGame<1, true>> benchmark;
  benchmark.runBenchmark(counters);
}
} // namespace fbpcf::frontend

int main(int argc, char* argv[]) {
  facebook::initFacebook(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
