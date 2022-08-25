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
#include "fbpcf/mpc_std_lib/oram/ISinglePointArrayGenerator.h"
#include "fbpcf/mpc_std_lib/oram/IWriteOnlyOram.h"
#include "fbpcf/mpc_std_lib/oram/LinearOramFactory.h"
#include "fbpcf/mpc_std_lib/oram/ObliviousDeltaCalculatorFactory.h"
#include "fbpcf/mpc_std_lib/oram/SinglePointArrayGeneratorFactory.h"
#include "fbpcf/mpc_std_lib/oram/WriteOnlyOramFactory.h"
#include "fbpcf/mpc_std_lib/util/test/util.h"
#include "fbpcf/scheduler/IScheduler.h"
#include "fbpcf/scheduler/LazySchedulerFactory.h"

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
        scheduler::getLazySchedulerFactoryWithRealEngine(0, *agentFactory0_)
            ->create());
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
        scheduler::getLazySchedulerFactoryWithRealEngine(1, *agentFactory1_)
            ->create());
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

class ObliviousDeltaCalculatorBenchmark
    : public engine::util::NetworkedBenchmark {
 public:
  void setup() override {
    auto [agentFactory0, agentFactory1] =
        engine::util::getSocketAgentFactories();
    agentFactory0_ = std::move(agentFactory0);
    agentFactory1_ = std::move(agentFactory1);

    auto [input0, input1, _] =
        util::generateObliviousDeltaCalculatorInputs(batchSize_);
    input0_ = input0;
    input1_ = input1;
  }

 protected:
  void initSender() override {
    scheduler::SchedulerKeeper<0>::setScheduler(
        scheduler::getLazySchedulerFactoryWithRealEngine(0, *agentFactory0_)
            ->create());
    ObliviousDeltaCalculatorFactory<0> factory(true, 0, 1);
    sender_ = factory.create();
  }

  void runSender() override {
    sender_->calculateDelta(
        input0_.delta0Shares, input0_.delta1Shares, input0_.alphaShares);
  }

  void initReceiver() override {
    scheduler::SchedulerKeeper<1>::setScheduler(
        scheduler::getLazySchedulerFactoryWithRealEngine(1, *agentFactory1_)
            ->create());
    ObliviousDeltaCalculatorFactory<1> factory(false, 0, 1);
    receiver_ = factory.create();
  }

  void runReceiver() override {
    receiver_->calculateDelta(
        input1_.delta0Shares, input1_.delta1Shares, input1_.alphaShares);
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

  std::unique_ptr<IObliviousDeltaCalculator> sender_;
  std::unique_ptr<IObliviousDeltaCalculator> receiver_;

  util::ObliviousDeltaCalculatorInputType input0_;
  util::ObliviousDeltaCalculatorInputType input1_;
};

BENCHMARK_COUNTERS(ObliviousDeltaCalculator_Benchmark, counters) {
  ObliviousDeltaCalculatorBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

class SinglePointArrayGeneratorBenchmark
    : public engine::util::NetworkedBenchmark {
 public:
  void setup() override {
    auto [agentFactory0, agentFactory1] =
        engine::util::getSocketAgentFactories();
    agentFactory0_ = std::move(agentFactory0);
    agentFactory1_ = std::move(agentFactory1);

    size_t width = std::ceil(std::log2(length_));
    size_t batchSize = 128;

    party0Input_ =
        std::vector<std::vector<bool>>(width, std::vector<bool>(batchSize));
    party1Input_ =
        std::vector<std::vector<bool>>(width, std::vector<bool>(batchSize));
    for (size_t i = 0; i < batchSize; i++) {
      auto [share0, share1, _] =
          util::generateSharedRandomBoolVectorForSinglePointArrayGenerator(
              length_);
      for (size_t j = 0; j < width; j++) {
        party0Input_[j][i] = share0.at(j);
        party1Input_[j][i] = share1.at(j);
      }
    }
  }

 protected:
  void initSender() override {
    scheduler::SchedulerKeeper<0>::setScheduler(
        scheduler::getLazySchedulerFactoryWithRealEngine(0, *agentFactory0_)
            ->create());
    SinglePointArrayGeneratorFactory factory(
        true, std::make_unique<ObliviousDeltaCalculatorFactory<0>>(true, 0, 1));
    sender_ = factory.create();
  }

  void runSender() override {
    sender_->generateSinglePointArrays(party0Input_, length_);
  }

  void initReceiver() override {
    scheduler::SchedulerKeeper<1>::setScheduler(
        scheduler::getLazySchedulerFactoryWithRealEngine(1, *agentFactory1_)
            ->create());
    SinglePointArrayGeneratorFactory factory(
        false,
        std::make_unique<ObliviousDeltaCalculatorFactory<1>>(false, 0, 1));
    receiver_ = factory.create();
  }

  void runReceiver() override {
    receiver_->generateSinglePointArrays(party1Input_, length_);
  }

  std::pair<uint64_t, uint64_t> getTrafficStatistics() override {
    return scheduler::SchedulerKeeper<0>::getTrafficStatistics();
  }

 private:
  size_t length_ = 16384;

  std::unique_ptr<engine::communication::IPartyCommunicationAgentFactory>
      agentFactory0_;
  std::unique_ptr<engine::communication::IPartyCommunicationAgentFactory>
      agentFactory1_;

  std::unique_ptr<ISinglePointArrayGenerator> sender_;
  std::unique_ptr<ISinglePointArrayGenerator> receiver_;

  std::vector<std::vector<bool>> party0Input_;
  std::vector<std::vector<bool>> party1Input_;
};

BENCHMARK_COUNTERS(SinglePointArrayGenerator_Benchmark, counters) {
  SinglePointArrayGeneratorBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

class BaseWriteOnlyOramBenchmark : public engine::util::NetworkedBenchmark {
 public:
  void setup() override {
    auto [agentFactory0, agentFactory1] =
        engine::util::getSocketAgentFactories();
    agentFactory0_ = std::move(agentFactory0);
    agentFactory1_ = std::move(agentFactory1);

    auto batchSize = 2048;
    auto [input0, input1, _] =
        util::generateRandomValuesToAdd<uint32_t>(oramSize_, batchSize);
    input0_ = input0;
    input1_ = input1;
  }

 protected:
  void initSender() override {
    scheduler::SchedulerKeeper<0>::setScheduler(
        scheduler::getLazySchedulerFactoryWithRealEngine(0, *agentFactory0_)
            ->create());
    auto factory = getOramFactory(true);
    sender_ = factory->create(oramSize_);
  }

  void runSender() override {
    runMethod(sender_, input0_);
  }

  void initReceiver() override {
    scheduler::SchedulerKeeper<1>::setScheduler(
        scheduler::getLazySchedulerFactoryWithRealEngine(1, *agentFactory1_)
            ->create());
    auto factory = getOramFactory(false);
    receiver_ = factory->create(oramSize_);
  }

  void runReceiver() override {
    runMethod(receiver_, input1_);
  }

  std::pair<uint64_t, uint64_t> getTrafficStatistics() override {
    auto schedulerTraffic =
        scheduler::SchedulerKeeper<0>::getTrafficStatistics();
    auto oramTraffic = sender_->getTrafficStatistics();
    return {
        schedulerTraffic.first + oramTraffic.first,
        schedulerTraffic.second + oramTraffic.second};
  }

  virtual std::unique_ptr<IWriteOnlyOramFactory<uint32_t>> getOramFactory(
      bool amIParty0) = 0;

  virtual void runMethod(
      std::unique_ptr<IWriteOnlyOram<uint32_t>>& oram,
      util::WritingType input) = 0;

  std::unique_ptr<engine::communication::IPartyCommunicationAgentFactory>
      agentFactory0_;
  std::unique_ptr<engine::communication::IPartyCommunicationAgentFactory>
      agentFactory1_;

  size_t oramSize_ = 150;

 private:
  std::unique_ptr<IWriteOnlyOram<uint32_t>> sender_;
  std::unique_ptr<IWriteOnlyOram<uint32_t>> receiver_;

  util::WritingType input0_;
  util::WritingType input1_;
};

class ObliviousAddBatchBenchmark : virtual public BaseWriteOnlyOramBenchmark {
 protected:
  void runMethod(
      std::unique_ptr<IWriteOnlyOram<uint32_t>>& oram,
      util::WritingType input) override {
    oram->obliviousAddBatch(input.indexShares, input.valueShares);
  }
};

class PublicReadBenchmark : virtual public BaseWriteOnlyOramBenchmark {
 protected:
  void runMethod(
      std::unique_ptr<IWriteOnlyOram<uint32_t>>& oram,
      util::WritingType) override {
    for (auto i = 0; i < oramSize_; ++i) {
      oram->publicRead(i, IWriteOnlyOram<uint32_t>::Alice);
    }
  }
};

class SecretReadBenchmark : virtual public BaseWriteOnlyOramBenchmark {
 protected:
  void runMethod(
      std::unique_ptr<IWriteOnlyOram<uint32_t>>& oram,
      util::WritingType) override {
    for (auto i = 0; i < oramSize_; ++i) {
      oram->secretRead(i);
    }
  }
};

class WriteOnlyOramBenchmark : virtual public BaseWriteOnlyOramBenchmark {
 protected:
  std::unique_ptr<IWriteOnlyOramFactory<uint32_t>> getOramFactory(
      bool amIParty0) override {
    return amIParty0
        ? getSecureWriteOnlyOramFactory<uint32_t, indicatorWidth, 0>(
              true, 0, 1, *agentFactory0_)
        : getSecureWriteOnlyOramFactory<uint32_t, indicatorWidth, 1>(
              false, 0, 1, *agentFactory1_);
  }
};

class WriteOnlyOramObliviousAddBatchBenchmark
    : public WriteOnlyOramBenchmark,
      public ObliviousAddBatchBenchmark {};

BENCHMARK_COUNTERS(WriteOnlyOramObliviousAddBatch_Benchmark, counters) {
  WriteOnlyOramObliviousAddBatchBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

class WriteOnlyOramPublicReadBenchmark : public WriteOnlyOramBenchmark,
                                         public PublicReadBenchmark {};

BENCHMARK_COUNTERS(WriteOnlyOramPublicRead_Benchmark, counters) {
  WriteOnlyOramPublicReadBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

class WriteOnlyOramSecretReadBenchmark : public WriteOnlyOramBenchmark,
                                         public SecretReadBenchmark {};

BENCHMARK_COUNTERS(WriteOnlyOramSecretRead_Benchmark, counters) {
  WriteOnlyOramSecretReadBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

class LinearOramBenchmark : virtual public BaseWriteOnlyOramBenchmark {
 protected:
  std::unique_ptr<IWriteOnlyOramFactory<uint32_t>> getOramFactory(
      bool amIParty0) override {
    return amIParty0
        ? getSecureLinearOramFactory<uint32_t, 0>(true, 0, 1, *agentFactory0_)
        : getSecureLinearOramFactory<uint32_t, 1>(false, 0, 1, *agentFactory1_);
  }
};

class LinearOramObliviousAddBatchBenchmark : public LinearOramBenchmark,
                                             public ObliviousAddBatchBenchmark {
};

BENCHMARK_COUNTERS(LinearOramObliviousAddBatch_Benchmark, counters) {
  LinearOramObliviousAddBatchBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

class LinearOramPublicReadBenchmark : public LinearOramBenchmark,
                                      public PublicReadBenchmark {};

BENCHMARK_COUNTERS(LinearOramPublicRead_Benchmark, counters) {
  LinearOramPublicReadBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

class LinearOramSecretReadBenchmark : public LinearOramBenchmark,
                                      public SecretReadBenchmark {};

BENCHMARK_COUNTERS(LinearOramSecretRead_Benchmark, counters) {
  LinearOramSecretReadBenchmark benchmark;
  benchmark.runBenchmark(counters);
}
} // namespace fbpcf::mpc_std_lib::oram

int main(int argc, char* argv[]) {
  facebook::initFacebook(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
