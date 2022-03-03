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

#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/RcotExtenderFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/RegularErrorMultiPointCot.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/RegularErrorMultiPointCotFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/SinglePointCotFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/TenLocalLinearMatrixMultiplierFactory.h"
#include "fbpcf/mpc_framework/engine/util/test/benchmarks/BenchmarkHelper.h"
#include "fbpcf/mpc_framework/engine/util/util.h"

using namespace folly;
using namespace fbpcf::mpc_framework::engine;
using namespace fbpcf::mpc_framework::engine::tuple_generator::
    oblivious_transfer;

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

BENCHMARK_COUNTERS(SinglePointCot, counters) {
  BenchmarkSuspender braces;
  auto [agent0, agent1] = util::getSocketAgents();

  ferret::SinglePointCotFactory factory;
  auto sender = factory.create(agent0);
  auto receiver = factory.create(agent1);

  auto baseOtSize = std::log2(ferret::kExtendedSize / ferret::kWeight);
  auto [baseOTSend, baseOTReceive, delta] = getBaseOT(baseOtSize);

  braces.dismiss();

  auto senderTask = std::async(
      [](std::unique_ptr<ferret::ISinglePointCot> spcot,
         std::vector<__m128i>&& baseCot,
         __m128i delta) {
        spcot->senderInit(delta);
        return spcot->senderExtend(std::move(baseCot));
      },
      std::move(sender),
      std::move(baseOTSend),
      delta);
  auto receiverTask = std::async(
      [](std::unique_ptr<ferret::ISinglePointCot> spcot,
         std::vector<__m128i>&& baseCot) {
        spcot->receiverInit();
        return spcot->receiverExtend(std::move(baseCot));
      },
      std::move(receiver),
      std::move(baseOTReceive));

  senderTask.get();
  receiverTask.get();

  BENCHMARK_SUSPEND {
    auto [senderSent, senderReceived] = agent0->getTrafficStatistics();
    counters["transmitted_bytes"] = senderSent + senderReceived;
  }
}

BENCHMARK_COUNTERS(MultiPointCot, counters) {
  BenchmarkSuspender braces;
  auto [agent0, agent1] = util::getSocketAgents();

  ferret::RegularErrorMultiPointCotFactory factory(
      std::make_unique<ferret::SinglePointCotFactory>());

  auto sender = factory.create(agent0);
  auto receiver = factory.create(agent1);

  auto baseOtSize =
      std::log2(ferret::kExtendedSize / ferret::kWeight) * ferret::kWeight;
  auto [baseOTSend, baseOTReceive, delta] = getBaseOT(baseOtSize);

  braces.dismiss();

  auto senderTask = std::async(
      [](std::unique_ptr<ferret::IMultiPointCot> mpcot,
         std::vector<__m128i>&& baseCot,
         __m128i delta) {
        mpcot->senderInit(delta, ferret::kExtendedSize, ferret::kWeight);
        return mpcot->senderExtend(std::move(baseCot));
      },
      std::move(sender),
      std::move(baseOTSend),
      delta);
  auto receiverTask = std::async(
      [](std::unique_ptr<ferret::IMultiPointCot> mpcot,
         std::vector<__m128i>&& baseCot) {
        mpcot->receiverInit(ferret::kExtendedSize, ferret::kWeight);
        return mpcot->receiverExtend(std::move(baseCot));
      },
      std::move(receiver),
      std::move(baseOTReceive));

  senderTask.get();
  receiverTask.get();

  BENCHMARK_SUSPEND {
    auto [senderSent, senderReceived] = agent0->getTrafficStatistics();
    counters["transmitted_bytes"] = senderSent + senderReceived;
  }
}

BENCHMARK_COUNTERS(RcotExtender, counters) {
  BenchmarkSuspender braces;
  auto [agent0, agent1] = util::getSocketAgents();

  ferret::RcotExtenderFactory factory(
      std::make_unique<ferret::TenLocalLinearMatrixMultiplierFactory>(),
      std::make_unique<ferret::RegularErrorMultiPointCotFactory>(
          std::make_unique<ferret::SinglePointCotFactory>()));

  auto extender0 = factory.create();
  auto extender1 = factory.create();

  extender0->setCommunicationAgent(std::move(agent0));
  extender1->setCommunicationAgent(std::move(agent1));

  auto baseOtSize = ferret::kBaseSize +
      std::log2(ferret::kExtendedSize / ferret::kWeight) * ferret::kWeight;
  auto [baseOTSend, baseOTReceive, delta] = getBaseOT(baseOtSize);

  braces.dismiss();

  auto senderTask = std::async(
      [](std::unique_ptr<ferret::IRcotExtender> rcotExtender,
         std::vector<__m128i>&& baseCot,
         __m128i delta) {
        rcotExtender->senderInit(
            delta, ferret::kExtendedSize, ferret::kBaseSize, ferret::kWeight);
        rcotExtender->senderExtendRcot(std::move(baseCot));
        return rcotExtender;
      },
      std::move(extender0),
      std::move(baseOTSend),
      delta);
  auto receiverTask = std::async(
      [](std::unique_ptr<ferret::IRcotExtender> rcotExtender,
         std::vector<__m128i>&& baseCot) {
        rcotExtender->receiverInit(
            ferret::kExtendedSize, ferret::kBaseSize, ferret::kWeight);
        rcotExtender->receiverExtendRcot(std::move(baseCot));
      },
      std::move(extender1),
      std::move(baseOTReceive));

  extender0 = senderTask.get();
  receiverTask.get();

  BENCHMARK_SUSPEND {
    auto [senderSent, senderReceived] = extender0->getTrafficStatistics();
    counters["transmitted_bytes"] = senderSent + senderReceived;
  }
}

int main(int argc, char* argv[]) {
  facebook::initFacebook(&argc, &argv);
  runBenchmarks();
  return 0;
}
