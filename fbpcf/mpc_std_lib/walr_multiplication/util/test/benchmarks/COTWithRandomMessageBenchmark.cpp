/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/types.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <future>
#include <memory>
#include <random>
#include <tuple>
#include <vector>

#include "common/init/Init.h"

#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
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
#include "fbpcf/mpc_std_lib/walr_multiplication/util/COTWithRandomMessageFactory.h"

namespace fbpcf::mpc_std_lib::walr::util {

class COTWithRandomMessageBenchmark : public engine::util::NetworkedBenchmark {
 public:
  void setup() override {
    auto [agent0, agent1] = engine::util::getSocketAgents();
    agent0_ = std::move(agent0);
    agent1_ = std::move(agent1);

    auto [rcotAgent0, rcotAgent1] = engine::util::getSocketAgents();
    rcotAgent0_ = std::move(rcotAgent0);
    rcotAgent1_ = std::move(rcotAgent1);

    std::random_device rd;
    std::mt19937_64 e(rd());
    std::uniform_int_distribution<uint64_t> dist(0, 0xFFFFFFFFFFFFFFFF);
    delta_ = _mm_set_epi64x(dist(e), dist(e));
    engine::util::setLsbTo1(delta_);

    choice_ = engine::util::getRandomBoolVector(size_);

    auto prgFactory0 = std::make_unique<engine::util::AesPrgFactory>();
    auto prgFactory1 = std::make_unique<engine::util::AesPrgFactory>();

    factory_ = std::make_unique<COTWithRandomMessageFactory>(getRcotFactory());
  }

 protected:
  void initSender() override {
    sender_ =
        factory_->create(delta_, std::move(agent0_), std::move(rcotAgent0_));
  }

  void runSender() override {
    sender_->send(size_);
  }

  void initReceiver() override {
    receiver_ = factory_->create(std::move(agent1_), std::move(rcotAgent1_));
  }

  void runReceiver() override {
    receiver_->receive(choice_);
  }

  std::pair<uint64_t, uint64_t> getTrafficStatistics() override {
    return sender_->getTrafficStatistics();
  }

  virtual std::unique_ptr<engine::tuple_generator::oblivious_transfer::
                              IRandomCorrelatedObliviousTransferFactory>
  getRcotFactory() = 0;

  std::unique_ptr<COTWithRandomMessageFactory> factory_;

 private:
  const size_t size_ = 1000000;
  __m128i delta_;
  std::vector<bool> choice_;

  std::unique_ptr<engine::communication::IPartyCommunicationAgent> agent0_;
  std::unique_ptr<engine::communication::IPartyCommunicationAgent> agent1_;

  std::unique_ptr<engine::communication::IPartyCommunicationAgent> rcotAgent0_;
  std::unique_ptr<engine::communication::IPartyCommunicationAgent> rcotAgent1_;

  std::unique_ptr<COTWithRandomMessage> sender_;
  std::unique_ptr<COTWithRandomMessage> receiver_;
};

class DummyRCOTBasedCOTwRMBenchmark final
    : public COTWithRandomMessageBenchmark {
 protected:
  std::unique_ptr<engine::tuple_generator::oblivious_transfer::
                      IRandomCorrelatedObliviousTransferFactory>
  getRcotFactory() override {
    return std::make_unique<
        engine::tuple_generator::oblivious_transfer::insecure::
            DummyRandomCorrelatedObliviousTransferFactory>();
  }
};

class EmpFerretBasedCOTwRMBenchmark final
    : public COTWithRandomMessageBenchmark {
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

class IknpFerretBasedCOTwRMBenchmark final
    : public COTWithRandomMessageBenchmark {
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

BENCHMARK_COUNTERS(DummyRCOTBasedCOTwRM, counters) {
  DummyRCOTBasedCOTwRMBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

BENCHMARK_COUNTERS(EmpFerretBasedCOTwRM, counters) {
  EmpFerretBasedCOTwRMBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

BENCHMARK_COUNTERS(IknpFerretBasedCOTwRM, counters) {
  IknpFerretBasedCOTwRMBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

} // namespace fbpcf::mpc_std_lib::walr::util

int main(int argc, char* argv[]) {
  facebook::initFacebook(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
