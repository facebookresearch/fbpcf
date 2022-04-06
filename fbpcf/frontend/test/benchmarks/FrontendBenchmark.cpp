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
#include "fbpcf/frontend/BitString.h"
#include "fbpcf/frontend/mpcGame.h"
#include "fbpcf/scheduler/IScheduler.h"
#include "fbpcf/scheduler/SchedulerHelper.h"

namespace fbpcf::frontend {

const bool unsafe = true;
const uint32_t batchSize = 1000;

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
    return b1 | b2;
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

template <int schedulerId, bool isBoolOutput, bool usingBatch>
class IntGame : public MpcGame<schedulerId> {
 public:
  using SecSignedInt = typename frontend::MpcGame<
      schedulerId>::template SecSignedInt<32, usingBatch>;
  using SecBit =
      typename frontend::MpcGame<schedulerId>::template SecBit<usingBatch>;

  using TOutput =
      typename std::conditional<isBoolOutput, SecBit, SecSignedInt>::type;

  explicit IntGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : MpcGame<schedulerId>(std::move(scheduler)) {}

  virtual ~IntGame() = default;

  void play() {
    SecSignedInt b1;
    SecSignedInt b2;
    if constexpr (usingBatch) {
      b1 = SecSignedInt(
          std::vector<int32_t>(
              batchSize, static_cast<int32_t>(folly::Random::rand32())),
          0);
      b2 = SecSignedInt(
          std::vector<int32_t>(
              batchSize, static_cast<int32_t>(folly::Random::rand32())),
          1);
    } else {
      b1 = SecSignedInt(static_cast<int32_t>(folly::Random::rand32()), 0);
      b2 = SecSignedInt(static_cast<int32_t>(folly::Random::rand32()), 1);
    }
    TOutput b3;
    for (auto i = 0; i < 100; i++) {
      b3 = operation(b1, b2);
    }
    b3.openToParty(0).getValue();
  }

 protected:
  virtual TOutput operation(SecSignedInt b1, SecSignedInt b2) = 0;
};

template <int schedulerId, bool usingBatch>
class IntAddGame : public IntGame<schedulerId, false, usingBatch> {
 public:
  explicit IntAddGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : IntGame<schedulerId, false, usingBatch>(std::move(scheduler)) {}

 protected:
  typename IntGame<schedulerId, false, usingBatch>::SecSignedInt operation(
      typename IntGame<schedulerId, false, usingBatch>::SecSignedInt b1,
      typename IntGame<schedulerId, false, usingBatch>::SecSignedInt b2)
      override {
    return b1 + b2;
  }
};

BENCHMARK_COUNTERS(IntAddBenchmark, counters) {
  FrontendBenchmark<IntAddGame<0, false>, IntAddGame<1, false>> benchmark;
  benchmark.runBenchmark(counters);
}

BENCHMARK_COUNTERS(IntAddBatchBenchmark, counters) {
  FrontendBenchmark<IntAddGame<0, true>, IntAddGame<1, true>> benchmark;
  benchmark.runBenchmark(counters);
}

template <int schedulerId, bool usingBatch>
class IntSubtractGame : public IntGame<schedulerId, false, usingBatch> {
 public:
  explicit IntSubtractGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : IntGame<schedulerId, false, usingBatch>(std::move(scheduler)) {}

 protected:
  typename IntGame<schedulerId, false, usingBatch>::SecSignedInt operation(
      typename IntGame<schedulerId, false, usingBatch>::SecSignedInt b1,
      typename IntGame<schedulerId, false, usingBatch>::SecSignedInt b2)
      override {
    return b1 - b2;
  }
};

BENCHMARK_COUNTERS(IntSubtractBenchmark, counters) {
  FrontendBenchmark<IntSubtractGame<0, false>, IntSubtractGame<1, false>>
      benchmark;
  benchmark.runBenchmark(counters);
}

BENCHMARK_COUNTERS(IntSubtractBatchBenchmark, counters) {
  FrontendBenchmark<IntSubtractGame<0, true>, IntSubtractGame<1, true>>
      benchmark;
  benchmark.runBenchmark(counters);
}

template <int schedulerId, bool usingBatch>
class IntMuxGame : public IntGame<schedulerId, false, usingBatch> {
 public:
  explicit IntMuxGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : IntGame<schedulerId, false, usingBatch>(std::move(scheduler)) {}

 protected:
  typename IntGame<schedulerId, false, usingBatch>::SecSignedInt operation(
      typename IntGame<schedulerId, false, usingBatch>::SecSignedInt b1,
      typename IntGame<schedulerId, false, usingBatch>::SecSignedInt b2)
      override {
    typename IntGame<schedulerId, false, usingBatch>::SecBit choice;
    if constexpr (usingBatch) {
      choice = typename IntGame<schedulerId, false, usingBatch>::SecBit(
          std::vector<bool>(batchSize, true), 1);
    } else {
      choice =
          typename IntGame<schedulerId, false, usingBatch>::SecBit(true, 1);
    }
    return b1.mux(choice, b2);
  }
};

BENCHMARK_COUNTERS(IntMuxBenchmark, counters) {
  FrontendBenchmark<IntMuxGame<0, false>, IntMuxGame<1, false>> benchmark;
  benchmark.runBenchmark(counters);
}

BENCHMARK_COUNTERS(IntMuxBatchBenchmark, counters) {
  FrontendBenchmark<IntMuxGame<0, true>, IntMuxGame<1, true>> benchmark;
  benchmark.runBenchmark(counters);
}

template <int schedulerId, bool usingBatch>
class IntLessThanGame : public IntGame<schedulerId, true, usingBatch> {
 public:
  explicit IntLessThanGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : IntGame<schedulerId, true, usingBatch>(std::move(scheduler)) {}

 protected:
  typename IntGame<schedulerId, true, usingBatch>::SecBit operation(
      typename IntGame<schedulerId, true, usingBatch>::SecSignedInt b1,
      typename IntGame<schedulerId, true, usingBatch>::SecSignedInt b2)
      override {
    return b1 < b2;
  }
};

BENCHMARK_COUNTERS(IntLessThanBenchmark, counters) {
  FrontendBenchmark<IntLessThanGame<0, false>, IntLessThanGame<1, false>>
      benchmark;
  benchmark.runBenchmark(counters);
}

BENCHMARK_COUNTERS(IntLessThanBatchBenchmark, counters) {
  FrontendBenchmark<IntLessThanGame<0, true>, IntLessThanGame<1, true>>
      benchmark;
  benchmark.runBenchmark(counters);
}

template <int schedulerId, bool usingBatch>
class IntLessThanOrEqualToGame : public IntGame<schedulerId, true, usingBatch> {
 public:
  explicit IntLessThanOrEqualToGame(
      std::unique_ptr<scheduler::IScheduler> scheduler)
      : IntGame<schedulerId, true, usingBatch>(std::move(scheduler)) {}

 protected:
  typename IntGame<schedulerId, true, usingBatch>::SecBit operation(
      typename IntGame<schedulerId, true, usingBatch>::SecSignedInt b1,
      typename IntGame<schedulerId, true, usingBatch>::SecSignedInt b2)
      override {
    return b1 <= b2;
  }
};

BENCHMARK_COUNTERS(IntLessThanOrEqualToBenchmark, counters) {
  FrontendBenchmark<
      IntLessThanOrEqualToGame<0, false>,
      IntLessThanOrEqualToGame<1, false>>
      benchmark;
  benchmark.runBenchmark(counters);
}

BENCHMARK_COUNTERS(IntLessThanOrEqualToBatchBenchmark, counters) {
  FrontendBenchmark<
      IntLessThanOrEqualToGame<0, true>,
      IntLessThanOrEqualToGame<1, true>>
      benchmark;
  benchmark.runBenchmark(counters);
}

template <int schedulerId, bool usingBatch>
class IntGreaterThanGame : public IntGame<schedulerId, true, usingBatch> {
 public:
  explicit IntGreaterThanGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : IntGame<schedulerId, true, usingBatch>(std::move(scheduler)) {}

 protected:
  typename IntGame<schedulerId, true, usingBatch>::SecBit operation(
      typename IntGame<schedulerId, true, usingBatch>::SecSignedInt b1,
      typename IntGame<schedulerId, true, usingBatch>::SecSignedInt b2)
      override {
    return b1 > b2;
  }
};

BENCHMARK_COUNTERS(IntGreaterThanBenchmark, counters) {
  FrontendBenchmark<IntGreaterThanGame<0, false>, IntGreaterThanGame<1, false>>
      benchmark;
  benchmark.runBenchmark(counters);
}

BENCHMARK_COUNTERS(IntGreaterThanBatchBenchmark, counters) {
  FrontendBenchmark<IntGreaterThanGame<0, true>, IntGreaterThanGame<1, true>>
      benchmark;
  benchmark.runBenchmark(counters);
}

template <int schedulerId, bool usingBatch>
class IntGreaterThanOrEqualToGame
    : public IntGame<schedulerId, true, usingBatch> {
 public:
  explicit IntGreaterThanOrEqualToGame(
      std::unique_ptr<scheduler::IScheduler> scheduler)
      : IntGame<schedulerId, true, usingBatch>(std::move(scheduler)) {}

 protected:
  typename IntGame<schedulerId, true, usingBatch>::SecBit operation(
      typename IntGame<schedulerId, true, usingBatch>::SecSignedInt b1,
      typename IntGame<schedulerId, true, usingBatch>::SecSignedInt b2)
      override {
    return b1 >= b2;
  }
};

BENCHMARK_COUNTERS(IntGreaterThanOrEqualToBenchmark, counters) {
  FrontendBenchmark<
      IntGreaterThanOrEqualToGame<0, false>,
      IntGreaterThanOrEqualToGame<1, false>>
      benchmark;
  benchmark.runBenchmark(counters);
}

BENCHMARK_COUNTERS(IntGreaterThanOrEqualToBatchBenchmark, counters) {
  FrontendBenchmark<
      IntGreaterThanOrEqualToGame<0, true>,
      IntGreaterThanOrEqualToGame<1, true>>
      benchmark;
  benchmark.runBenchmark(counters);
}

template <int schedulerId, bool usingBatch>
class IntEqualToGame : public IntGame<schedulerId, true, usingBatch> {
 public:
  explicit IntEqualToGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : IntGame<schedulerId, true, usingBatch>(std::move(scheduler)) {}

 protected:
  typename IntGame<schedulerId, true, usingBatch>::SecBit operation(
      typename IntGame<schedulerId, true, usingBatch>::SecSignedInt b1,
      typename IntGame<schedulerId, true, usingBatch>::SecSignedInt b2)
      override {
    return b1 == b2;
  }
};

BENCHMARK_COUNTERS(IntEqualToBenchmark, counters) {
  FrontendBenchmark<IntEqualToGame<0, false>, IntEqualToGame<1, false>>
      benchmark;
  benchmark.runBenchmark(counters);
}

BENCHMARK_COUNTERS(IntEqualToBatchBenchmark, counters) {
  FrontendBenchmark<IntEqualToGame<0, true>, IntEqualToGame<1, true>> benchmark;
  benchmark.runBenchmark(counters);
}

template <int schedulerId, bool usingBatch>
class BitStringGame : public MpcGame<schedulerId> {
 public:
  using BitString = typename frontend::BitString<true, schedulerId, usingBatch>;

  explicit BitStringGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : MpcGame<schedulerId>(std::move(scheduler)) {}

  virtual ~BitStringGame() = default;

  void play() {
    auto size = 32;
    BitString b1;
    BitString b2;
    if constexpr (usingBatch) {
      b1 = BitString(
          std::vector<std::vector<bool>>(
              batchSize, std::vector<bool>(size, true)),
          0);
      b2 = BitString(
          std::vector<std::vector<bool>>(
              batchSize, std::vector<bool>(size, false)),
          1);
    } else {
      b1 = BitString(std::vector<bool>(size, true), 0);
      b2 = BitString(std::vector<bool>(size, false), 1);
    }

    BitString b3;
    for (auto i = 0; i < 100; i++) {
      b3 = operation(b1, b2);
    }
    b3.openToParty(0).getValue();
  }

 protected:
  virtual BitString operation(BitString b1, BitString b2) = 0;
};

template <int schedulerId, bool usingBatch>
class BitStringNotGame : public BitStringGame<schedulerId, usingBatch> {
 public:
  explicit BitStringNotGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : BitStringGame<schedulerId, usingBatch>(std::move(scheduler)) {}

 protected:
  typename BitStringGame<schedulerId, usingBatch>::BitString operation(
      typename BitStringGame<schedulerId, usingBatch>::BitString b1,
      typename BitStringGame<schedulerId, usingBatch>::BitString) {
    return !b1;
  }
};

BENCHMARK_COUNTERS(BitStringNotBenchmark, counters) {
  FrontendBenchmark<BitStringNotGame<0, false>, BitStringNotGame<1, false>>
      benchmark;
  benchmark.runBenchmark(counters);
}

BENCHMARK_COUNTERS(BitStringNotBatchBenchmark, counters) {
  FrontendBenchmark<BitStringNotGame<0, true>, BitStringNotGame<1, true>>
      benchmark;
  benchmark.runBenchmark(counters);
}

template <int schedulerId, bool usingBatch>
class BitStringAndGame : public BitStringGame<schedulerId, usingBatch> {
 public:
  explicit BitStringAndGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : BitStringGame<schedulerId, usingBatch>(std::move(scheduler)) {}

 protected:
  typename BitStringGame<schedulerId, usingBatch>::BitString operation(
      typename BitStringGame<schedulerId, usingBatch>::BitString b1,
      typename BitStringGame<schedulerId, usingBatch>::BitString b2) override {
    return b1 & b2;
  }
};

BENCHMARK_COUNTERS(BitStringAndBenchmark, counters) {
  FrontendBenchmark<BitStringAndGame<0, false>, BitStringAndGame<1, false>>
      benchmark;
  benchmark.runBenchmark(counters);
}

BENCHMARK_COUNTERS(BitStringAndBatchBenchmark, counters) {
  FrontendBenchmark<BitStringAndGame<0, true>, BitStringAndGame<1, true>>
      benchmark;
  benchmark.runBenchmark(counters);
}

template <int schedulerId, bool usingBatch>
class BitStringXorGame : public BitStringGame<schedulerId, usingBatch> {
 public:
  explicit BitStringXorGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : BitStringGame<schedulerId, usingBatch>(std::move(scheduler)) {}

 protected:
  typename BitStringGame<schedulerId, usingBatch>::BitString operation(
      typename BitStringGame<schedulerId, usingBatch>::BitString b1,
      typename BitStringGame<schedulerId, usingBatch>::BitString b2) override {
    return b1 ^ b2;
  }
};

BENCHMARK_COUNTERS(BitStringXorBenchmark, counters) {
  FrontendBenchmark<BitStringXorGame<0, false>, BitStringXorGame<1, false>>
      benchmark;
  benchmark.runBenchmark(counters);
}

BENCHMARK_COUNTERS(BitStringXorBatchBenchmark, counters) {
  FrontendBenchmark<BitStringXorGame<0, true>, BitStringXorGame<1, true>>
      benchmark;
  benchmark.runBenchmark(counters);
}

template <int schedulerId, bool usingBatch>
class BitStringMuxGame : public BitStringGame<schedulerId, usingBatch> {
 public:
  explicit BitStringMuxGame(std::unique_ptr<scheduler::IScheduler> scheduler)
      : BitStringGame<schedulerId, usingBatch>(std::move(scheduler)) {}

 protected:
  typename BitStringGame<schedulerId, usingBatch>::BitString operation(
      typename BitStringGame<schedulerId, usingBatch>::BitString b1,
      typename BitStringGame<schedulerId, usingBatch>::BitString b2) override {
    typename IntGame<schedulerId, false, usingBatch>::SecBit choice;
    if constexpr (usingBatch) {
      choice = typename IntGame<schedulerId, false, usingBatch>::SecBit(
          std::vector<bool>(batchSize, true), 1);
    } else {
      choice =
          typename IntGame<schedulerId, false, usingBatch>::SecBit(true, 1);
    }
    return b1.mux(choice, b2);
  }
};

BENCHMARK_COUNTERS(BitStringMuxBenchmark, counters) {
  FrontendBenchmark<BitStringMuxGame<0, false>, BitStringMuxGame<1, false>>
      benchmark;
  benchmark.runBenchmark(counters);
}

BENCHMARK_COUNTERS(BitStringMuxBatchBenchmark, counters) {
  FrontendBenchmark<BitStringMuxGame<0, true>, BitStringMuxGame<1, true>>
      benchmark;
  benchmark.runBenchmark(counters);
}
} // namespace fbpcf::frontend

int main(int argc, char* argv[]) {
  facebook::initFacebook(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
