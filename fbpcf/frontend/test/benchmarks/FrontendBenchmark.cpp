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

template <int schedulerId, bool isBoolOutput>
class IntGame : public MpcGame<schedulerId> {
 public:
  using SecSignedInt =
      typename frontend::MpcGame<schedulerId>::template SecSignedInt<32, false>;
  using SecBit =
      typename frontend::MpcGame<schedulerId>::template SecBit<false>;

  using TOutput =
      typename std::conditional<isBoolOutput, SecBit, SecSignedInt>::type;

  explicit IntGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : MpcGame<schedulerId>(std::move(scheduler)) {}

  virtual ~IntGame() = default;

  void play() {
    SecSignedInt b1(static_cast<int32_t>(folly::Random::rand32()), 0);
    SecSignedInt b2(static_cast<int32_t>(folly::Random::rand32()), 1);
    TOutput b3;
    for (auto i = 0; i < 100; i++) {
      b3 = operation(b1, b2);
    }
    b2.openToParty(0).getValue();
  }

 protected:
  virtual TOutput operation(SecSignedInt b1, SecSignedInt b2) = 0;
};

template <int schedulerId>
class IntAddGame : public IntGame<schedulerId, false> {
 public:
  explicit IntAddGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : IntGame<schedulerId, false>(std::move(scheduler)) {}

 protected:
  typename IntGame<schedulerId, false>::SecSignedInt operation(
      typename IntGame<schedulerId, false>::SecSignedInt b1,
      typename IntGame<schedulerId, false>::SecSignedInt b2) override {
    return b1 + b2;
  }
};

BENCHMARK_COUNTERS(IntAddBenchmark, counters) {
  FrontendBenchmark<IntAddGame<0>, IntAddGame<1>> benchmark;
  benchmark.runBenchmark(counters);
}

template <int schedulerId>
class IntSubtractGame : public IntGame<schedulerId, false> {
 public:
  explicit IntSubtractGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : IntGame<schedulerId, false>(std::move(scheduler)) {}

 protected:
  typename IntGame<schedulerId, false>::SecSignedInt operation(
      typename IntGame<schedulerId, false>::SecSignedInt b1,
      typename IntGame<schedulerId, false>::SecSignedInt b2) override {
    return b1 - b2;
  }
};

BENCHMARK_COUNTERS(IntSubtractBenchmark, counters) {
  FrontendBenchmark<IntSubtractGame<0>, IntSubtractGame<1>> benchmark;
  benchmark.runBenchmark(counters);
}

template <int schedulerId>
class IntMuxGame : public IntGame<schedulerId, false> {
 public:
  explicit IntMuxGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : IntGame<schedulerId, false>(std::move(scheduler)) {}

 protected:
  typename IntGame<schedulerId, false>::SecSignedInt operation(
      typename IntGame<schedulerId, false>::SecSignedInt b1,
      typename IntGame<schedulerId, false>::SecSignedInt b2) override {
    typename IntGame<schedulerId, false>::SecBit choice(true, 1);
    return b1.mux(choice, b2);
  }
};

BENCHMARK_COUNTERS(IntMuxBenchmark, counters) {
  FrontendBenchmark<IntMuxGame<0>, IntMuxGame<1>> benchmark;
  benchmark.runBenchmark(counters);
}

template <int schedulerId>
class IntLessThanGame : public IntGame<schedulerId, true> {
 public:
  explicit IntLessThanGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : IntGame<schedulerId, true>(std::move(scheduler)) {}

 protected:
  typename IntGame<schedulerId, true>::SecBit operation(
      typename IntGame<schedulerId, true>::SecSignedInt b1,
      typename IntGame<schedulerId, true>::SecSignedInt b2) override {
    return b1 < b2;
  }
};

BENCHMARK_COUNTERS(IntLessThanBenchmark, counters) {
  FrontendBenchmark<IntLessThanGame<0>, IntLessThanGame<1>> benchmark;
  benchmark.runBenchmark(counters);
}

template <int schedulerId>
class IntLessThanOrEqualToGame : public IntGame<schedulerId, true> {
 public:
  explicit IntLessThanOrEqualToGame(
      std::unique_ptr<scheduler::IScheduler> scheduler)
      : IntGame<schedulerId, true>(std::move(scheduler)) {}

 protected:
  typename IntGame<schedulerId, true>::SecBit operation(
      typename IntGame<schedulerId, true>::SecSignedInt b1,
      typename IntGame<schedulerId, true>::SecSignedInt b2) override {
    return b1 <= b2;
  }
};

BENCHMARK_COUNTERS(IntLessThanOrEqualToBenchmark, counters) {
  FrontendBenchmark<IntLessThanOrEqualToGame<0>, IntLessThanOrEqualToGame<1>>
      benchmark;
  benchmark.runBenchmark(counters);
}

template <int schedulerId>
class IntGreaterThanGame : public IntGame<schedulerId, true> {
 public:
  explicit IntGreaterThanGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : IntGame<schedulerId, true>(std::move(scheduler)) {}

 protected:
  typename IntGame<schedulerId, true>::SecBit operation(
      typename IntGame<schedulerId, true>::SecSignedInt b1,
      typename IntGame<schedulerId, true>::SecSignedInt b2) override {
    return b1 > b2;
  }
};

BENCHMARK_COUNTERS(IntGreaterThanBenchmark, counters) {
  FrontendBenchmark<IntGreaterThanGame<0>, IntGreaterThanGame<1>> benchmark;
  benchmark.runBenchmark(counters);
}

template <int schedulerId>
class IntGreaterThanOrEqualToGame : public IntGame<schedulerId, true> {
 public:
  explicit IntGreaterThanOrEqualToGame(
      std::unique_ptr<scheduler::IScheduler> scheduler)
      : IntGame<schedulerId, true>(std::move(scheduler)) {}

 protected:
  typename IntGame<schedulerId, true>::SecBit operation(
      typename IntGame<schedulerId, true>::SecSignedInt b1,
      typename IntGame<schedulerId, true>::SecSignedInt b2) override {
    return b1 >= b2;
  }
};

BENCHMARK_COUNTERS(IntGreaterThanOrEqualToBenchmark, counters) {
  FrontendBenchmark<
      IntGreaterThanOrEqualToGame<0>,
      IntGreaterThanOrEqualToGame<1>>
      benchmark;
  benchmark.runBenchmark(counters);
}

template <int schedulerId>
class IntEqualToGame : public IntGame<schedulerId, true> {
 public:
  explicit IntEqualToGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : IntGame<schedulerId, true>(std::move(scheduler)) {}

 protected:
  typename IntGame<schedulerId, true>::SecBit operation(
      typename IntGame<schedulerId, true>::SecSignedInt b1,
      typename IntGame<schedulerId, true>::SecSignedInt b2) override {
    return b1 == b2;
  }
};

BENCHMARK_COUNTERS(IntEqualToBenchmark, counters) {
  FrontendBenchmark<IntEqualToGame<0>, IntEqualToGame<1>> benchmark;
  benchmark.runBenchmark(counters);
}
} // namespace fbpcf::frontend

int main(int argc, char* argv[]) {
  facebook::initFacebook(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
