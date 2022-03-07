/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <map>

#include "fbpcf/mpc_framework/engine/communication/IPartyCommunicationAgentFactory.h"
#include "fbpcf/mpc_framework/engine/communication/SocketPartyCommunicationAgent.h"

namespace fbpcf::mpc_framework::engine::communication {

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

  // it's OK if a party with a smaller id doesn't know a party with larger id's
  // ip address, since the party with smaller id will always be the server.
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
      auto serverId = id < myId_ ? id : myId_;
      auto iter = partyInfos_.find(serverId);
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

} // namespace fbpcf::mpc_framework::engine::communication
