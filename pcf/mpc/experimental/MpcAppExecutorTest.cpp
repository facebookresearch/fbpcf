/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/

#include <future>
#include <vector>

#include <gtest/gtest.h>

#include "folly/Random.h"

#include "../test/MillionaireApp.h"
#include "./MpcAppExecutor.h"

namespace pcf {
class MpcAppExecutorTest : public ::testing::Test {
 protected:
  void SetUp() override {
    port_ = 5000 + folly::Random::rand32() % 1000;
  }

  static void
  executeApps(Party party, const std::string& serverIp, uint16_t port) {
    constexpr int kNumOfTask = 10;
    constexpr int kConcurrency = 4;
    std::vector<std::unique_ptr<MillionaireApp>> apps;

    for (auto i = 0; i < kNumOfTask; i++) {
      apps.push_back(std::make_unique<MillionaireApp>(
          party, serverIp, port + i % kConcurrency));
    }

    MpcAppExecutor<MillionaireApp> executor{kConcurrency};
    executor.execute(apps);
  }

 protected:
  uint16_t port_;
}; // namespace pcf

TEST_F(MpcAppExecutorTest, TestConcurrency) {
  auto futureAlice = std::async(executeApps, Party::Alice, "", port_);
  auto futureBob = std::async(executeApps, Party::Bob, "127.0.0.1", port_);

  futureAlice.wait();
  futureBob.wait();
}
} // namespace pcf
