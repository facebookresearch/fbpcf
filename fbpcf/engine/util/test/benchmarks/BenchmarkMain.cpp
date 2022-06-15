/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Benchmark.h>
#include <algorithm>
#include <random>
#include <vector>
#include "common/init/Init.h"

#include "fbpcf/engine/util/AesPrg.h"
#include "fbpcf/engine/util/aes.h"
#include "fbpcf/engine/util/test/benchmarks/LocalBenchmark.h"
#include "folly/BenchmarkUtil.h"

namespace fbpcf::engine::util {

DEFINE_int64(
    AES_Benchmark_Size,
    1024,
    "How many blocks are used to benchmark AESNI");

__m128i getRandomSeed() {
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);

  __m128i seed = _mm_set_epi64x(dist(e), dist(e));
  return seed;
}

std::vector<__m128i> generateData() {
  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);

  std::vector<__m128i> data(FLAGS_AES_Benchmark_Size);

  // for benchmark, the concrete value doesn't matter;
  for (auto& item : data) {
    item = _mm_set_epi64x(dist(e), dist(e));
  }
  return data;
}

class AesBenchmark : public LocalBenchmark {
 public:
  void setup() override {
    seed = getRandomSeed();
    data = generateData();
    cipher = std::make_unique<Aes>(seed);
    prg = std::make_unique<AesPrg>(seed);
  }

  void teardown() override {
    folly::doNotOptimizeAway(data);
  }

 protected:
  __m128i seed;
  std::vector<__m128i> data;
  std::unique_ptr<Aes> cipher;
  std::unique_ptr<AesPrg> prg;
};

class AesEncryptInPlaceBenchmark final : public AesBenchmark {
 public:
  void run(unsigned int n) override {
    while (n--) {
      cipher->encryptInPlace(data);
    }
  }
};

BENCHMARK(Aes_encryptInPlace, n) {
  AesEncryptInPlaceBenchmark benchmark;
  benchmark.runBenchmark(n);
}

class AesInPlaceHashBenchmark final : public AesBenchmark {
 public:
  void run(unsigned int n) override {
    while (n--) {
      cipher->inPlaceHash(data);
    }
  }
};

BENCHMARK(Aes_inPlaceHash, n) {
  AesInPlaceHashBenchmark benchmark;
  benchmark.runBenchmark(n);
}

class AesPrgGetRandomBitsBenchmark final : public AesBenchmark {
 public:
  void run(unsigned int n) override {
    while (n--) {
      // Initialize the prg within the loop to ensure that the initial buffer
      // generation is measured.
      AesPrg prg(seed, FLAGS_AES_Benchmark_Size);
      prg.getRandomBits(FLAGS_AES_Benchmark_Size * 32);
    }
  }
};

BENCHMARK(AesPrg_getRandomBits, n) {
  AesPrgGetRandomBitsBenchmark benchmark;
  benchmark.runBenchmark(n);
}

class AesPrgGetRandomBytesBenchmark final : public AesBenchmark {
 public:
  void run(unsigned int n) override {
    while (n--) {
      // Initialize the prg within the loop to ensure that the initial buffer
      // generation is measured.
      AesPrg prg(seed, FLAGS_AES_Benchmark_Size);
      prg.getRandomBytes(FLAGS_AES_Benchmark_Size * 4);
    }
  }
};

BENCHMARK(AesPrg_getRandomBytes, n) {
  AesPrgGetRandomBytesBenchmark benchmark;
  benchmark.runBenchmark(n);
}

class AesPrgGetRandomDataInPlaceBenchmark final : public AesBenchmark {
 public:
  void run(unsigned int n) override {
    while (n--) {
      prg->getRandomDataInPlace(data);
    }
  }
};

BENCHMARK(AesPrg_getRandomDataInPlace, n) {
  AesPrgGetRandomDataInPlaceBenchmark benchmark;
  benchmark.runBenchmark(n);
}
} // namespace fbpcf::engine::util

int main(int argc, char* argv[]) {
  facebook::initFacebook(&argc, &argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  folly::runBenchmarks();
  return 0;
}
