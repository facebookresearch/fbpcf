/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <filesystem>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "folly/Format.h"
#include "folly/Random.h"

#include "../../../pcf/io/FileManagerUtil.h"
#include "../../../pcf/mpc/EmpGame.h"
#include "../../common/GroupedLiftMetrics.h"
#include "../AggregatorApp.h"

namespace private_lift {
class AggregatorAppIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    port_ = 5000 + folly::Random::rand32() % 1000;
    baseDir_ = "./measurement/private_measurement/oss/lift/aggregator/test/";
    outputPathAlice_ =
        folly::sformat("{}_res_alice_{}", baseDir_, folly::Random::rand32());
    outputPathBob_ =
        folly::sformat("{}_res_bob_{}", baseDir_, folly::Random::rand32());
  }

  void TearDown() override {
    std::filesystem::remove(outputPathAlice_);
    std::filesystem::remove(outputPathBob_);
  }

  static void runGame(
      const pcf::Party& party,
      const pcf::Visibility& visibility,
      const std::string& serverIp,
      const uint16_t& port,
      const uint32_t& numShards,
      const std::string& inputPath,
      const std::string& outputPath) {
    AggregatorApp(
        party, visibility, serverIp, port, numShards, inputPath, outputPath)
        .run();
  }

 protected:
  uint16_t port_;
  std::string baseDir_;
  std::string outputPathAlice_;
  std::string outputPathBob_;
};

TEST_F(AggregatorAppIntegrationTest, TestVisibilityPublic) {
  auto futureAlice = std::async(
      runGame,
      pcf::Party::Alice,
      pcf::Visibility::Public,
      "",
      port_,
      3,
      baseDir_ + "aggregator_alice",
      outputPathAlice_);
  auto futureBob = std::async(
      runGame,
      pcf::Party::Bob,
      pcf::Visibility::Public,
      "127.0.0.1",
      port_,
      3,
      baseDir_ + "aggregator_bob",
      outputPathBob_);

  futureAlice.wait();
  futureBob.wait();

  auto resExpected = GroupedLiftMetrics::fromJson(
      pcf::io::read(baseDir_ + "aggregator_metrics"));
  auto resAlice = GroupedLiftMetrics::fromJson(pcf::io::read(outputPathAlice_));
  auto resBob = GroupedLiftMetrics::fromJson(pcf::io::read(outputPathBob_));
  EXPECT_EQ(resExpected, resAlice);
  EXPECT_EQ(resExpected, resBob);
}

TEST_F(AggregatorAppIntegrationTest, TestVisibilityBob) {
  auto futureAlice = std::async(
      runGame,
      pcf::Party::Alice,
      pcf::Visibility::Bob,
      "",
      port_,
      3,
      baseDir_ + "aggregator_alice",
      outputPathAlice_);
  auto futureBob = std::async(
      runGame,
      pcf::Party::Bob,
      pcf::Visibility::Bob,
      "127.0.0.1",
      port_,
      3,
      baseDir_ + "aggregator_bob",
      outputPathBob_);

  futureAlice.wait();
  futureBob.wait();

  auto resExpected = GroupedLiftMetrics::fromJson(
      pcf::io::read(baseDir_ + "aggregator_metrics"));
  auto resAlice = GroupedLiftMetrics::fromJson(pcf::io::read(outputPathAlice_));
  auto resBob = GroupedLiftMetrics::fromJson(pcf::io::read(outputPathBob_));
  GroupedLiftMetrics zeroMetrics{
      LiftMetrics{}, std::vector<LiftMetrics>{3, LiftMetrics{}}};
  EXPECT_EQ(zeroMetrics, resAlice);
  EXPECT_EQ(resExpected, resBob);
}
} // namespace private_lift
