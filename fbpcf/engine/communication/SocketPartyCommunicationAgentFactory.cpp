/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/engine/communication/SocketPartyCommunicationAgentFactory.h"
#include <stdexcept>
#include <string>

#include "folly/logging/xlog.h"

namespace fbpcf::engine::communication {

SocketPartyCommunicationAgent::TlsInfo getTlsInfoFromArgs(
    bool useTls,
    std::string ca_cert_path,
    std::string server_cert_path,
    std::string private_key_path,
    std::string passphrase_path) {
  const char* home_dir = std::getenv("HOME");
  if (home_dir == nullptr) {
    home_dir = "";
  }

  std::string home_dir_string(home_dir);

  fbpcf::engine::communication::SocketPartyCommunicationAgent::TlsInfo tlsInfo;
  tlsInfo.useTls = useTls;
  tlsInfo.rootCaCertPath = useTls ? (home_dir_string + "/" + ca_cert_path) : "";
  tlsInfo.certPath = useTls ? (home_dir_string + "/" + server_cert_path) : "";
  tlsInfo.keyPath = useTls ? (home_dir_string + "/" + private_key_path) : "";
  tlsInfo.passphrasePath =
      useTls ? (home_dir_string + "/" + passphrase_path) : "";

  return tlsInfo;
}

std::unique_ptr<IPartyCommunicationAgent>
SocketPartyCommunicationAgentFactory::create(int id, std::string name) {
  if (id == myId_) {
    throw std::runtime_error("No need to talk to myself!");
  } else {
    auto iter = initialConnections_.find(id);
    if (iter == initialConnections_.end()) {
      throw std::runtime_error("Don't know how to connect to this party!");
    }
    auto recorder = std::make_shared<PartyCommunicationAgentTrafficRecorder>();
    metricCollector_->addNewRecorder(name, recorder);

    if (id > myId_) {
      // We first try to bind on the assigned port number (and its nexts). If
      // that failed, we will try to bind to a free port instead. In either
      // case, we will tell the client which port to use.
      auto assignedPortNo = ++iter->second.first.portNo;
      auto [socket, portNo] = createSocketFromMaybeFreePort(assignedPortNo);
      iter->second.second->sendSingleT<int>(portNo);
      return std::make_unique<SocketPartyCommunicationAgent>(
          socket, portNo, tlsInfo_, recorder, timeoutInSec_);
    } else {
      auto portNo = iter->second.second->receiveSingleT<int>();
      return std::make_unique<SocketPartyCommunicationAgent>(
          iter->second.first.address,
          portNo,
          tlsInfo_,
          recorder,
          timeoutInSec_);
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
    XLOGF(
        INFO,
        "Failed to bind on the assigned port: {}, binding to a free one instead.",
        portNo);
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

  return {sockfd, getPortFromSocket(sockfd)};
}

int SocketPartyCommunicationAgentFactory::getPortFromSocket(int sockfd) {
  struct sockaddr_in actualAddr;

  socklen_t len = sizeof(actualAddr);
  if (::getsockname(sockfd, (struct sockaddr*)&actualAddr, &len) != 0) {
    throw std::runtime_error("error on getting socket name.");
  }
  if (ntohs(actualAddr.sin_port) == 0) {
    throw std::runtime_error("port number shouldn't be 0.");
  }

  return ntohs(actualAddr.sin_port);
}

void SocketPartyCommunicationAgentFactory::setupInitialSockets(
    const std::map<int, PartyInfo>& partyInfos) {
  for (const auto& [partyId, partyInfo] : partyInfos) {
    if (myId_ < partyId) {
      auto [socket, port] = createSocketFromMaybeFreePort(partyInfo.portNo);
      sockets_.insert({partyId, socket});
    }
  }
}

void SocketPartyCommunicationAgentFactory::setupInitialConnection(
    const std::map<int, PartyInfo>& partyInfos) {
  for (const auto& [partyId, partyInfo] : partyInfos) {
    if (myId_ < partyId) {
      auto recorder =
          std::make_shared<PartyCommunicationAgentTrafficRecorder>();
      metricCollector_->addNewRecorder(
          "Port_number_sync_traffic_with_party_" + std::to_string(partyId),
          recorder);
      initialConnections_.insert(
          {partyId,
           std::make_pair(
               partyInfo,
               std::make_unique<SocketPartyCommunicationAgent>(
                   sockets_.at(partyId),
                   partyInfo.portNo,
                   tlsInfo_,
                   recorder,
                   timeoutInSec_))});

    } else if (myId_ > partyId) {
      auto recorder =
          std::make_shared<PartyCommunicationAgentTrafficRecorder>();
      metricCollector_->addNewRecorder(
          "Port_number_sync_traffic_with_party_" + std::to_string(partyId),
          recorder);
      initialConnections_.insert(
          {partyId,
           std::make_pair(
               partyInfo,
               std::make_unique<SocketPartyCommunicationAgent>(
                   partyInfo.address,
                   partyInfo.portNo,
                   tlsInfo_,
                   recorder,
                   timeoutInSec_))});
    }
  }
}

} // namespace fbpcf::engine::communication
