/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Benchmark.h>

#include "common/init/Init.h"

#include "fbpcf/engine/util/test/benchmarks/BenchmarkHelper.h"
#include "fbpcf/engine/util/test/benchmarks/NetworkedBenchmark.h"
#include "fbpcf/mpc_std_lib/oram/DifferenceCalculatorFactory.h"
#include "fbpcf/mpc_std_lib/oram/IDifferenceCalculatorFactory.h"
#include "fbpcf/mpc_std_lib/util/test/util.h"
#include "fbpcf/scheduler/IScheduler.h"
#include "fbpcf/scheduler/SchedulerHelper.h"

namespace fbpcf::mpc_std_lib::oram {

const int8_t indicatorWidth = 16;

class DifferenceCalculatorBenchmark : public engine::util::NetworkedBenchmark {
 public:
  void setup() override {
    auto [agentFactory0, agentFactory1] =
        engine::util::getSocketAgentFactories();
    agentFactory0_ = std::move(agentFactory0);
    agentFactory1_ = std::move(agentFactory1);

    auto [input0, input1, _] =
        util::generateRandomInputs<uint32_t, indicatorWidth>(batchSize_);
    input0_ = input0;
    input1_ = input1;
  }

 protected:
  void initSender() override {
    scheduler::SchedulerKeeper<0>::setScheduler(
        scheduler::createLazySchedulerWithRealEngine(0, *agentFactory0_));
    DifferenceCalculatorFactory<uint32_t, indicatorWidth, 0> factory(
        true, 0, 1);
    sender_ = factory.create();
  }

  void runSender() override {
    sender_->calculateDifferenceBatch(
        input0_.indicatorShares,
        input0_.minuendShares,
        input0_.subtrahendShares);
  }

  void initReceiver() override {
    scheduler::SchedulerKeeper<1>::setScheduler(
        scheduler::createLazySchedulerWithRealEngine(1, *agentFactory1_));
    DifferenceCalculatorFactory<uint32_t, indicatorWidth, 1> factory(
        false, 0, 1);
    receiver_ = factory.create();
  }

  void runReceiver() override {
    receiver_->calculateDifferenceBatch(
        input1_.indicatorShares,
        input1_.minuendShares,
        input1_.subtrahendShares);
  }

  std::pair<uint64_t, uint64_t> getTrafficStatistics() override {
    return scheduler::SchedulerKeeper<0>::getTrafficStatistics();
  }

 private:
  size_t batchSize_ = 16384;

  std::unique_ptr<engine::communication::IPartyCommunicationAgentFactory>
      agentFactory0_;
  std::unique_ptr<engine::communication::IPartyCommunicationAgentFactory>
      agentFactory1_;

  std::unique_ptr<IDifferenceCalculator<uint32_t>> sender_;
  std::unique_ptr<IDifferenceCalculator<uint32_t>> receiver_;

  util::InputType<uint32_t> input0_;
  util::InputType<uint32_t> input1_;
};

BENCHMARK_COUNTERS(DifferenceCalculator_Benchmark, counters) {
  DifferenceCalculatorBenchmark benchmark;
  benchmark.runBenchmark(counters);
}
} // namespace fbpcf::mpc_std_lib::oram

int main(int argc, char* argv[]) {
  facebook::initFacebook(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
