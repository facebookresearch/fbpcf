/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <map>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <stdexcept>
#include <string>

#include <fbpcf/util/MetricCollector.h>
#include "fbpcf/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/engine/communication/SocketPartyCommunicationAgent.h"
#include "fbpcf/engine/communication/SocketPartyCommunicationAgentFactory.h"

namespace fbpcf::engine::communication {

/**
 * An communication factory API for tests only
 * It allows you to separate the socket binding
 * from the actual listen+connect step.
 */
class SocketPartyCommunicationAgentFactoryForTests final
    : public SocketPartyCommunicationAgentFactory {
 public:
  SocketPartyCommunicationAgentFactoryForTests(
      int myId,
      std::map<int, PartyInfo> partyInfos,
      SocketPartyCommunicationAgent::TlsInfo tlsInfo,
      std::shared_ptr<fbpcf::util::MetricCollector> metricCollector)
      : SocketPartyCommunicationAgentFactory(
            myId,
            partyInfos,
            tlsInfo,
            metricCollector,
            true) {}

  /**
   * Returns the ports that were bound to.
   * This is important when the initial connection is
   * different than the port that was provided. This
   * happens when the provided port was not free.
   */
  std::map<int, int> getBoundPorts() {
    std::map<int, int> ports;
    for (const auto [partyId, socket] : sockets_) {
      ports.insert({partyId, getPortFromSocket(socket)});
    }

    return ports;
  }

  void completeNetworkingSetup(std::map<int, PartyInfo> partyInfos) {
    setupInitialConnection(partyInfos);
  }
};

} // namespace fbpcf::engine::communication
