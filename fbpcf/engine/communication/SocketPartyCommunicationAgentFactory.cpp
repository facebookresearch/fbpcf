/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/engine/communication/SocketPartyCommunicationAgentFactory.h"
#include <stdexcept>

#include "folly/logging/xlog.h"

namespace fbpcf::engine::communication {

std::unique_ptr<IPartyCommunicationAgent>
SocketPartyCommunicationAgentFactory::create(int id) {
  if (id == myId_) {
    throw std::runtime_error("No need to talk to myself!");
  } else {
    auto iter = initialConnections_.find(id);
    if (iter == initialConnections_.end()) {
      throw std::runtime_error("Don't know how to connect to this party!");
    }

    if (id > myId_) {
      // We first try to bind on the assigned port number (and its nexts). If
      // that failed, we will try to bind to a free port instead. In either
      // case, we will tell the client which port to use.
      auto assignedPortNo = ++iter->second.first.portNo;
      auto [socket, portNo] = createSocketFromMaybeFreePort(assignedPortNo);
      iter->second.second->sendSingleT<int>(portNo);
      return std::make_unique<SocketPartyCommunicationAgent>(
          socket, portNo, useTls_, tlsDir_);
    } else {
      auto portNo = iter->second.second->receiveSingleT<int>();
      return std::make_unique<SocketPartyCommunicationAgent>(
          iter->second.first.address, portNo, useTls_, tlsDir_);
    }
  }
}

std::pair<int, int>
SocketPartyCommunicationAgentFactory::createSocketFromMaybeFreePort(
    int portNo) {
  auto sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    throw std::runtime_error("error opening socket");
  }
  int enable = 1;

  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
    XLOG(INFO) << "setsockopt(SO_REUSEADDR) failed";
  }

  struct sockaddr_in servAddr;

  memset((char*)&servAddr, 0, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = INADDR_ANY;
  servAddr.sin_port = htons(portNo);

  // if binding to the original port number fails, try to grab a free port
  // instead
  if (::bind(sockfd, (struct sockaddr*)&servAddr, sizeof(struct sockaddr_in)) <
      0) {
    XLOG(INFO)
        << "Failed to bind on the assigned port, binding to a free one instead.";
    portNo = 0;
    servAddr.sin_port = htons(portNo);
    if (::bind(
            sockfd, (struct sockaddr*)&servAddr, sizeof(struct sockaddr_in)) <
        0) {
      // throw when failed to bind to a random free port.
      throw std::runtime_error("error on binding to a free port");
    }
  }
  // only expect 1 client to connect
  listen(sockfd, 1);

  // port number is known.
  if (portNo != 0) {
    return {sockfd, portNo};
  }

  struct sockaddr_in actualAddr;

  socklen_t len = sizeof(actualAddr);
  if (::getsockname(sockfd, (struct sockaddr*)&actualAddr, &len) != 0) {
    throw std::runtime_error("error on getting socket name.");
  }
  if (ntohs(actualAddr.sin_port) == 0) {
    throw std::runtime_error("port number shouldn't be 0.");
  }
  return {sockfd, ntohs(actualAddr.sin_port)};
}

void SocketPartyCommunicationAgentFactory::setupInitialConnection(
    const std::map<int, PartyInfo>& partyInfos) {
  for (auto& item : partyInfos) {
    if (myId_ < item.first) {
      auto [socket, _] = createSocketFromMaybeFreePort(item.second.portNo);
      initialConnections_.insert(
          {item.first,
           std::make_pair(
               item.second,
               std::make_unique<SocketPartyCommunicationAgent>(
                   socket, item.second.portNo, useTls_, tlsDir_))});

    } else if (myId_ > item.first) {
      initialConnections_.insert(
          {item.first,
           std::make_pair(
               item.second,
               std::make_unique<SocketPartyCommunicationAgent>(
                   item.second.address,
                   item.second.portNo,
                   useTls_,
                   tlsDir_))});
    }
  }
}

} // namespace fbpcf::engine::communication
