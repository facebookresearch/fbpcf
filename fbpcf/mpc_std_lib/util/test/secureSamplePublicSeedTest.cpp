/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/mpc_std_lib/util/secureSamplePublicSeed.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "fbpcf/engine/communication/test/AgentFactoryCreationHelper.h"
#include "fbpcf/engine/communication/test/SocketInTestHelper.h"
#include "fbpcf/engine/communication/test/TlsCommunicationUtils.h"
#include "fbpcf/test/TestHelper.h"

namespace fbpcf::mpc_std_lib::util {

__m128i testSecureSamplePublicSeed(
    bool amISendingFirst,
    std::reference_wrapper<
        engine::communication::IPartyCommunicationAgentFactory> factory) {
  auto agent = factory.get().create(amISendingFirst ? 0 : 1, "test");
  return secureSamplePublicSeed(amISendingFirst, *agent);
}

TEST(SecureSamplePublicSeedTest, TestSampleSucceed) {
  fbpcf::engine::communication::SocketPartyCommunicationAgent::TlsInfo tlsInfo;

  tlsInfo.certPath = "";
  tlsInfo.keyPath = "";
  tlsInfo.passphrasePath = "";
  tlsInfo.useTls = false;

  std::vector<std::unique_ptr<
      engine::communication::SocketPartyCommunicationAgentFactoryForTests>>
      factories(2);
  engine::communication::getSocketFactoriesForMultipleParties(
      2, tlsInfo, factories);
  auto t0 = std::async(
      testSecureSamplePublicSeed,
      false,
      std::reference_wrapper(*factories.at(0)));
  auto v1 = testSecureSamplePublicSeed(true, *factories.at(1));
  auto v0 = t0.get();
  EXPECT_TRUE(compareM128i(v0, v1));
}

} // namespace fbpcf::mpc_std_lib::util
