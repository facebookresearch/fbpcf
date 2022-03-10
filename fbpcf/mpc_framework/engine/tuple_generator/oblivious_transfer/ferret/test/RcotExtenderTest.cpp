/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <emmintrin.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <immintrin.h>
#include <future>
#include <memory>

#include "fbpcf/mpc_framework/engine/communication/InMemoryPartyCommunicationAgentHost.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/DummyMatrixMultiplierFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/DummyMultiPointCotFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/DummyRcotExtender.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/DummyRcotExtenderFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/DummySinglePointCotFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/RcotExtender.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/RcotExtenderFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/RegularErrorMultiPointCotFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/SinglePointCotFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/TenLocalLinearMatrixMultiplierFactory.h"
#include "fbpcf/mpc_framework/engine/util/AesPrgFactory.h"
#include "fbpcf/mpc_framework/test/TestHelper.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer::ferret {

void testRcotExtender(std::unique_ptr<IRcotExtenderFactory> factory) {
  communication::InMemoryPartyCommunicationAgentHost host;

  auto agent0 = host.getAgent(0);
  auto agent1 = host.getAgent(1);

  auto extender0 = factory->create();
  auto extender1 = factory->create();

  __m128i delta = _mm_set_epi32(0xFFFFFFFF, 0, 0, 1);

  int64_t baseSize = 128;
  int64_t extendedSize = 16384;
  int64_t weight = 8;

  auto senderTask =
      [delta, baseSize, extendedSize, weight](
          std::unique_ptr<IRcotExtender> otExtender,
          std::unique_ptr<communication::IPartyCommunicationAgent> agent) {
        auto baseOTSize =
            otExtender->senderInit(delta, extendedSize, baseSize, weight);
        std::vector<__m128i> baseOT(baseOTSize);
        for (int i = 0; i < baseOTSize; i++) {
          baseOT[i] = _mm_set_epi32(i, i, i, i << 1);
        }
        otExtender->setCommunicationAgent(std::move(agent));
        return otExtender->senderExtendRcot(std::move(baseOT));
      };

  auto receiverTask =
      [delta, baseSize, extendedSize, weight](
          std::unique_ptr<IRcotExtender> otExtender,
          std::unique_ptr<communication::IPartyCommunicationAgent> agent) {
        auto baseOTSize =
            otExtender->receiverInit(extendedSize, baseSize, weight);
        std::vector<__m128i> baseOT(baseOTSize);
        for (int i = 0; i < baseOTSize; i++) {
          baseOT[i] = _mm_set_epi32(i, i, i, i << 1);
          if (i % 2 == 0) {
            baseOT[i] = _mm_xor_si128(baseOT[i], delta);
          }
        }
        otExtender->setCommunicationAgent(std::move(agent));
        return otExtender->receiverExtendRcot(std::move(baseOT));
      };

  auto f0 = std::async(senderTask, std::move(extender0), std::move(agent0));
  auto f1 = std::async(receiverTask, std::move(extender1), std::move(agent1));

  auto resultSend = f0.get();
  auto resultReceive = f1.get();

  EXPECT_EQ(resultSend.size(), extendedSize);
  EXPECT_EQ(resultSend.size(), resultReceive.size());

  for (int i = 0; i < resultSend.size(); i++) {
    // Note that LSB(resultReceive[i]) is the choice bit;
    // LSB(resultSend[i]) = 0 and
    // LSB(delta) = 1
    EXPECT_EQ(_mm_extract_epi8(resultSend[i], 0) & 1, 0);
    EXPECT_TRUE(
        compareM128i(resultSend[i], resultReceive[i]) ||
        compareM128i(_mm_xor_si128(resultSend[i], delta), resultReceive[i]));
  }
}

TEST(RcotExtenderTest, testDummyRcot) {
  testRcotExtender(std::make_unique<insecure::DummyRcotExtenderFactory>());
}

TEST(RcotExtenderTest, testFerretRcotWithDummyObjects) {
  testRcotExtender(std::make_unique<RcotExtenderFactory>(
      std::make_unique<insecure::DummyMatrixMultiplierFactory>(),
      std::make_unique<insecure::DummyMultiPointCotFactory>(
          std::make_unique<util::AesPrgFactory>(1024))));
}

TEST(RcotExtenderTest, testFerretRcotWith10LocalLinearRandomMatrix) {
  testRcotExtender(std::make_unique<RcotExtenderFactory>(
      std::make_unique<TenLocalLinearMatrixMultiplierFactory>(),
      std::make_unique<insecure::DummyMultiPointCotFactory>(
          std::make_unique<util::AesPrgFactory>(1024))));
}

TEST(RCotExtenderTest, testWithRegularErrorMPCOT) {
  testRcotExtender(std::make_unique<RcotExtenderFactory>(
      std::make_unique<TenLocalLinearMatrixMultiplierFactory>(),
      std::make_unique<RegularErrorMultiPointCotFactory>(
          std::make_unique<insecure::DummySinglePointCotFactory>(
              std::make_unique<util::AesPrgFactory>(1024)))));
}

TEST(RcotExtenderTest, testWithRegularErrorMPCOTandRealSPCOT) {
  testRcotExtender(std::make_unique<RcotExtenderFactory>(
      std::make_unique<TenLocalLinearMatrixMultiplierFactory>(),
      std::make_unique<RegularErrorMultiPointCotFactory>(
          std::make_unique<SinglePointCotFactory>())));
}

} // namespace
  // fbpcf::engine::tuple_generator::oblivious_transfer::ferret
