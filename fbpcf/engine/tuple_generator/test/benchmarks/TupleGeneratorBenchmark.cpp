/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Benchmark.h>

#include "common/init/Init.h"

#include "fbpcf/engine/tuple_generator/ProductShareGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/RcotBasedBidirectionObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/RcotHelper.h"
#include "fbpcf/engine/tuple_generator/test/TupleGeneratorTestHelper.h"
#include "fbpcf/engine/util/AesPrgFactory.h"
#include "fbpcf/engine/util/test/benchmarks/BenchmarkHelper.h"
#include "fbpcf/engine/util/test/benchmarks/NetworkedBenchmark.h"
#include "fbpcf/util/MetricCollector.h"

namespace fbpcf::engine::tuple_generator {

class ProductShareGeneratorBenchmark : public util::NetworkedBenchmark {
 public:
  void setup() override {
    auto [agentFactory0, agentFactory1] = util::getSocketAgentFactories();
    agentFactory0_ = std::move(agentFactory0);
    agentFactory1_ = std::move(agentFactory1);

    senderFactory_ = std::make_unique<ProductShareGeneratorFactory>(
        std::make_unique<util::AesPrgFactory>(),
        std::make_unique<
            oblivious_transfer::RcotBasedBidirectionObliviousTransferFactory>(
            0, *agentFactory0_, oblivious_transfer::createFerretRcotFactory()));

    receiverFactory_ = std::make_unique<ProductShareGeneratorFactory>(
        std::make_unique<util::AesPrgFactory>(),
        std::make_unique<
            oblivious_transfer::RcotBasedBidirectionObliviousTransferFactory>(
            1, *agentFactory1_, oblivious_transfer::createFerretRcotFactory()));

    senderLeft_ = util::getRandomBoolVector(size_);
    senderRight_ = util::getRandomBoolVector(size_);

    receiverLeft_ = util::getRandomBoolVector(size_);
    receiverRight_ = util::getRandomBoolVector(size_);
  }

 protected:
  void initSender() override {
    sender_ = senderFactory_->create(1);
  }

  void runSender() override {
    sender_->generateBooleanProductShares(senderLeft_, senderRight_);
  }

  void initReceiver() override {
    receiver_ = receiverFactory_->create(0);
  }

  void runReceiver() override {
    receiver_->generateBooleanProductShares(receiverLeft_, receiverRight_);
  }

 private:
  size_t size_ = 10000000;

  std::unique_ptr<communication::IPartyCommunicationAgentFactory>
      agentFactory0_;
  std::unique_ptr<communication::IPartyCommunicationAgentFactory>
      agentFactory1_;

  std::unique_ptr<IProductShareGeneratorFactory> senderFactory_;
  std::unique_ptr<IProductShareGeneratorFactory> receiverFactory_;

  std::unique_ptr<IProductShareGenerator> sender_;
  std::unique_ptr<IProductShareGenerator> receiver_;

  std::vector<bool> senderLeft_;
  std::vector<bool> receiverLeft_;

  std::vector<bool> senderRight_;
  std::vector<bool> receiverRight_;
};

BENCHMARK_COUNTERS(ProductShareGenerator, counters) {
  ProductShareGeneratorBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

class BaseTupleGeneratorBenchmark : public util::NetworkedBenchmark {
 public:
  void setup() override {
    auto [agentFactory0, agentFactory1] = util::getSocketAgentFactories();
    agentFactory0_ = std::move(agentFactory0);
    agentFactory1_ = std::move(agentFactory1);

    senderFactory_ = getTupleGeneratorFactory(0, *agentFactory0_);
    receiverFactory_ = getTupleGeneratorFactory(1, *agentFactory1_);
  }

 protected:
  size_t size_ = 10000000;
  std::unique_ptr<ITupleGenerator> sender_;
  std::unique_ptr<ITupleGenerator> receiver_;

  void initSender() override {
    sender_ = senderFactory_->create();
  }

  void runSender() override {
    sender_->getBooleanTuple(size_);
  }

  void initReceiver() override {
    receiver_ = receiverFactory_->create();
  }

  void runReceiver() override {
    receiver_->getBooleanTuple(size_);
  }

  virtual std::unique_ptr<ITupleGeneratorFactory> getTupleGeneratorFactory(
      int myId,
      communication::IPartyCommunicationAgentFactory& agentFactory) = 0;

  size_t bufferSize_ = 1600000;

 private:
  std::unique_ptr<communication::IPartyCommunicationAgentFactory>
      agentFactory0_;
  std::unique_ptr<communication::IPartyCommunicationAgentFactory>
      agentFactory1_;

  std::unique_ptr<ITupleGeneratorFactory> senderFactory_;
  std::unique_ptr<ITupleGeneratorFactory> receiverFactory_;
};

class TupleGeneratorBenchmark final : public BaseTupleGeneratorBenchmark {
 protected:
  std::unique_ptr<ITupleGeneratorFactory> getTupleGeneratorFactory(
      int myId,
      communication::IPartyCommunicationAgentFactory& agentFactory) override {
    auto otFactory = std::make_unique<
        oblivious_transfer::RcotBasedBidirectionObliviousTransferFactory>(
        myId, agentFactory, oblivious_transfer::createFerretRcotFactory());
    auto productShareGeneratorFactory =
        std::make_unique<ProductShareGeneratorFactory>(
            std::make_unique<util::AesPrgFactory>(bufferSize_),
            std::move(otFactory));
    return std::make_unique<TupleGeneratorFactory>(
        std::move(productShareGeneratorFactory),
        std::make_unique<util::AesPrgFactory>(),
        bufferSize_,
        myId,
        2,
        std::make_shared<fbpcf::util::MetricCollector>(
            "tuple_generator_benchmark"));
  }
};

BENCHMARK_COUNTERS(TupleGenerator, counters) {
  TupleGeneratorBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

class TwoPartyTupleGeneratorBenchmark final
    : public BaseTupleGeneratorBenchmark {
 protected:
  std::unique_ptr<ITupleGeneratorFactory> getTupleGeneratorFactory(
      int myId,
      communication::IPartyCommunicationAgentFactory& agentFactory) override {
    return std::make_unique<TwoPartyTupleGeneratorFactory>(
        oblivious_transfer::createFerretRcotFactory(),
        agentFactory,
        myId,
        bufferSize_,
        std::make_shared<fbpcf::util::MetricCollector>(
            "tuple_generator_benchmark"));
  }
};

BENCHMARK_COUNTERS(TwoPartyTupleGenerator, counters) {
  TwoPartyTupleGeneratorBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

class TwoPartyCompositeTupleGeneratorBenchmark
    : public BaseTupleGeneratorBenchmark {
 public:
  explicit TwoPartyCompositeTupleGeneratorBenchmark(size_t compositeTupleSize) {
    tupleSizes_ = std::map<size_t, uint32_t>{{compositeTupleSize, size_}};
  }

 protected:
  std::unique_ptr<ITupleGeneratorFactory> getTupleGeneratorFactory(
      int myId,
      communication::IPartyCommunicationAgentFactory& agentFactory) override {
    return std::make_unique<TwoPartyTupleGeneratorFactory>(
        oblivious_transfer::createFerretRcotFactory(),
        agentFactory,
        myId,
        bufferSize_,
        std::make_shared<fbpcf::util::MetricCollector>(
            "tuple_generator_benchmark"));
  }
  void runSender() override {
    sender_->getCompositeTuple(tupleSizes_);
  }
  void runReceiver() override {
    receiver_->getCompositeTuple(tupleSizes_);
  }

 private:
  std::map<size_t, uint32_t> tupleSizes_;
};

BENCHMARK_COUNTERS(TwoPartyCompositeTupleGeneratorForSize8, counters) {
  TwoPartyCompositeTupleGeneratorBenchmark benchmark(8);
  benchmark.runBenchmark(counters);
}

BENCHMARK_COUNTERS(TwoPartyCompositeTupleGeneratorForSize16, counters) {
  TwoPartyCompositeTupleGeneratorBenchmark benchmark(16);
  benchmark.runBenchmark(counters);
}

BENCHMARK_COUNTERS(TwoPartyCompositeTupleGeneratorForSize32, counters) {
  TwoPartyCompositeTupleGeneratorBenchmark benchmark(32);
  benchmark.runBenchmark(counters);
}

BENCHMARK_COUNTERS(TwoPartyCompositeTupleGeneratorForSize64, counters) {
  TwoPartyCompositeTupleGeneratorBenchmark benchmark(64);
  benchmark.runBenchmark(counters);
}

BENCHMARK_COUNTERS(TwoPartyCompositeTupleGeneratorForSize128, counters) {
  TwoPartyCompositeTupleGeneratorBenchmark benchmark(128);
  benchmark.runBenchmark(counters);
}

} // namespace fbpcf::engine::tuple_generator

int main(int argc, char* argv[]) {
  facebook::initFacebook(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
