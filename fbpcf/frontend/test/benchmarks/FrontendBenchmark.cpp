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
        scheduler::createLazySchedulerWithInsecureEngine<unsafe>(
            0, *agentFactory0_));
  }

  void runSender() override {
    sender_->play();
  }

  void initReceiver() override {
    receiver_ = std::make_unique<Game1>(
        scheduler::createLazySchedulerWithInsecureEngine<unsafe>(
            1, *agentFactory1_));
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

template <int schedulerId>
class BitGame : public MpcGame<schedulerId> {
 public:
  using SecBit =
      typename frontend::MpcGame<schedulerId>::template SecBit<false>;

  explicit BitGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : MpcGame<schedulerId>(std::move(scheduler)) {}

  virtual ~BitGame() = default;

  void play() {
    SecBit b1(true, 0);
    SecBit b2(false, 1);
    for (auto i = 0; i < 100; i++) {
      b2 = operation(b1, b2);
    }
    b2.openToParty(0).getValue();
  }

 protected:
  virtual SecBit operation(SecBit b1, SecBit b2) = 0;
};

template <int schedulerId>
class BitAndGame : public BitGame<schedulerId> {
 public:
  explicit BitAndGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : BitGame<schedulerId>(std::move(scheduler)) {}

 protected:
  typename BitGame<schedulerId>::SecBit operation(
      typename BitGame<schedulerId>::SecBit b1,
      typename BitGame<schedulerId>::SecBit b2) override {
    return b1 & b2;
  }
};

BENCHMARK_COUNTERS(BitAndBenchmark, counters) {
  FrontendBenchmark<BitAndGame<0>, BitAndGame<1>> benchmark;
  benchmark.runBenchmark(counters);
}

template <int schedulerId>
class BitXorGame : public BitGame<schedulerId> {
 public:
  explicit BitXorGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : BitGame<schedulerId>(std::move(scheduler)) {}

 protected:
  typename BitGame<schedulerId>::SecBit operation(
      typename BitGame<schedulerId>::SecBit b1,
      typename BitGame<schedulerId>::SecBit b2) override {
    return b1 ^ b2;
  }
};

BENCHMARK_COUNTERS(BitXorBenchmark, counters) {
  FrontendBenchmark<BitXorGame<0>, BitXorGame<1>> benchmark;
  benchmark.runBenchmark(counters);
}

template <int schedulerId>
class BitOrGame : public BitGame<schedulerId> {
 public:
  explicit BitOrGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : BitGame<schedulerId>(std::move(scheduler)) {}

 protected:
  typename BitGame<schedulerId>::SecBit operation(
      typename BitGame<schedulerId>::SecBit b1,
      typename BitGame<schedulerId>::SecBit b2) override {
    return b1 || b2;
  }
};

BENCHMARK_COUNTERS(BitOrBenchmark, counters) {
  FrontendBenchmark<BitOrGame<0>, BitOrGame<1>> benchmark;
  benchmark.runBenchmark(counters);
}

template <int schedulerId>
class BitNotGame : public BitGame<schedulerId> {
 public:
  explicit BitNotGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : BitGame<schedulerId>(std::move(scheduler)) {}

 protected:
  typename BitGame<schedulerId>::SecBit operation(
      typename BitGame<schedulerId>::SecBit b1,
      typename BitGame<schedulerId>::SecBit) override {
    return !b1;
  }
};

BENCHMARK_COUNTERS(BitNotBenchmark, counters) {
  FrontendBenchmark<BitNotGame<0>, BitNotGame<1>> benchmark;
  benchmark.runBenchmark(counters);
}
} // namespace fbpcf::frontend

int main(int argc, char* argv[]) {
  facebook::initFacebook(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
