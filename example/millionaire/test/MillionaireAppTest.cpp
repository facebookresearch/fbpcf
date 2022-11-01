/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <future>

#include <gtest/gtest.h>

#include "folly/Random.h"

#include "../MillionaireApp.h" // @manual

namespace fbpcf {

class MillionaireAppTest : public ::testing::Test {
 protected:
  void SetUp() override {
    port_ = 5000 + folly::Random::rand32() % 1000;
  }

  static void runGame(Party party, const std::string& serverIp, uint16_t port) {
    MillionaireApp(party, serverIp, port).run();
  }

 protected:
  uint16_t port_;
};

// The purpose of this test case is to ensure there is no exception to run
// MillionaireApp. Correctness test is coverred in MillionaireGameTest.
TEST_F(MillionaireAppTest, TestNoException) {
  auto futureAlice = std::async(runGame, Party::Alice, "", port_);
  auto futureBob = std::async(runGame, Party::Bob, "127.0.0.1", port_);

  futureAlice.wait();
  futureBob.wait();
}
} // namespace fbpcf
