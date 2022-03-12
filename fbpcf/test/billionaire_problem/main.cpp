/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gflags/gflags.h>
#include "fbpcf/scheduler/SchedulerHelper.h"
#include "folly/init/Init.h"
#include "folly/logging/xlog.h"

#include "fbpcf/engine/communication/SocketPartyCommunicationAgentFactory.h"
#include "fbpcf/test/billionaire_problem/BillionaireProblemGame.h"

DEFINE_int32(party, 0, "my party ID");
DEFINE_string(server_ip, "127.0.0.1", "server's ip address");
DEFINE_int32(port, 5000, "server port number");

int main(int argc, char* argv[]) {
  folly::init(&argc, &argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  XLOGF(INFO, "party: {}", FLAGS_party);
  XLOGF(INFO, "server IP: {}", FLAGS_server_ip);
  XLOGF(INFO, "port: {}", FLAGS_port);

  std::map<
      int,
      fbpcf::engine::communication::SocketPartyCommunicationAgentFactory::
          PartyInfo>
      partyInfos(
          {{0, {FLAGS_server_ip, FLAGS_port}},
           {1, {FLAGS_server_ip, FLAGS_port}}});
  auto factory = std::make_unique<
      fbpcf::engine::communication::SocketPartyCommunicationAgentFactory>(
      FLAGS_party, partyInfos);

  auto game = std::make_unique<
      fbpcf::billionaire_problem::BillionaireProblemGame<0, true>>(
      fbpcf::scheduler::createLazySchedulerWithRealEngine(
          FLAGS_party, *factory));

  const int size = 32;

  typename fbpcf::billionaire_problem::BillionaireProblemGame<0, true>::
      AssetsLists myAssets = {
          .cash = std::vector<uint32_t>(size),
          .stock = std::vector<uint32_t>(size),
          .property = std::vector<uint32_t>(size),
      };

  typename fbpcf::billionaire_problem::BillionaireProblemGame<0, true>::
      AssetsLists dummyAssets = {
          .cash = std::vector<uint32_t>(size),
          .stock = std::vector<uint32_t>(size),
          .property = std::vector<uint32_t>(size),
      };

  std::random_device rd;
  std::mt19937_64 e(rd());
  std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);

  for (auto& item : myAssets.cash) {
    item = dist(e);
  }
  for (auto& item : myAssets.stock) {
    item = dist(e);
  }
  for (auto& item : myAssets.property) {
    item = dist(e);
  }

  try {
    auto mpcResult = FLAGS_party == 0
        ? game->billionaireProblem(myAssets, dummyAssets)
        : game->billionaireProblem(dummyAssets, myAssets);
  } catch (...) {
    XLOG(FATAL, "Failed to execute the game!");
  }
  XLOG(INFO, "Game executed successfully!");
}
