/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <cstdint>
#include <functional>
#include <future>
#include <memory>
#include <random>
#include <thread>

#include "fbpcf/engine/communication/InMemoryPartyCommunicationAgentFactory.h"
#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/engine/tuple_generator/DummyProductShareGenerator.h"
#include "fbpcf/engine/tuple_generator/DummyProductShareGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/IProductShareGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/ProductShareGenerator.h"
#include "fbpcf/engine/tuple_generator/ProductShareGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/DummyBidirectionObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/ExtenderBasedRandomCorrelatedObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/RcotBasedBidirectionObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/RcotHelper.h"
#include "fbpcf/engine/tuple_generator/test/TupleGeneratorTestHelper.h"
#include "fbpcf/engine/util/AesPrgFactory.h"

namespace fbpcf::engine::tuple_generator {

void testGenerator(
    std::unique_ptr<IProductShareGeneratorFactory> factory0,
    std::unique_ptr<IProductShareGeneratorFactory> factory1) {
  int size = 1024;
  std::vector<bool> left0(size);
  std::vector<bool> right0(size);
  std::vector<bool> left1(size);
  std::vector<bool> right1(size);

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint8_t> dist(0, 1);

  for (int i = 0; i < size; i++) {
    left0[i] = dist(e);
    right0[i] = dist(e);
    left1[i] = dist(e);
    right1[i] = dist(e);
  }
  auto task = [](std::unique_ptr<IProductShareGeneratorFactory> factory,
                 int partnerId,
                 const std::vector<bool>& left,
                 const std::vector<bool>& right) {
    auto generator = factory->create(partnerId);
    return generator->generateBooleanProductShares(left, right);
  };

  auto f0 = std::async(task, std::move(factory0), 1, left0, right0);

  auto f1 = std::async(task, std::move(factory1), 0, left1, right1);

  auto result0 = f0.get();
  auto result1 = f1.get();

  for (int i = 0; i < size; i++) {
    EXPECT_EQ(
        result0[i] ^ result1[i],
        (left0[i] && right1[i]) ^ (left1[i] && right0[i]));
  }
}

void testIntegerGenerator(
    std::unique_ptr<IProductShareGeneratorFactory> factory0,
    std::unique_ptr<IProductShareGeneratorFactory> factory1) {
  int size = 1024;
  std::vector<uint64_t> left0(size);
  std::vector<uint64_t> right0(size);
  std::vector<uint64_t> left1(size);
  std::vector<uint64_t> right1(size);

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint64_t> dist;

  for (int i = 0; i < size; i++) {
    left0[i] = dist(e);
    right0[i] = dist(e);
    left1[i] = dist(e);
    right1[i] = dist(e);
  }
  auto task = [](std::unique_ptr<IProductShareGeneratorFactory> factory,
                 int partnerId,
                 const std::vector<uint64_t>& left,
                 const std::vector<uint64_t>& right) {
    auto generator = factory->create(partnerId);
    return generator->generateIntegerProductShares(left, right);
  };

  auto f0 = std::async(task, std::move(factory0), 1, left0, right0);

  auto f1 = std::async(task, std::move(factory1), 0, left1, right1);

  auto result0 = f0.get();
  auto result1 = f1.get();

  for (int i = 0; i < size; i++) {
    EXPECT_EQ(
        result0[i] + result1[i],
        (left0[i] * right1[i]) + (left1[i] * right0[i]));
  }
}

TEST(ProductShareGenerator, testDummyGenerator) {
  auto factorys = communication::getInMemoryAgentFactory(2);
  testGenerator(
      std::make_unique<insecure::DummyProductShareGeneratorFactory>(
          *factorys[0]),
      std::make_unique<insecure::DummyProductShareGeneratorFactory>(
          *factorys[1]));
}

TEST(ProductShareGenerator, testDummyIntegerGenerator) {
  auto factorys = communication::getInMemoryAgentFactory(2);
  testIntegerGenerator(
      std::make_unique<insecure::DummyProductShareGeneratorFactory>(
          *factorys[0]),
      std::make_unique<insecure::DummyProductShareGeneratorFactory>(
          *factorys[1]));
}

TEST(ProductShareGenerator, testRealGeneratorWithDummyOT) {
  auto factorys = communication::getInMemoryAgentFactory(2);
  testGenerator(
      std::make_unique<ProductShareGeneratorFactory>(
          std::make_unique<util::AesPrgFactory>(),
          std::make_unique<oblivious_transfer::insecure::
                               DummyBidirectionObliviousTransferFactory>(
              *factorys[0])),
      std::make_unique<ProductShareGeneratorFactory>(
          std::make_unique<util::AesPrgFactory>(),
          std::make_unique<oblivious_transfer::insecure::
                               DummyBidirectionObliviousTransferFactory>(
              *factorys[1])));
}

TEST(ProductShareGenerator, testRealIntegerGeneratorWithDummyOT) {
  auto factorys = communication::getInMemoryAgentFactory(2);
  testIntegerGenerator(
      std::make_unique<ProductShareGeneratorFactory>(
          std::make_unique<util::AesPrgFactory>(),
          std::make_unique<oblivious_transfer::insecure::
                               DummyBidirectionObliviousTransferFactory>(
              *factorys[0])),
      std::make_unique<ProductShareGeneratorFactory>(
          std::make_unique<util::AesPrgFactory>(),
          std::make_unique<oblivious_transfer::insecure::
                               DummyBidirectionObliviousTransferFactory>(
              *factorys[1])));
}

TEST(ProductShareGenerator, testRealGeneratorWithRealOT) {
  auto agentFactories = communication::getInMemoryAgentFactory(2);

  testGenerator(
      std::make_unique<ProductShareGeneratorFactory>(
          std::make_unique<util::AesPrgFactory>(),
          std::make_unique<
              oblivious_transfer::RcotBasedBidirectionObliviousTransferFactory>(
              0,
              *agentFactories.at(0),
              oblivious_transfer::createFerretRcotFactory(
                  kTestExtendedSize, kTestBaseSize, kTestWeight))),
      std::make_unique<ProductShareGeneratorFactory>(
          std::make_unique<util::AesPrgFactory>(),
          std::make_unique<
              oblivious_transfer::RcotBasedBidirectionObliviousTransferFactory>(
              1,
              *agentFactories.at(1),
              oblivious_transfer::createFerretRcotFactory(
                  kTestExtendedSize, kTestBaseSize, kTestWeight))));
}

TEST(ProductShareGenerator, testRealIntegerGeneratorWithRealOT) {
  auto agentFactories = communication::getInMemoryAgentFactory(2);
  testIntegerGenerator(
      std::make_unique<ProductShareGeneratorFactory>(
          std::make_unique<util::AesPrgFactory>(),
          std::make_unique<
              oblivious_transfer::RcotBasedBidirectionObliviousTransferFactory>(
              0,
              *agentFactories.at(0),
              oblivious_transfer::createFerretRcotFactory(
                  kTestExtendedSize, kTestBaseSize, kTestWeight))),
      std::make_unique<ProductShareGeneratorFactory>(
          std::make_unique<util::AesPrgFactory>(),
          std::make_unique<
              oblivious_transfer::RcotBasedBidirectionObliviousTransferFactory>(
              1,
              *agentFactories.at(1),
              oblivious_transfer::createFerretRcotFactory(
                  kTestExtendedSize, kTestBaseSize, kTestWeight))));
}

} // namespace fbpcf::engine::tuple_generator
