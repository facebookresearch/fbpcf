/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Benchmark.h>
#include <future>
#include <random>

#include "common/init/Init.h"

#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/IMultiPointCot.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/ISinglePointCot.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/RcotExtenderFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/RegularErrorMultiPointCot.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/RegularErrorMultiPointCotFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/SinglePointCotFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/TenLocalLinearMatrixMultiplierFactory.h"
#include "fbpcf/mpc_framework/engine/util/test/benchmarks/BenchmarkHelper.h"
#include "fbpcf/mpc_framework/engine/util/test/benchmarks/NetworkedBenchmark.h"
#include "fbpcf/mpc_framework/engine/util/util.h"

namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer::
    ferret {

std::tuple<std::vector<__m128i>, std::vector<__m128i>, __m128i> getBaseOT(
    size_t baseOtSize) {
  std::vector<__m128i> baseOTSend(baseOtSize);
  std::vector<__m128i> baseOTReceive(baseOtSize);

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint64_t> dist(0, 0xFFFFFFFFFFFFFFFF);
  std::uniform_int_distribution<uint32_t> randomChoice(0, 1);

  __m128i delta = _mm_set_epi64x(dist(e), dist(e));
  util::setLsbTo1(delta);

  for (int i = 0; i < baseOtSize; i++) {
    baseOTSend[i] = _mm_set_epi64x(dist(e), dist(e));
    util::setLsbTo0(baseOTSend[i]);
    baseOTReceive[i] = baseOTSend[i];

    if (randomChoice(e)) {
      baseOTReceive[i] = _mm_xor_si128(baseOTReceive[i], delta);
    }
  }

  return {baseOTSend, baseOTReceive, delta};
}

class SinglePointCotBenchmark final : public util::NetworkedBenchmark {
 public:
  void setup() override {
    auto [agent0, agent1] = util::getSocketAgents();
    agent0_ = std::move(agent0);
    agent1_ = std::move(agent1);

    SinglePointCotFactory factory;
    sender_ = factory.create(agent0_);
    receiver_ = factory.create(agent1_);

    auto baseOtSize = std::log2(kExtendedSize / kWeight);
    auto [baseOTSend, baseOTReceive, delta] = getBaseOT(baseOtSize);
    baseOTSend_ = std::move(baseOTSend);
    baseOTReceive_ = std::move(baseOTReceive);

    sender_->senderInit(delta);
    receiver_->receiverInit();
  }

 protected:
  void runSender() override {
    sender_->senderExtend(std::move(baseOTSend_));
  }

  void runReceiver() override {
    receiver_->receiverExtend(std::move(baseOTReceive_));
  }

  std::pair<uint64_t, uint64_t> getTrafficStatistics() override {
    return agent0_->getTrafficStatistics();
  }

 private:
  std::unique_ptr<communication::IPartyCommunicationAgent> agent0_;
  std::unique_ptr<communication::IPartyCommunicationAgent> agent1_;

  std::unique_ptr<ISinglePointCot> sender_;
  std::unique_ptr<ISinglePointCot> receiver_;

  std::vector<__m128i> baseOTSend_;
  std::vector<__m128i> baseOTReceive_;
};

BENCHMARK_COUNTERS(SinglePointCot, counters) {
  SinglePointCotBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

class MultiPointCotBenchmark final : public util::NetworkedBenchmark {
 public:
  void setup() override {
    auto [agent0, agent1] = util::getSocketAgents();
    agent0_ = std::move(agent0);
    agent1_ = std::move(agent1);

    RegularErrorMultiPointCotFactory factory(
        std::make_unique<SinglePointCotFactory>());

    sender_ = factory.create(agent0_);
    receiver_ = factory.create(agent1_);

    auto baseOtSize = std::log2(kExtendedSize / kWeight) * kWeight;
    auto [baseOTSend, baseOTReceive, delta] = getBaseOT(baseOtSize);
    baseOTSend_ = std::move(baseOTSend);
    baseOTReceive_ = std::move(baseOTReceive);

    sender_->senderInit(delta, kExtendedSize, kWeight);
    receiver_->receiverInit(kExtendedSize, kWeight);
  }

 protected:
  void runSender() override {
    sender_->senderExtend(std::move(baseOTSend_));
  }

  void runReceiver() override {
    receiver_->receiverExtend(std::move(baseOTReceive_));
  }

  std::pair<uint64_t, uint64_t> getTrafficStatistics() override {
    return agent0_->getTrafficStatistics();
  }

 private:
  std::unique_ptr<communication::IPartyCommunicationAgent> agent0_;
  std::unique_ptr<communication::IPartyCommunicationAgent> agent1_;

  std::unique_ptr<IMultiPointCot> sender_;
  std::unique_ptr<IMultiPointCot> receiver_;

  std::vector<__m128i> baseOTSend_;
  std::vector<__m128i> baseOTReceive_;
};

BENCHMARK_COUNTERS(MultiPointCot, counters) {
  MultiPointCotBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

class RcotExtenderBenchmark final : public util::NetworkedBenchmark {
 public:
  void setup() override {
    auto [agent0, agent1] = util::getSocketAgents();

    RcotExtenderFactory factory(
        std::make_unique<TenLocalLinearMatrixMultiplierFactory>(),
        std::make_unique<RegularErrorMultiPointCotFactory>(
            std::make_unique<SinglePointCotFactory>()));

    sender_ = factory.create();
    receiver_ = factory.create();

    sender_->setCommunicationAgent(std::move(agent0));
    receiver_->setCommunicationAgent(std::move(agent1));

    auto baseOtSize = kBaseSize + std::log2(kExtendedSize / kWeight) * kWeight;
    auto [baseOTSend, baseOTReceive, delta] = getBaseOT(baseOtSize);
    baseOTSend_ = std::move(baseOTSend);
    baseOTReceive_ = std::move(baseOTReceive);

    sender_->senderInit(delta, kExtendedSize, kBaseSize, kWeight);
    receiver_->receiverInit(kExtendedSize, kBaseSize, kWeight);
  }

 protected:
  void runSender() override {
    sender_->senderExtendRcot(std::move(baseOTSend_));
  }

  void runReceiver() override {
    receiver_->receiverExtendRcot(std::move(baseOTReceive_));
  }

  std::pair<uint64_t, uint64_t> getTrafficStatistics() override {
    return sender_->getTrafficStatistics();
  }

 private:
  std::unique_ptr<IRcotExtender> sender_;
  std::unique_ptr<IRcotExtender> receiver_;

  std::vector<__m128i> baseOTSend_;
  std::vector<__m128i> baseOTReceive_;
};

BENCHMARK_COUNTERS(RcotExtender, counters) {
  RcotExtenderBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

} // namespace
  // fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer::ferret

int main(int argc, char* argv[]) {
  facebook::initFacebook(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
