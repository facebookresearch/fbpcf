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

#include "fbpcf/mpc_framework/engine/util/AesPrg.h"
#include "fbpcf/mpc_framework/engine/util/aes.h"
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

BENCHMARK(Aes_encryptInPlace, n) {
  folly::BenchmarkSuspender braces;
  auto seed = getRandomSeed();
  auto data = generateData();
  braces.dismiss();

  Aes cipher(seed);

  while (n--) {
    cipher.encryptInPlace(data);
  }
  folly::doNotOptimizeAway(data);
}

BENCHMARK(Aes_inPlaceHash, n) {
  folly::BenchmarkSuspender braces;
  auto seed = getRandomSeed();
  auto data = generateData();
  braces.dismiss();

  Aes cipher(seed);

  while (n--) {
    cipher.inPlaceHash(data);
  }
  folly::doNotOptimizeAway(data);
}

BENCHMARK(AesPrg_getRandomBits, n) {
  folly::BenchmarkSuspender braces;
  auto seed = getRandomSeed();
  braces.dismiss();

  while (n--) {
    // Initialize the prg within the loop to ensure that the initial buffer
    // generation is measured.
    AesPrg prg(seed, FLAGS_AES_Benchmark_Size);
    prg.getRandomBits(FLAGS_AES_Benchmark_Size * 32);
  }
}

BENCHMARK(AesPrg_getRandomBytes, n) {
  folly::BenchmarkSuspender braces;
  auto seed = getRandomSeed();
  braces.dismiss();

  while (n--) {
    // Initialize the prg within the loop to ensure that the initial buffer
    // generation is measured.
    AesPrg prg(seed, FLAGS_AES_Benchmark_Size);
    prg.getRandomBytes(FLAGS_AES_Benchmark_Size * 4);
  }
}

BENCHMARK(AesPrg_getRandomDataInPlace, n) {
  folly::BenchmarkSuspender braces;
  auto seed = getRandomSeed();
  auto data = generateData();
  braces.dismiss();

  AesPrg prg(seed);
  while (n--) {
    prg.getRandomDataInPlace(data);
  }
  folly::doNotOptimizeAway(data);
}
} // namespace fbpcf::engine::util

int main(int argc, char* argv[]) {
  facebook::initFacebook(&argc, &argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  folly::runBenchmarks();
  return 0;
}
