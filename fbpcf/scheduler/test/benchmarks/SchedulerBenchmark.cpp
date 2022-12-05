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
#include "fbpcf/scheduler/EagerSchedulerFactory.h"
#include "fbpcf/scheduler/LazySchedulerFactory.h"
#include "fbpcf/scheduler/test/benchmarks/AllocatorBenchmark.h"
#include "fbpcf/scheduler/test/benchmarks/WireKeeperBenchmark.h"

namespace fbpcf::scheduler {

// IAllocator benchmarks

BENCHMARK(VectorArenaAllocator_allocate, n) {
  benchmarkAllocate<int64_t>(
      std::make_unique<VectorArenaAllocator<int64_t, unsafe>>(), n);
}

BENCHMARK(VectorArenaAllocator_free, n) {
  benchmarkFree<int64_t>(
      std::make_unique<VectorArenaAllocator<int64_t, unsafe>>(), n);
}

BENCHMARK(VectorArenaAllocator_get, n) {
  benchmarkGet<int64_t>(
      std::make_unique<VectorArenaAllocator<int64_t, unsafe>>(), n);
}

BENCHMARK(UnorderedMapAllocator_allocate, n) {
  benchmarkAllocate<int64_t>(
      std::make_unique<UnorderedMapAllocator<int64_t>>(), n);
}

BENCHMARK(UnorderedMapAllocator_free, n) {
  benchmarkFree<int64_t>(std::make_unique<UnorderedMapAllocator<int64_t>>(), n);
}

BENCHMARK(UnorderedMapAllocator_get, n) {
  benchmarkGet<int64_t>(std::make_unique<UnorderedMapAllocator<int64_t>>(), n);
}

// WireKeeper benchmarks

BENCHMARK(WireKeeperBenchmark_allocateBooleanValue, n) {
  BENCHMARK_WIREKEEPER {
    wireKeeper->allocateBooleanValue();
  }
}

BENCHMARK(WireKeeperBenchmark_getBooleanValue, n) {
  BENCHMARK_WIREKEEPER {
    wireKeeper->getBooleanValue(wireIds.at(n));
  }
}

BENCHMARK(WireKeeperBenchmark_setBooleanValue, n) {
  BENCHMARK_WIREKEEPER {
    wireKeeper->setBooleanValue(wireIds.at(n), n & 1);
  }
}

BENCHMARK(WireKeeperBenchmark_getFirstAvailableLevel, n) {
  BENCHMARK_WIREKEEPER {
    wireKeeper->getFirstAvailableLevel(wireIds.at(n));
  }
}

BENCHMARK(WireKeeperBenchmark_setFirstAvailableLevel, n) {
  BENCHMARK_WIREKEEPER {
    wireKeeper->setFirstAvailableLevel(wireIds.at(n), n);
  }
}

BENCHMARK(WireKeeperBenchmark_increaseReferenceCount, n) {
  BENCHMARK_WIREKEEPER {
    wireKeeper->increaseReferenceCount(wireIds.at(n));
  }
}

BENCHMARK(WireKeeperBenchmark_decreaseReferenceCount, n) {
  BENCHMARK_WIREKEEPER {
    wireKeeper->decreaseReferenceCount(wireIds.at(n));
  }
}

// Scheduler benchmarks

class SchedulerBenchmark : public engine::util::NetworkedBenchmark {
 public:
  void setup() override {
    auto [agentFactory0, agentFactory1] =
        engine::util::getSocketAgentFactories();
    agentFactory0_ = std::move(agentFactory0);
    agentFactory1_ = std::move(agentFactory1);

    // Set up randomized inputs
    batchInput0_ = engine::util::getRandomBoolVector(batchSize_);
    batchInput1_ = engine::util::getRandomBoolVector(batchSize_);

    input0_ = batchInput0_.at(0);
    input1_ = batchInput1_.at(0);

    randomParty_ = batchInput0_.at(1);
  }

 protected:
  void initSender() override {
    sender_ = getScheduler(0, *agentFactory0_);
  }

  void runSender() override {
    runMethod(sender_);
  }

  void initReceiver() override {
    receiver_ = getScheduler(1, *agentFactory1_);
  }

  void runReceiver() override {
    runMethod(receiver_);
  }

  virtual std::unique_ptr<IScheduler> getScheduler(
      int myId,
      engine::communication::IPartyCommunicationAgentFactory&
          communicationAgentFactory) = 0;

  virtual void runMethod(std::unique_ptr<IScheduler>& scheduler) = 0;

  size_t batchSize_ = 1000;
  std::vector<bool> batchInput0_;
  std::vector<bool> batchInput1_;

  bool input0_;
  bool input1_;

  int randomParty_;

 private:
  std::unique_ptr<engine::communication::IPartyCommunicationAgentFactory>
      agentFactory0_;
  std::unique_ptr<engine::communication::IPartyCommunicationAgentFactory>
      agentFactory1_;

  std::unique_ptr<IScheduler> sender_;
  std::unique_ptr<IScheduler> receiver_;
};

class LazySchedulerBenchmark : virtual public SchedulerBenchmark {
 protected:
  std::unique_ptr<IScheduler> getScheduler(
      int myId,
      engine::communication::IPartyCommunicationAgentFactory&
          communicationAgentFactory) override {
    return scheduler::getLazySchedulerFactoryWithInsecureEngine<unsafe>(
               myId, communicationAgentFactory)
        ->create();
  }
};

class EagerSchedulerBenchmark : virtual public SchedulerBenchmark {
 protected:
  std::unique_ptr<IScheduler> getScheduler(
      int myId,
      engine::communication::IPartyCommunicationAgentFactory&
          communicationAgentFactory) override {
    return scheduler::getEagerSchedulerFactoryWithInsecureEngine<unsafe>(
               myId, communicationAgentFactory)
        ->create();
  }
};

class NonFreeGatesBenchmark : virtual public SchedulerBenchmark {
 protected:
  void runMethod(std::unique_ptr<IScheduler>& scheduler) override {
    auto wire1 = scheduler->privateBooleanInput(input0_, randomParty_);
    auto wire2 = scheduler->privateBooleanInput(input1_, 1 - randomParty_);
    IScheduler::WireId<IScheduler::Boolean> wire3;
    for (auto level = 0; level < 10; level++) {
      for (auto i = 0; i < 200; i++) {
        wire3 = scheduler->privateAndPrivate(wire1, wire2);
      }
      wire2 = wire3;
    }
    scheduler->getBooleanValue(
        scheduler->openBooleanValueToParty(wire3, randomParty_));
  }
};

class NonFreeGatesBatchBenchmark : virtual public SchedulerBenchmark {
 protected:
  void runMethod(std::unique_ptr<IScheduler>& scheduler) override {
    auto wire1 =
        scheduler->privateBooleanInputBatch(batchInput0_, randomParty_);
    auto wire2 =
        scheduler->privateBooleanInputBatch(batchInput1_, 1 - randomParty_);
    IScheduler::WireId<IScheduler::Boolean> wire3;
    for (auto level = 0; level < 10; level++) {
      for (auto i = 0; i < 200; i++) {
        wire3 = scheduler->privateAndPrivateBatch(wire1, wire2);
      }
      wire2 = wire3;
    }
    scheduler->getBooleanValueBatch(
        scheduler->openBooleanValueToPartyBatch(wire3, randomParty_));
  }
};

class NonFreeGatesCompositeBenchmark : virtual public SchedulerBenchmark {
 protected:
  void runMethod(std::unique_ptr<IScheduler>& scheduler) override {
    auto leftWire = scheduler->privateBooleanInput(input0_, randomParty_);
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rightWires(10);
    for (auto i = 0; i < rightWires.size(); i++) {
      rightWires.at(i) =
          scheduler->privateBooleanInput(input1_, 1 - randomParty_);
    }

    std::vector<IScheduler::WireId<IScheduler::Boolean>> outputWires;
    for (auto level = 0; level < 10; level++) {
      for (auto i = 0; i < 500; i++) {
        outputWires =
            scheduler->privateAndPrivateComposite(leftWire, rightWires);
      }
      rightWires = outputWires;
    }

    for (auto i = 0; i < outputWires.size(); i++) {
      scheduler->getBooleanValue(
          scheduler->openBooleanValueToParty(outputWires.at(i), randomParty_));
    }
  }
};

class NonFreeGatesCompositeBatchBenchmark : virtual public SchedulerBenchmark {
 protected:
  void runMethod(std::unique_ptr<IScheduler>& scheduler) override {
    auto leftWire =
        scheduler->privateBooleanInputBatch(batchInput0_, randomParty_);
    std::vector<IScheduler::WireId<IScheduler::Boolean>> rightWires(10);
    for (auto i = 0; i < rightWires.size(); i++) {
      rightWires.at(i) =
          scheduler->privateBooleanInputBatch(batchInput1_, 1 - randomParty_);
    }

    std::vector<IScheduler::WireId<IScheduler::Boolean>> outputWires;
    for (auto level = 0; level < 10; level++) {
      for (auto i = 0; i < 500; i++) {
        outputWires =
            scheduler->privateAndPrivateCompositeBatch(leftWire, rightWires);
      }
      rightWires = outputWires;
    }

    for (auto i = 0; i < outputWires.size(); i++) {
      scheduler->getBooleanValueBatch(scheduler->openBooleanValueToPartyBatch(
          outputWires.at(i), randomParty_));
    }
  }
};

class LazyScheduler_NonFreeGates_Benchmark : public LazySchedulerBenchmark,
                                             public NonFreeGatesBenchmark {};

BENCHMARK_COUNTERS(LazyScheduler_NonFreeGates, counters) {
  LazyScheduler_NonFreeGates_Benchmark benchmark;
  benchmark.runBenchmark(counters);
}

class EagerScheduler_NonFreeGates_Benchmark : public EagerSchedulerBenchmark,
                                              public NonFreeGatesBenchmark {};

BENCHMARK_COUNTERS(EagerScheduler_NonFreeGates, counters) {
  EagerScheduler_NonFreeGates_Benchmark benchmark;
  benchmark.runBenchmark(counters);
}

class LazyScheduler_NonFreeGatesBatch_Benchmark
    : public LazySchedulerBenchmark,
      public NonFreeGatesBatchBenchmark {};

BENCHMARK_COUNTERS(LazyScheduler_NonFreeGatesBatch, counters) {
  LazyScheduler_NonFreeGatesBatch_Benchmark benchmark;
  benchmark.runBenchmark(counters);
}

class EagerScheduler_NonFreeGatesBatch_Benchmark
    : public EagerSchedulerBenchmark,
      public NonFreeGatesBatchBenchmark {};

BENCHMARK_COUNTERS(EagerScheduler_NonFreeGatesBatch, counters) {
  EagerScheduler_NonFreeGatesBatch_Benchmark benchmark;
  benchmark.runBenchmark(counters);
}

class LazyScheduler_NonFreeGatesComposite_Benchmark
    : public LazySchedulerBenchmark,
      public NonFreeGatesCompositeBenchmark {};

BENCHMARK_COUNTERS(LazyScheduler_NonFreeGatesComposite, counters) {
  LazyScheduler_NonFreeGatesComposite_Benchmark benchmark;
  benchmark.runBenchmark(counters);
}

class EagerScheduler_NonFreeGatesComposite_Benchmark
    : public EagerSchedulerBenchmark,
      public NonFreeGatesCompositeBenchmark {};

BENCHMARK_COUNTERS(EagerScheduler_NonFreeGatesComposite, counters) {
  EagerScheduler_NonFreeGatesComposite_Benchmark benchmark;
  benchmark.runBenchmark(counters);
}

class LazyScheduler_NonFreeGatesCompositeBatch_Benchmark
    : public LazySchedulerBenchmark,
      public NonFreeGatesCompositeBatchBenchmark {};

BENCHMARK_COUNTERS(LazyScheduler_NonFreeGatesCompositeBatch, counters) {
  LazyScheduler_NonFreeGatesCompositeBatch_Benchmark benchmark;
  benchmark.runBenchmark(counters);
}

class EagerScheduler_NonFreeGatesCompositeBatch_Benchmark
    : public EagerSchedulerBenchmark,
      public NonFreeGatesCompositeBatchBenchmark {};

BENCHMARK_COUNTERS(EagerScheduler_NonFreeGatesCompositeBatch, counters) {
  EagerScheduler_NonFreeGatesCompositeBatch_Benchmark benchmark;
  benchmark.runBenchmark(counters);
}
} // namespace fbpcf::scheduler

int main(int argc, char* argv[]) {
  facebook::initFacebook(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
