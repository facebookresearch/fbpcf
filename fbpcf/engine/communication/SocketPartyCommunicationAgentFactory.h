/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <map>

#include "fbpcf/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/engine/communication/SocketPartyCommunicationAgent.h"

namespace fbpcf::engine::communication {

/**
 * An communication factory API
 */
class SocketPartyCommunicationAgentFactory final
    : public IPartyCommunicationAgentFactory {
 public:
  struct PartyInfo {
    std::string address;
    int portNo;
  };

  /** it's OK if a party with a smaller id doesn't know a party with larger id's
  * ip address, since the party with smaller id will always be the server.
  *@param partyInfos This is a map that contains connection information for all
other parties, 0...n where myId is some m, 0 <= m <= n. The map contains entries
for all parties != m. For all parties where id < m, it will contain an address
and a port and will behave as a TCP client. For all parties where id > m, it
will contain a port (and an address that will be ignored) and will behave as a
TCP server. We expect the gap between port numbers are large enough to allow
establishing multiple connections (>3) between each party pair.
  */
  SocketPartyCommunicationAgentFactory(
      int myId,
      std::map<int, PartyInfo> partyInfos)
      : myId_(myId),
        partyInfos_(std::move(partyInfos)),
        useTls_(false),
        tlsDir_("") {}

  SocketPartyCommunicationAgentFactory(
      int myId,
      std::map<int, PartyInfo> partyInfos,
      bool useTls,
      std::string tlsDir)
      : myId_(myId),
        partyInfos_(std::move(partyInfos)),
        useTls_(useTls),
        tlsDir_(tlsDir) {}

  /**
   * create an agent that talks to a certain party
   */
  std::unique_ptr<IPartyCommunicationAgent> create(int id) override {
    if (id == myId_) {
      throw std::runtime_error("No need to talk to myself!");
    } else {
      auto iter = partyInfos_.find(id);
      if (iter == partyInfos_.end()) {
        throw std::runtime_error("Don't know how to connect to this party!");
      }
      // increasing port number since each connection will exclusively occupy a
      // port number. Need to use a new one for next new connection.
      auto portNo = iter->second.portNo++;
      if (id > myId_) {
        return std::make_unique<SocketPartyCommunicationAgent>(
            portNo, useTls_, tlsDir_);
      } else {
        return std::make_unique<SocketPartyCommunicationAgent>(
            iter->second.address, portNo, useTls_, tlsDir_);
      }
    }
  }

 private:
  int myId_;
  std::map<int, PartyInfo> partyInfos_;

  /*
    useTls_ and tlsDir_ need to be different because
    for one-way TLS, the client does not need any certs
    but still needs to be part of the handshake
    */
  bool useTls_;
  std::string tlsDir_;
};

} // namespace fbpcf::engine::communication
