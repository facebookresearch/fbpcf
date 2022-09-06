/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <emmintrin.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <future>
#include <memory>
#include <random>
#include <thread>
#include "fbpcf/engine/communication/InMemoryPartyCommunicationAgentHost.h"
#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/test/TestHelper.h"

#include "fbpcf/engine/tuple_generator/oblivious_transfer/DummyRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/EmpShRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ExtenderBasedRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/RcotExtenderFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/RegularErrorMultiPointCotFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/SinglePointCotFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ferret/TenLocalLinearMatrixMultiplierFactory.h"

#include <fbpcf/mpc_std_lib/walr_multiplication/util/COTWithRandomMessage.h>
#include <fbpcf/mpc_std_lib/walr_multiplication/util/COTWithRandomMessageFactory.h>

namespace fbpcf::mpc_std_lib::walr::util {

void testCOTWithRandomMessage(
    std::unique_ptr<engine::tuple_generator::oblivious_transfer::
                        IRandomCorrelatedObliviousTransferFactory> rcotFactory0,
    std::unique_ptr<engine::tuple_generator::oblivious_transfer::
                        IRandomCorrelatedObliviousTransferFactory>
        rcotFactory1) {
  auto agentFactories = engine::communication::getInMemoryAgentFactory(2);

  auto senderTask =
      [&agentFactories](
          std::unique_ptr<engine::tuple_generator::oblivious_transfer::
                              IRandomCorrelatedObliviousTransferFactory>
              rcotFactory,
          __m128i delta,
          size_t size,
          int myId,
          int partnerId) {
        auto otFactory = std::make_unique<COTWithRandomMessageFactory>(
            std::move(rcotFactory));

        auto otAgent =
            agentFactories[myId]->create(partnerId, "COTwRM_sender_traffic");
        auto rcotAgent = agentFactories[myId]->create(
            partnerId, "RCOT_of_COTwRM_sender_traffic");
        auto ot =
            otFactory->create(delta, std::move(otAgent), std::move(rcotAgent));
        return ot->send(size);
      };

  auto receiverTask =
      [&agentFactories](
          std::unique_ptr<engine::tuple_generator::oblivious_transfer::
                              IRandomCorrelatedObliviousTransferFactory>
              rcotFactory,
          const std::vector<bool>& choice,
          int myId,
          int partnerId) {
        auto otFactory = std::make_unique<COTWithRandomMessageFactory>(
            std::move(rcotFactory));

        auto otAgent =
            agentFactories[myId]->create(partnerId, "COTwRM_receiver_traffic");
        auto rcotAgent = agentFactories[myId]->create(
            partnerId, "RCOT_of_COTwRM_receiver_traffic");
        auto ot = otFactory->create(std::move(otAgent), std::move(rcotAgent));
        return ot->receive(choice);
      };

  __m128i delta = engine::util::getRandomM128iFromSystemNoise();
  engine::util::setLsbTo1(delta);
  int size = 200;
  std::vector<bool> choice(size);

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::bernoulli_distribution dist(0.5);
  std::generate(
      choice.begin(), choice.end(), [&dist, &e]() { return dist(e); });

  auto sender =
      std::async(senderTask, std::move(rcotFactory0), delta, size, 0, 1);
  auto receiver =
      std::async(receiverTask, std::move(rcotFactory1), choice, 1, 0);

  auto [m0, m1] = sender.get();
  auto m = receiver.get();

  for (int i = 0; i < size; i++) {
    EXPECT_TRUE(compareM128i(m0[i], _mm_xor_si128(m1[i], delta)));
    if (choice[i]) {
      EXPECT_TRUE(compareM128i(m1[i], m[i]));
    } else {
      EXPECT_TRUE(compareM128i(m0[i], m[i]));
    }
  }
}

TEST(COTWithRandomMessageTest, testDummyRCOTBasedCOTwR) {
  testCOTWithRandomMessage(
      std::make_unique<engine::tuple_generator::oblivious_transfer::insecure::
                           DummyRandomCorrelatedObliviousTransferFactory>(),
      std::make_unique<engine::tuple_generator::oblivious_transfer::insecure::
                           DummyRandomCorrelatedObliviousTransferFactory>());
}

TEST(COTWithRandomMessageTest, testFERRETBasedCOTwR) {
  testCOTWithRandomMessage(
      std::make_unique<
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
          engine::tuple_generator::oblivious_transfer::ferret::kWeight),
      std::make_unique<
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
          engine::tuple_generator::oblivious_transfer::ferret::kWeight));
}

} // namespace fbpcf::mpc_std_lib::walr::util
