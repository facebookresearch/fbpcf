/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Benchmark.h>

#include "common/init/Init.h"

#include "fbpcf/engine/SecretShareEngineFactory.h"
#include "fbpcf/engine/util/test/benchmarks/BenchmarkHelper.h"
#include "fbpcf/engine/util/test/benchmarks/NetworkedBenchmark.h"

namespace fbpcf::engine {

class BaseSecretShareEngineBenchmark : public util::NetworkedBenchmark {
 public:
  void setup() override {
    auto [agentFactory0, agentFactory1] = util::getSocketAgentFactories();
    agentFactory0_ = std::move(agentFactory0);
    agentFactory1_ = std::move(agentFactory1);

    // We intentionally use a dummy tuple generator here to measure only the
    // network traffic incurred by the secret share engine.
    senderFactory_ =
        getInsecureEngineFactoryWithDummyTupleGenerator(0, 2, *agentFactory0_);
    receiverFactory_ =
        getInsecureEngineFactoryWithDummyTupleGenerator(1, 2, *agentFactory1_);

    // Set up randomized inputs
    batchInput0_ = util::getRandomBoolVector(batchSize_);
    batchInput1_ = util::getRandomBoolVector(batchSize_);

    input0_ = batchInput0_.at(0);
    input1_ = batchInput1_.at(0);

    randomParty_ = batchInput0_.at(1);
  }

 protected:
  void initSender() override {
    sender_ = senderFactory_->create();
  }

  void runSender() override {
    runMethod(sender_);
  }

  void initReceiver() override {
    receiver_ = receiverFactory_->create();
  }

  void runReceiver() override {
    runMethod(receiver_);
  }

  std::pair<uint64_t, uint64_t> getTrafficStatistics() override {
    return sender_->getTrafficStatistics();
  }

  virtual void runMethod(std::unique_ptr<ISecretShareEngine>& engine) = 0;

  size_t batchSize_ = 1000;
  std::vector<bool> batchInput0_;
  std::vector<bool> batchInput1_;

  bool input0_;
  bool input1_;

  int randomParty_;

 private:
  std::unique_ptr<communication::IPartyCommunicationAgentFactory>
      agentFactory0_;
  std::unique_ptr<communication::IPartyCommunicationAgentFactory>
      agentFactory1_;

  std::unique_ptr<ISecretShareEngineFactory> senderFactory_;
  std::unique_ptr<ISecretShareEngineFactory> receiverFactory_;

  std::unique_ptr<ISecretShareEngine> sender_;
  std::unique_ptr<ISecretShareEngine> receiver_;
};

class SetInputBenchmark final : public BaseSecretShareEngineBenchmark {
 protected:
  virtual void runMethod(std::unique_ptr<ISecretShareEngine>& engine) override {
    engine->setInput(randomParty_, input0_);
  }
};

BENCHMARK_COUNTERS(SetInput, counters) {
  SetInputBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

class SetBatchInputBenchmark final : public BaseSecretShareEngineBenchmark {
 protected:
  virtual void runMethod(std::unique_ptr<ISecretShareEngine>& engine) override {
    engine->setBatchInput(randomParty_, batchInput0_);
  }
};

BENCHMARK_COUNTERS(SetBatchInput, counters) {
  SetBatchInputBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

class ComputeSymmetricXORBenchmark final
    : public BaseSecretShareEngineBenchmark {
 protected:
  virtual void runMethod(std::unique_ptr<ISecretShareEngine>& engine) override {
    engine->computeSymmetricXOR(input0_, input1_);
  }
};

BENCHMARK_COUNTERS(ComputeSymmetricXOR, counters) {
  ComputeSymmetricXORBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

class ComputeBatchSymmetricXORBenchmark final
    : public BaseSecretShareEngineBenchmark {
 protected:
  virtual void runMethod(std::unique_ptr<ISecretShareEngine>& engine) override {
    engine->computeBatchSymmetricXOR(batchInput0_, batchInput1_);
  }
};

BENCHMARK_COUNTERS(ComputeBatchSymmetricXOR, counters) {
  ComputeBatchSymmetricXORBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

class ComputeAsymmetricXORBenchmark final
    : public BaseSecretShareEngineBenchmark {
 protected:
  virtual void runMethod(std::unique_ptr<ISecretShareEngine>& engine) override {
    engine->computeAsymmetricXOR(input0_, input1_);
  }
};

BENCHMARK_COUNTERS(ComputeAsymmetricXOR, counters) {
  ComputeAsymmetricXORBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

class ComputeBatchAsymmetricXORBenchmark final
    : public BaseSecretShareEngineBenchmark {
 protected:
  virtual void runMethod(std::unique_ptr<ISecretShareEngine>& engine) override {
    engine->computeBatchAsymmetricXOR(batchInput0_, batchInput1_);
  }
};

BENCHMARK_COUNTERS(ComputeBatchAsymmetricXOR, counters) {
  ComputeBatchAsymmetricXORBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

class ComputeSymmetricNOTBenchmark final
    : public BaseSecretShareEngineBenchmark {
 protected:
  virtual void runMethod(std::unique_ptr<ISecretShareEngine>& engine) override {
    engine->computeSymmetricNOT(input0_);
  }
};

BENCHMARK_COUNTERS(ComputeSymmetricNOT, counters) {
  ComputeSymmetricNOTBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

class ComputeBatchSymmetricNOTBenchmark final
    : public BaseSecretShareEngineBenchmark {
 protected:
  virtual void runMethod(std::unique_ptr<ISecretShareEngine>& engine) override {
    engine->computeBatchSymmetricNOT(batchInput0_);
  }
};

BENCHMARK_COUNTERS(ComputeBatchSymmetricNOT, counters) {
  ComputeBatchSymmetricNOTBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

class ComputeAsymmetricNOTBenchmark final
    : public BaseSecretShareEngineBenchmark {
 protected:
  virtual void runMethod(std::unique_ptr<ISecretShareEngine>& engine) override {
    engine->computeAsymmetricNOT(input0_);
  }
};

BENCHMARK_COUNTERS(ComputeAsymmetricNOT, counters) {
  ComputeAsymmetricNOTBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

class ComputeBatchAsymmetricNOTBenchmark final
    : public BaseSecretShareEngineBenchmark {
 protected:
  virtual void runMethod(std::unique_ptr<ISecretShareEngine>& engine) override {
    engine->computeBatchAsymmetricNOT(batchInput0_);
  }
};

BENCHMARK_COUNTERS(ComputeBatchAsymmetricNOT, counters) {
  ComputeBatchAsymmetricNOTBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

class ComputeFreeANDBenchmark final : public BaseSecretShareEngineBenchmark {
 protected:
  virtual void runMethod(std::unique_ptr<ISecretShareEngine>& engine) override {
    engine->computeFreeAND(input0_, input1_);
  }
};

BENCHMARK_COUNTERS(ComputeFreeAND, counters) {
  ComputeFreeANDBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

class ComputeBatchFreeANDBenchmark final
    : public BaseSecretShareEngineBenchmark {
 protected:
  virtual void runMethod(std::unique_ptr<ISecretShareEngine>& engine) override {
    engine->computeBatchFreeAND(batchInput0_, batchInput1_);
  }
};

BENCHMARK_COUNTERS(ComputeBatchFreeAND, counters) {
  ComputeBatchFreeANDBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

class ComputeNonFreeANDBenchmark final : public BaseSecretShareEngineBenchmark {
 protected:
  virtual void runMethod(std::unique_ptr<ISecretShareEngine>& engine) override {
    auto index = engine->scheduleAND(input0_, input1_);
    engine->executeScheduledOperations();
    engine->getANDExecutionResult(index);
  }
};

BENCHMARK_COUNTERS(ComputeNonFreeAND, counters) {
  ComputeNonFreeANDBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

class ComputeBatchNonFreeANDBenchmark final
    : public BaseSecretShareEngineBenchmark {
 protected:
  virtual void runMethod(std::unique_ptr<ISecretShareEngine>& engine) override {
    auto index = engine->scheduleBatchAND(batchInput0_, batchInput1_);
    engine->executeScheduledOperations();
    engine->getBatchANDExecutionResult(index);
  }
};

BENCHMARK_COUNTERS(ComputeBatchNonFreeAND, counters) {
  ComputeBatchNonFreeANDBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

class ComputeCompositeANDBenchmark final
    : public BaseSecretShareEngineBenchmark {
 protected:
  virtual void runMethod(std::unique_ptr<ISecretShareEngine>& engine) override {
    auto index = engine->scheduleCompositeAND(input0_, batchInput0_);
    engine->executeScheduledOperations();
    engine->getCompositeANDExecutionResult(index);
  }
};

BENCHMARK_COUNTERS(ComputeCompositeAND, counters) {
  ComputeCompositeANDBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

class ComputeBatchCompositeANDBenchmark final
    : public BaseSecretShareEngineBenchmark {
 protected:
  virtual void runMethod(std::unique_ptr<ISecretShareEngine>& engine) override {
    auto index = engine->scheduleBatchCompositeAND(
        batchInput0_, std::vector<std::vector<bool>>(batchSize_, batchInput1_));
    engine->executeScheduledOperations();
    engine->getBatchCompositeANDExecutionResult(index);
  }
};

BENCHMARK_COUNTERS(ComputeBatchCompositeAND, counters) {
  ComputeBatchCompositeANDBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

class RevealToPartyBenchmark final : public BaseSecretShareEngineBenchmark {
 protected:
  virtual void runMethod(std::unique_ptr<ISecretShareEngine>& engine) override {
    engine->revealToParty(randomParty_, batchInput0_);
  }
};

BENCHMARK_COUNTERS(RevealToParty, counters) {
  RevealToPartyBenchmark benchmark;
  benchmark.runBenchmark(counters);
}
} // namespace fbpcf::engine

int main(int argc, char* argv[]) {
  facebook::initFacebook(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
