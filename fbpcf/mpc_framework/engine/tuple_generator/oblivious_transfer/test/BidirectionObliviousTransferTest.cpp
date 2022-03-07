/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <future>
#include <memory>
#include <random>
#include <thread>
#include "fbpcf/mpc_framework/engine/communication/InMemoryPartyCommunicationAgentHost.h"
#include "fbpcf/mpc_framework/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/DummyBidirectionObliviousTransferFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/DummyRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/EmpShRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ExtenderBasedRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/RcotBasedBidirectionObliviousTransfer.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/RcotBasedBidirectionObliviousTransferFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/DummyMatrixMultiplierFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/DummyMultiPointCotFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/DummyRcotExtenderFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/DummySinglePointCotFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/RcotExtenderFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/RegularErrorMultiPointCotFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/SinglePointCotFactory.h"
#include "fbpcf/mpc_framework/engine/tuple_generator/oblivious_transfer/ferret/TenLocalLinearMatrixMultiplierFactory.h"
#include "fbpcf/mpc_framework/engine/util/AesPrgFactory.h"

namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer {

TEST(DummyBidirectionObliviousTransferTest, testBiDirectionOT) {
  auto factorys = communication::getInMemoryAgentFactory(2);

  insecure::DummyBidirectionObliviousTransferFactory<bool> factory0(
      *factorys[0]);
  insecure::DummyBidirectionObliviousTransferFactory<bool> factory1(
      *factorys[1]);

  communication::InMemoryPartyCommunicationAgentHost host;

  auto ot0 = factory0.create(1);
  auto ot1 = factory1.create(0);

  auto task = [](std::unique_ptr<IBidirectionObliviousTransfer<bool>> ot,
                 const std::vector<bool>& input0,
                 const std::vector<bool>& input1,
                 const std::vector<bool>& choice) {
    return ot->biDirectionOT(input0, input1, choice);
  };

  int size = 1024;
  std::vector<bool> inputA[2];
  inputA[0] = std::vector<bool>(size);
  inputA[1] = std::vector<bool>(size);
  std::vector<bool> choiceA(size);

  std::vector<bool> inputB[2];
  inputB[0] = std::vector<bool>(size);
  inputB[1] = std::vector<bool>(size);
  std::vector<bool> choiceB(size);

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint8_t> dist(0, 1);

  for (int i = 0; i < size; i++) {
    inputA[0][i] = dist(e);
    inputA[1][i] = dist(e);
    choiceA[i] = dist(e);

    inputB[0][i] = dist(e);
    inputB[1][i] = dist(e);
    choiceB[i] = dist(e);
  }

  auto f0 = std::async(task, std::move(ot0), inputA[0], inputA[1], choiceA);
  auto f1 = std::async(task, std::move(ot1), inputB[0], inputB[1], choiceB);

  auto resultA = f0.get();
  auto resultB = f1.get();

  for (int i = 0; i < size; i++) {
    EXPECT_EQ(resultA[i], inputB[choiceA[i]][i]);
    EXPECT_EQ(resultB[i], inputA[choiceB[i]][i]);
  }
}

void testRcotBasedBidirectionObliviousTransfer(
    std::unique_ptr<IRandomCorrelatedObliviousTransferFactory> factory0,
    std::unique_ptr<IRandomCorrelatedObliviousTransferFactory> factory1) {
  auto agentFactorys = communication::getInMemoryAgentFactory(2);

  auto task = [&agentFactorys](
                  const std::vector<bool>& input0,
                  const std::vector<bool>& input1,
                  const std::vector<bool>& choice,
                  std::unique_ptr<IRandomCorrelatedObliviousTransferFactory>
                      factory,
                  int myId) {
    auto otFactory =
        std::make_unique<RcotBasedBidirectionObliviousTransferFactory<bool>>(
            myId, *agentFactorys[myId], std::move(factory));
    auto ot = otFactory->create(1 - myId);
    return ot->biDirectionOT(input0, input1, choice);
  };

  int size = 11000000;
  std::vector<bool> inputA[2];
  inputA[0] = std::vector<bool>(size);
  inputA[1] = std::vector<bool>(size);
  std::vector<bool> choiceA(size);

  std::vector<bool> inputB[2];
  inputB[0] = std::vector<bool>(size);
  inputB[1] = std::vector<bool>(size);
  std::vector<bool> choiceB(size);

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint8_t> dist(0, 1);

  for (int i = 0; i < size; i++) {
    inputA[0][i] = dist(e);
    inputA[1][i] = dist(e);
    choiceA[i] = dist(e);

    inputB[0][i] = dist(e);
    inputB[1][i] = dist(e);
    choiceB[i] = dist(e);
  }
  // needs a better bootstrapper
  auto f1 =
      std::async(task, inputA[0], inputA[1], choiceA, std::move(factory0), 1);

  auto f2 =
      std::async(task, inputB[0], inputB[1], choiceB, std::move(factory1), 0);

  auto resultA = f1.get();
  auto resultB = f2.get();

  for (int i = 0; i < size; i++) {
    EXPECT_EQ(resultA[i], inputB[choiceA[i]][i]);
    EXPECT_EQ(resultB[i], inputA[choiceB[i]][i]);
  }
}

TEST(
    RcotBasedBidirectionObliviousTransferTest,
    testBiDirectionOTWithDummyRcot) {
  testRcotBasedBidirectionObliviousTransfer(
      std::make_unique<
          insecure::DummyRandomCorrelatedObliviousTransferFactory>(),
      std::make_unique<
          insecure::DummyRandomCorrelatedObliviousTransferFactory>());
}

TEST(
    RcotBasedBidirectionObliviousTransferTest,
    testBiDirectionOTWithExtenderBasedRcotPoweredByDummyExtender) {
  testRcotBasedBidirectionObliviousTransfer(
      std::make_unique<ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<
              insecure::DummyRandomCorrelatedObliviousTransferFactory>(),
          std::make_unique<ferret::insecure::DummyRcotExtenderFactory>(),
          1024,
          128,
          8),
      std::make_unique<ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<
              insecure::DummyRandomCorrelatedObliviousTransferFactory>(),
          std::make_unique<ferret::insecure::DummyRcotExtenderFactory>(),
          1024,
          128,
          8));
}

TEST(
    RcotBasedBidirectionObliviousTransferTest,
    testBiDirectionOTWithExtenderBasedRcotPoweredByFerretExtenderPoweredByDummyMpcotAndDummyMatrixMultipler) {
  testRcotBasedBidirectionObliviousTransfer(
      std::make_unique<ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<
              insecure::DummyRandomCorrelatedObliviousTransferFactory>(),
          std::make_unique<ferret::RcotExtenderFactory>(
              std::make_unique<
                  ferret::insecure::DummyMatrixMultiplierFactory>(),
              std::make_unique<ferret::insecure::DummyMultiPointCotFactory>(
                  std::make_unique<util::AesPrgFactory>(1024))),
          1024,
          128,
          8),
      std::make_unique<ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<
              insecure::DummyRandomCorrelatedObliviousTransferFactory>(),
          std::make_unique<ferret::RcotExtenderFactory>(
              std::make_unique<
                  ferret::insecure::DummyMatrixMultiplierFactory>(),
              std::make_unique<ferret::insecure::DummyMultiPointCotFactory>(
                  std::make_unique<util::AesPrgFactory>(1024))),
          1024,
          128,
          8));
}

TEST(
    RcotBasedBidirectionObliviousTransferTest,
    testBiDirectionOTWithExtenderBasedRcotPoweredByFerretExtenderPoweredByDummyMpcotAnd10LocalLinearMatrixMultipler) {
  testRcotBasedBidirectionObliviousTransfer(
      std::make_unique<ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<
              insecure::DummyRandomCorrelatedObliviousTransferFactory>(),
          std::make_unique<ferret::RcotExtenderFactory>(
              std::make_unique<ferret::TenLocalLinearMatrixMultiplierFactory>(),
              std::make_unique<ferret::insecure::DummyMultiPointCotFactory>(
                  std::make_unique<util::AesPrgFactory>(1024))),
          1024,
          128,
          8),
      std::make_unique<ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<
              insecure::DummyRandomCorrelatedObliviousTransferFactory>(),
          std::make_unique<ferret::RcotExtenderFactory>(
              std::make_unique<ferret::TenLocalLinearMatrixMultiplierFactory>(),
              std::make_unique<ferret::insecure::DummyMultiPointCotFactory>(
                  std::make_unique<util::AesPrgFactory>(1024))),
          1024,
          128,
          8));
}

TEST(
    RcotBasedBidirectionObliviousTransferTest,
    testBiDirectionOTWithExtenderBasedRcotPoweredByFerretExtenderPoweredByMpcotWithDummySpcotAnd10LocalLinearMatrixMultipler) {
  testRcotBasedBidirectionObliviousTransfer(
      std::make_unique<ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<
              insecure::DummyRandomCorrelatedObliviousTransferFactory>(),
          std::make_unique<ferret::RcotExtenderFactory>(
              std::make_unique<ferret::TenLocalLinearMatrixMultiplierFactory>(),
              std::make_unique<ferret::RegularErrorMultiPointCotFactory>(
                  std::make_unique<
                      ferret::insecure::DummySinglePointCotFactory>(
                      std::make_unique<util::AesPrgFactory>(1024)))),
          1024,
          128,
          8),
      std::make_unique<ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<
              insecure::DummyRandomCorrelatedObliviousTransferFactory>(),
          std::make_unique<ferret::RcotExtenderFactory>(
              std::make_unique<ferret::TenLocalLinearMatrixMultiplierFactory>(),
              std::make_unique<ferret::RegularErrorMultiPointCotFactory>(
                  std::make_unique<
                      ferret::insecure::DummySinglePointCotFactory>(
                      std::make_unique<util::AesPrgFactory>(1024)))),
          1024,
          128,
          8));
}

TEST(
    RcotBasedBidirectionObliviousTransferTest,
    testBiDirectionOTWithExtenderBasedRcotPoweredByFerretExtenderPoweredByMpcotWithRealSpcotAnd10LocalLinearMatrixMultipler) {
  testRcotBasedBidirectionObliviousTransfer(
      std::make_unique<ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<
              insecure::DummyRandomCorrelatedObliviousTransferFactory>(),
          std::make_unique<ferret::RcotExtenderFactory>(
              std::make_unique<ferret::TenLocalLinearMatrixMultiplierFactory>(),
              std::make_unique<ferret::RegularErrorMultiPointCotFactory>(
                  std::make_unique<ferret::SinglePointCotFactory>())),
          ferret::kExtendedSize,
          ferret::kBaseSize,
          ferret::kWeight),
      std::make_unique<ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<
              insecure::DummyRandomCorrelatedObliviousTransferFactory>(),
          std::make_unique<ferret::RcotExtenderFactory>(
              std::make_unique<ferret::TenLocalLinearMatrixMultiplierFactory>(),
              std::make_unique<ferret::RegularErrorMultiPointCotFactory>(
                  std::make_unique<ferret::SinglePointCotFactory>())),
          ferret::kExtendedSize,
          ferret::kBaseSize,
          ferret::kWeight));
}

TEST(
    RcotBasedBidirectionObliviousTransferTest,
    testBiDirectionOTWithBootstrappedExtenderBasedRcotPoweredByFerretExtenderPoweredByMpcotWithRealSpcotAnd10LocalLinearMatrixMultipler) {
  testRcotBasedBidirectionObliviousTransfer(
      std::make_unique<ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<EmpShRandomCorrelatedObliviousTransferFactory>(
              std::make_unique<util::AesPrgFactory>(1024)),
          std::make_unique<ferret::RcotExtenderFactory>(
              std::make_unique<ferret::TenLocalLinearMatrixMultiplierFactory>(),
              std::make_unique<ferret::RegularErrorMultiPointCotFactory>(
                  std::make_unique<ferret::SinglePointCotFactory>())),
          ferret::kExtendedSize,
          ferret::kBaseSize,
          ferret::kWeight),
      std::make_unique<ExtenderBasedRandomCorrelatedObliviousTransferFactory>(
          std::make_unique<EmpShRandomCorrelatedObliviousTransferFactory>(
              std::make_unique<util::AesPrgFactory>(1024)),
          std::make_unique<ferret::RcotExtenderFactory>(
              std::make_unique<ferret::TenLocalLinearMatrixMultiplierFactory>(),
              std::make_unique<ferret::RegularErrorMultiPointCotFactory>(
                  std::make_unique<ferret::SinglePointCotFactory>())),
          ferret::kExtendedSize,
          ferret::kBaseSize,
          ferret::kWeight));
}
} // namespace fbpcf::mpc_framework::engine::tuple_generator::oblivious_transfer
