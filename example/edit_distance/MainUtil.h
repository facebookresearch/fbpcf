/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "./EditDistanceApp.h" //@manual
#include "fbpcf/engine/communication/SocketPartyCommunicationAgentFactory.h"

namespace fbpcf::edit_distance {

template <int MyRole>
void startEditDistanceGame(
    std::string& serverIp,
    int port,
    std::string& dataFilePath,
    std::string& paramsFilePath,
    std::string& outFilePath) {
  std::map<
      int,
      fbpcf::engine::communication::SocketPartyCommunicationAgentFactory::
          PartyInfo>
      partyInfos({{0, {serverIp, port}}, {1, {serverIp, port}}});

  fbpcf::engine::communication::SocketPartyCommunicationAgent::TlsInfo tlsInfo;
  tlsInfo.certPath = "";
  tlsInfo.keyPath = "";
  tlsInfo.passphrasePath = "";
  tlsInfo.useTls = false;

  auto communicationAgentFactory = std::make_unique<
      fbpcf::engine::communication::SocketPartyCommunicationAgentFactory>(
      MyRole, partyInfos, tlsInfo, "Edit Distance Traffic for main thread");

  XLOG(INFO, "Creating Edit Distance App");

  auto app = std::make_unique<fbpcf::edit_distance::EditDistanceApp<MyRole>>(
      MyRole,
      std::move(communicationAgentFactory),
      dataFilePath,
      paramsFilePath,
      outFilePath);

  app->run();
}

} // namespace fbpcf::edit_distance
