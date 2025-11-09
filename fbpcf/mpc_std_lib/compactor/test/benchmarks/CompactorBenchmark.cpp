/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Benchmark.h>

#include "common/init/Init.h"

#include "fbpcf/engine/util/AesPrgFactory.h"
#include "fbpcf/engine/util/test/benchmarks/BenchmarkHelper.h"
#include "fbpcf/engine/util/test/benchmarks/NetworkedBenchmark.h"
#include "fbpcf/mpc_std_lib/compactor/DummyCompactor.h"
#include "fbpcf/mpc_std_lib/compactor/ICompactor.h"
#include "fbpcf/mpc_std_lib/compactor/ICompactorFactory.h"
#include "fbpcf/mpc_std_lib/compactor/ShuffleBasedCompactor.h"
#include "fbpcf/mpc_std_lib/compactor/ShuffleBasedCompactorFactory.h"
#include "fbpcf/mpc_std_lib/permuter/AsWaksmanPermuterFactory.h"
#include "fbpcf/mpc_std_lib/shuffler/PermuteBasedShufflerFactory.h"
#include "fbpcf/mpc_std_lib/util/test/util.h"
#include "fbpcf/scheduler/IScheduler.h"
#include "fbpcf/scheduler/LazySchedulerFactory.h"

namespace fbpcf::mpc_std_lib::compactor {

const uint32_t batchSize = 1000;

class ShuffleBasedCompactorBenchmark : public engine::util::NetworkedBenchmark {
 public:
  void setup() override {
    auto [agentFactory0, agentFactory1] =
        engine::util::getSocketAgentFactories();
    agentFactory0_ = std::move(agentFactory0);
    agentFactory1_ = std::move(agentFactory1);

    std::vector<uint32_t> value(batchSize);
    std::iota(
        value.begin(), value.end(), 0); // assign unique values starting from 0.

    auto label = util::generateRandomBinary(batchSize);
    value_ = value;
    label_ = label;

    factory0_ =
        std::make_unique<ShuffleBasedCompactorFactory<uint32_t, bool, 0>>(
            0,
            1,
            std::make_unique<shuffler::PermuteBasedShufflerFactory<std::pair<
                frontend::Int<false, 32, true, 0, true>,
                frontend::Bit<true, 0, true>>>>(
                0,
                1,
                std::make_unique<permuter::AsWaksmanPermuterFactory<
                    std::pair<uint32_t, bool>,
                    0>>(0, 1),
                std::make_unique<engine::util::AesPrgFactory>()));
    factory1_ =
        std::make_unique<ShuffleBasedCompactorFactory<uint32_t, bool, 1>>(
            1,
            0,
            std::make_unique<shuffler::PermuteBasedShufflerFactory<std::pair<
                frontend::Int<false, 32, true, 1, true>,
                frontend::Bit<true, 1, true>>>>(
                1,
                0,
                std::make_unique<permuter::AsWaksmanPermuterFactory<
                    std::pair<uint32_t, bool>,
                    1>>(1, 0),
                std::make_unique<engine::util::AesPrgFactory>()));
  }

 protected:
  void initSender() override {
    scheduler::SchedulerKeeper<0>::setScheduler(
        scheduler::getLazySchedulerFactoryWithRealEngine(0, *agentFactory0_)
            ->create());
    compactor0_ = factory0_->create();
  }

  void runSender() override {
    auto secValue =
        util::MpcAdapters<uint32_t, 0>::processSecretInputs(value_, 0);
    auto secLabel = util::MpcAdapters<bool, 0>::processSecretInputs(label_, 0);

    auto [compactifiedValue, compactifiedLabel] =
        compactor0_->compaction(secValue, secLabel, batchSize, true);

    auto rstLabel =
        util::MpcAdapters<bool, 0>::openToParty(compactifiedLabel, 0);
    auto rstValue =
        util::MpcAdapters<uint32_t, 0>::openToParty(compactifiedValue, 0);
  }

  void initReceiver() override {
    scheduler::SchedulerKeeper<1>::setScheduler(
        scheduler::getLazySchedulerFactoryWithRealEngine(1, *agentFactory1_)
            ->create());
    compactor1_ = factory1_->create();
  }

  void runReceiver() override {
    auto secValue =
        util::MpcAdapters<uint32_t, 1>::processSecretInputs(value_, 0);
    auto secLabel = util::MpcAdapters<bool, 1>::processSecretInputs(label_, 0);

    auto [compactifiedValue, compactifiedLabel] =
        compactor1_->compaction(secValue, secLabel, batchSize, true);

    auto rstLabel =
        util::MpcAdapters<bool, 1>::openToParty(compactifiedLabel, 0);
    auto rstValue =
        util::MpcAdapters<uint32_t, 1>::openToParty(compactifiedValue, 0);
  }

  std::pair<uint64_t, uint64_t> getTrafficStatistics() override {
    auto schedulerTraffic =
        scheduler::SchedulerKeeper<0>::getTrafficStatistics();
    return schedulerTraffic;
  }

  std::unique_ptr<engine::communication::IPartyCommunicationAgentFactory>
      agentFactory0_;
  std::unique_ptr<engine::communication::IPartyCommunicationAgentFactory>
      agentFactory1_;

  std::unique_ptr<ICompactorFactory<
      typename util::SecBatchType<uint32_t, 0>::type,
      typename util::SecBatchType<bool, 0>::type>>
      factory0_;
  std::unique_ptr<ICompactorFactory<
      typename util::SecBatchType<uint32_t, 1>::type,
      typename util::SecBatchType<bool, 1>::type>>
      factory1_;

 private:
  std::unique_ptr<ICompactor<
      typename util::SecBatchType<uint32_t, 0>::type,
      typename util::SecBatchType<bool, 0>::type>>
      compactor0_;
  std::unique_ptr<ICompactor<
      typename util::SecBatchType<uint32_t, 1>::type,
      typename util::SecBatchType<bool, 1>::type>>
      compactor1_;

  std::vector<uint32_t> value_;
  std::vector<bool> label_;
};

BENCHMARK_COUNTERS(ShuffleBasedCompactor_Benchmark, counters) {
  ShuffleBasedCompactorBenchmark benchmark;
  benchmark.runBenchmark(counters);
}
} // namespace fbpcf::mpc_std_lib::compactor

int main(int argc, char* argv[]) {
  facebook::initFacebook(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
