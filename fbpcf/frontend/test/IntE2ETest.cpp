/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <fbpcf/test/TestHelper.h>
#include <gtest/gtest.h>
#include "fbpcf/engine/communication/SocketPartyCommunicationAgentFactory.h"
#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/frontend/Int.h"

namespace fbpcf::frontend {

template <int PARTY, int schedulerId>
static int runGame() {
  Integer<Secret<Signed<63>>, schedulerId> int1(PARTY == 0 ? -45 : 10, 0);
  Integer<Secret<Signed<63>>, schedulerId> int2(PARTY == 1 ? 15 : -90, 1);

  auto sum = int1 + int2;

  return sum.openToParty(PARTY + 1 % 2).getValue();
}

class IntE2ETest : public ::testing::Test {
 protected:
  void SetUp() override {
    auto factories = fbpcf::engine::communication::getSocketAgentFactory(2);
    fbpcf::setupRealBackend<5, 10>(*factories[0], *factories[1]);
  }

  void TearDown() override {}
};

TEST_F(IntE2ETest, TestCorrectness) {
  auto futureAlice = std::async(runGame<0, 5>);
  auto futureBob = std::async(runGame<1, 10>);

  int ans1 = futureAlice.get();
  int ans2 = futureBob.get();

  EXPECT_EQ(ans1, 30);
  EXPECT_EQ(ans2, 30);
}

} // namespace fbpcf::frontend
