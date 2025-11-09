/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gmock/gmock.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <random>
#include <vector>

#include "common/init/Init.h"

#include "fbpcf/engine/tuple_generator/oblivious_transfer/DummyRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/EmpShRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ExtenderBasedRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/IknpShRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/NpBaseObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/RcotExtenderFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/RegularErrorMultiPointCotFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/SinglePointCotFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/TenLocalLinearMatrixMultiplierFactory.h"
#include "fbpcf/engine/util/AesPrgFactory.h"
#include "fbpcf/engine/util/test/benchmarks/BenchmarkHelper.h"
#include "fbpcf/engine/util/test/benchmarks/NetworkedBenchmark.h"

#include "fbpcf/mpc_std_lib/util/test/util.h"
#include "fbpcf/mpc_std_lib/walr_multiplication/DummyMatrixMultiplicationFactory.h"
#include "fbpcf/mpc_std_lib/walr_multiplication/IWalrMatrixMultiplication.h"
#include "fbpcf/mpc_std_lib/walr_multiplication/IWalrMatrixMultiplicationFactory.h"
#include "fbpcf/mpc_std_lib/walr_multiplication/OTBasedMatrixMultiplicationFactory.h"
#include "fbpcf/mpc_std_lib/walr_multiplication/util/COTWithRandomMessageFactory.h"

#include "fbpcf/scheduler/IScheduler.h"
#include "fbpcf/scheduler/LazySchedulerFactory.h"
#include "fbpcf/util/MetricCollector.h"

namespace fbpcf::mpc_std_lib::walr {

class WalrMatrixMultiplicationBenchmark
    : public engine::util::NetworkedBenchmark {
 public:
  void setup() override {
    // These agent factories are used for setting up the backend schedulers
    auto [agentFactory0, agentFactory1] =
        engine::util::getSocketAgentFactories();
    agentFactory0_ = std::move(agentFactory0);
    agentFactory1_ = std::move(agentFactory1);
    auto [nFeatures, nLabels] = dataSize();
    initData(nFeatures, nLabels);
  }

 protected:
  void initSender() override {
    scheduler::SchedulerKeeper<0>::setScheduler(
        scheduler::getLazySchedulerFactoryWithRealEngine(0, *agentFactory0_)
            ->create());
    featureOwner_ = featureOwnerFactory_->create();
  }

  void runSender() override {
    auto secLabel0 =
        mpc_std_lib::util::MpcAdapters<bool, 0>::processSecretInputs(
            labels_, 1);
    featureOwner_->matrixVectorMultiplication(features_, secLabel0);
  }

  void initReceiver() override {
    scheduler::SchedulerKeeper<1>::setScheduler(
        scheduler::getLazySchedulerFactoryWithRealEngine(1, *agentFactory1_)
            ->create());
    labelOwner_ = labelOwnerFactory_->create();
  }

  void runReceiver() override {
    auto secLabel1 =
        mpc_std_lib::util::MpcAdapters<bool, 1>::processSecretInputs(
            labels_, 1);
    labelOwner_->matrixVectorMultiplication(secLabel1, dpNoise_);
  }

  std::pair<uint64_t, uint64_t> getTrafficStatistics() override {
    return featureOwner_->getTrafficStatistics();
  }

  std::unique_ptr<engine::communication::IPartyCommunicationAgentFactory>
      agentFactory0_;
  std::unique_ptr<engine::communication::IPartyCommunicationAgentFactory>
      agentFactory1_;

  // feature owner always use scheduler 0, while label owner uses scheduler 1
  std::unique_ptr<IWalrMatrixMultiplicationFactory<0>> featureOwnerFactory_;
  std::unique_ptr<IWalrMatrixMultiplicationFactory<1>> labelOwnerFactory_;

  virtual std::pair<size_t, size_t> dataSize() const = 0;

 private:
  inline void initData(size_t nFeatures, size_t nLabels) {
    std::random_device rd;
    std::mt19937_64 e(rd());
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    // generate feature matrix
    features_.reserve(nLabels);
    for (int i = 0; i < nLabels; ++i) {
      std::vector<double> column(nFeatures);
      std::generate(
          column.begin(), column.end(), [&dist, &e]() { return dist(e); });
      features_.push_back(column);
    }

    // generate label vector
    labels_ = mpc_std_lib::util::generateRandomBinary(nLabels);

    // generate DP noise
    dpNoise_.reserve(nFeatures);
    std::generate_n(std::back_inserter(dpNoise_), nFeatures, [&dist, &e]() {
      return dist(e);
    });
  }

  std::vector<std::vector<double>> features_;
  std::vector<bool> labels_;
  std::vector<double> dpNoise_;

  std::unique_ptr<IWalrMatrixMultiplication<0>> featureOwner_;
  std::unique_ptr<IWalrMatrixMultiplication<1>> labelOwner_;
};

class DummyMatrixMultiplicationBenchmark
    : public WalrMatrixMultiplicationBenchmark {
 public:
  void setup() override {
    WalrMatrixMultiplicationBenchmark::setup();
    auto [agentFactory0, agentFactory1] =
        engine::util::getSocketAgentFactories();
    senderAgentFactory_ = std::move(agentFactory0);
    receiverAgentFactory_ = std::move(agentFactory1);

    featureOwnerFactory_ =
        std::make_unique<insecure::DummyMatrixMultiplicationFactory<0>>(
            0,
            1,
            *senderAgentFactory_,
            std::make_shared<fbpcf::util::MetricCollector>(
                "dummy_matrix_multiplication_benchmark_0"));
    labelOwnerFactory_ =
        std::make_unique<insecure::DummyMatrixMultiplicationFactory<1>>(
            1,
            0,
            *receiverAgentFactory_,
            std::make_shared<fbpcf::util::MetricCollector>(
                "dummy_matrix_multiplication_benchmark_1"));
  }

 protected:
  std::pair<size_t, size_t> dataSize() const override {
    return {100, 5000};
  }

 private:
  std::unique_ptr<engine::communication::IPartyCommunicationAgentFactory>
      senderAgentFactory_;
  std::unique_ptr<engine::communication::IPartyCommunicationAgentFactory>
      receiverAgentFactory_;
};

class OTBasedMatrixMultiplicationBenchmark
    : public WalrMatrixMultiplicationBenchmark {
 public:
  void setup() override {
    WalrMatrixMultiplicationBenchmark::setup();
    auto [agentFactory0, agentFactory1] =
        engine::util::getSocketAgentFactories();
    senderAgentFactory_ = std::move(agentFactory0);
    receiverAgentFactory_ = std::move(agentFactory1);

    constexpr uint64_t divisor = static_cast<uint64_t>(1e9);

    auto prgFactory0 = std::make_unique<engine::util::AesPrgFactory>();
    auto prgFactory1 = std::make_unique<engine::util::AesPrgFactory>();

    auto cotWRMFactory0 =
        std::make_unique<util::COTWithRandomMessageFactory>(getRcotFactory());
    auto cotWRMFactory1 =
        std::make_unique<util::COTWithRandomMessageFactory>(getRcotFactory());

    featureOwnerFactory_ =
        std::make_unique<OTBasedMatrixMultiplicationFactory<0, uint64_t>>(
            0,
            1,
            true,
            divisor,
            *senderAgentFactory_,
            std::move(prgFactory0),
            std::move(cotWRMFactory0),
            std::make_shared<fbpcf::util::MetricCollector>(
                "ot_based_matrix_multiplication_benchmark_0"));
    labelOwnerFactory_ =
        std::make_unique<OTBasedMatrixMultiplicationFactory<1, uint64_t>>(
            1,
            0,
            false,
            divisor,
            *receiverAgentFactory_,
            std::move(prgFactory1),
            std::move(cotWRMFactory1),
            std::make_shared<fbpcf::util::MetricCollector>(
                "ot_based_matrix_multiplication_benchmark_1"));
  }

 protected:
  std::pair<size_t, size_t> dataSize() const override {
    return {100, 5000};
  }

  virtual std::unique_ptr<engine::tuple_generator::oblivious_transfer::
                              IRandomCorrelatedObliviousTransferFactory>
  getRcotFactory() = 0;

 private:
  std::unique_ptr<engine::communication::IPartyCommunicationAgentFactory>
      senderAgentFactory_;
  std::unique_ptr<engine::communication::IPartyCommunicationAgentFactory>
      receiverAgentFactory_;
};

class OTBasedMatrixMultiplicationWithDummyRCOTBenchmark final
    : public OTBasedMatrixMultiplicationBenchmark {
 protected:
  std::unique_ptr<engine::tuple_generator::oblivious_transfer::
                      IRandomCorrelatedObliviousTransferFactory>
  getRcotFactory() override {
    return std::make_unique<
        engine::tuple_generator::oblivious_transfer::insecure::
            DummyRandomCorrelatedObliviousTransferFactory>();
  }
};

class OTBasedMatrixMultiplicationWithEmpFerretBenchmark final
    : public OTBasedMatrixMultiplicationBenchmark {
 protected:
  std::unique_ptr<engine::tuple_generator::oblivious_transfer::
                      IRandomCorrelatedObliviousTransferFactory>
  getRcotFactory() override {
    return std::make_unique<
        engine::tuple_generator::oblivious_transfer::
            ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
        std::make_unique<engine::tuple_generator::oblivious_transfer::
                             EmpShRandomCorrelatedObliviousTransferFactory>(
            std::make_unique<engine::util::AesPrgFactory>(1024)),
        std::make_unique<engine::tuple_generator::oblivious_transfer::ferret::
                             RcotExtenderFactory>(
            std::make_unique<
                engine::tuple_generator::oblivious_transfer::ferret::
                    TenLocalLinearMatrixMultiplierFactory>(),
            std::make_unique<engine::tuple_generator::oblivious_transfer::
                                 ferret::RegularErrorMultiPointCotFactory>(
                std::make_unique<engine::tuple_generator::oblivious_transfer::
                                     ferret::SinglePointCotFactory>())),
        engine::tuple_generator::oblivious_transfer::ferret::kExtendedSize,
        engine::tuple_generator::oblivious_transfer::ferret::kBaseSize,
        engine::tuple_generator::oblivious_transfer::ferret::kWeight);
  }
};

class OTBasedMatrixMultiplicationWithIknpFerretBenchmark final
    : public OTBasedMatrixMultiplicationBenchmark {
 protected:
  std::unique_ptr<engine::tuple_generator::oblivious_transfer::
                      IRandomCorrelatedObliviousTransferFactory>
  getRcotFactory() override {
    return std::make_unique<
        engine::tuple_generator::oblivious_transfer::
            ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
        std::make_unique<engine::tuple_generator::oblivious_transfer::
                             IknpShRandomCorrelatedObliviousTransferFactory>(
            std::make_unique<engine::tuple_generator::oblivious_transfer::
                                 NpBaseObliviousTransferFactory>()),
        std::make_unique<engine::tuple_generator::oblivious_transfer::ferret::
                             RcotExtenderFactory>(
            std::make_unique<
                engine::tuple_generator::oblivious_transfer::ferret::
                    TenLocalLinearMatrixMultiplierFactory>(),
            std::make_unique<engine::tuple_generator::oblivious_transfer::
                                 ferret::RegularErrorMultiPointCotFactory>(
                std::make_unique<engine::tuple_generator::oblivious_transfer::
                                     ferret::SinglePointCotFactory>())),
        engine::tuple_generator::oblivious_transfer::ferret::kExtendedSize,
        engine::tuple_generator::oblivious_transfer::ferret::kBaseSize,
        engine::tuple_generator::oblivious_transfer::ferret::kWeight);
  }
};

BENCHMARK_COUNTERS(DummyMatrixMultiplication, counters) {
  DummyMatrixMultiplicationBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

BENCHMARK_COUNTERS(OTBasedMatrixMultiplicationWithDummyRCOT, counters) {
  OTBasedMatrixMultiplicationWithDummyRCOTBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

BENCHMARK_COUNTERS(OTBasedMatrixMultiplicationWithIknpFerret, counters) {
  OTBasedMatrixMultiplicationWithIknpFerretBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

BENCHMARK_COUNTERS(OTBasedMatrixMultiplicationWithEmpFerret, counters) {
  OTBasedMatrixMultiplicationWithEmpFerretBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

} // namespace fbpcf::mpc_std_lib::walr

int main(int argc, char* argv[]) {
  facebook::initFacebook(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
