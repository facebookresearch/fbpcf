/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Benchmark.h>
#include <random>

#include "common/init/Init.h"

#include "fbpcf/engine/tuple_generator/oblivious_transfer/IBaseObliviousTransfer.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/NpBaseObliviousTransferFactory.h"
#include "fbpcf/engine/util/test/benchmarks/BenchmarkHelper.h"
#include "fbpcf/engine/util/test/benchmarks/NetworkedBenchmark.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer {

class NpBaseObliviousTransferBenchmark : public util::NetworkedBenchmark {
 public:
  void setup() override {
    auto [agent0, agent1] = util::getSocketAgents();

    NpBaseObliviousTransferFactory factory;
    sender_ = factory.create(std::move(agent0));
    receiver_ = factory.create(std::move(agent1));

    std::random_device rd;
    std::mt19937_64 e(rd());
    std::uniform_int_distribution<uint8_t> randomChoice(0, 1);

    choice_ = std::vector<bool>(size_);
    for (auto i = 0; i < size_; ++i) {
      choice_[i] = randomChoice(e);
    }
  }

  void runSender() override {
    sender_->send(size_);
  }

  void runReceiver() override {
    receiver_->receive(choice_);
  }

  std::pair<uint64_t, uint64_t> getTrafficStatistics() override {
    return sender_->getTrafficStatistics();
  }

 private:
  size_t size_ = 1024;

  std::unique_ptr<IBaseObliviousTransfer> sender_;
  std::unique_ptr<IBaseObliviousTransfer> receiver_;

  std::vector<bool> choice_;
};

BENCHMARK_COUNTERS(NpBaseObliviousTransfer, counters) {
  NpBaseObliviousTransferBenchmark benchmark;
  benchmark.runBenchmark(counters);
}
} // namespace fbpcf::engine::tuple_generator::oblivious_transfer

int main(int argc, char* argv[]) {
  facebook::initFacebook(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
