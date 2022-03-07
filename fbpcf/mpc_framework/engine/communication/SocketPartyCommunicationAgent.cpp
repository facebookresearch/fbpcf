/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/mpc_framework/engine/communication/SocketPartyCommunicationAgent.h"

#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>
#include <string>

#include "folly/logging/xlog.h"

namespace fbpcf::mpc_framework::engine::communication {

SocketPartyCommunicationAgent::SocketPartyCommunicationAgent(
    int portNo,
    bool useTls,
    std::string tlsDir)
    : sentData_(0), receivedData_(0) {
  openServerPort(portNo);
}

SocketPartyCommunicationAgent::SocketPartyCommunicationAgent(
    const std::string& serverAddress,
    int portNo,
    bool useTls,
    std::string tlsDir)
    : sentData_(0), receivedData_(0) {
  openClientPort(serverAddress, portNo);
}

SocketPartyCommunicationAgent::~SocketPartyCommunicationAgent() {
  fclose(outgoingPort_);
  fclose(incomingPort_);
}

void SocketPartyCommunicationAgent::send(
    const std::vector<unsigned char>& data) {
  auto s =
      fwrite(data.data(), sizeof(unsigned char), data.size(), outgoingPort_);
  assert(s == data.size());
  sentData_ += s;
  fflush(outgoingPort_);
}

std::vector<unsigned char> SocketPartyCommunicationAgent::receive(int size) {
  std::vector<unsigned char> rst(size);
  auto s = fread(rst.data(), sizeof(unsigned char), size, incomingPort_);
  assert(s == size);
  receivedData_ += s;
  return rst;
}

void SocketPartyCommunicationAgent::openServerPort(int portNo) {
  XLOG(INFO) << "try to connect as server at port " << portNo;

  auto acceptedConnection = receiveFromClient(portNo);

  auto duplicatedConnection = dup(acceptedConnection);
  if (duplicatedConnection < 0) {
    throw std::runtime_error("error on duplicate socket");
  }

  outgoingPort_ = fdopen(acceptedConnection, "w");
  incomingPort_ = fdopen(duplicatedConnection, "r");

  XLOG(INFO) << "connected as server at port " << portNo;
  return;
}

void SocketPartyCommunicationAgent::openClientPort(
    const std::string& serverAddress,
    int portNo) {
  XLOG(INFO) << "try to connect as client to " << serverAddress << " at port "
             << portNo;

  const auto sockfd = connectToHost(serverAddress, portNo);

  auto duplicatedConnection = dup(sockfd);
  if (duplicatedConnection < 0) {
    throw std::runtime_error("error on duplicate socket");
  }
  incomingPort_ = fdopen(sockfd, "r");
  outgoingPort_ = fdopen(duplicatedConnection, "w");

  XLOG(INFO) << "connected as client to " << serverAddress << " at port "
             << portNo;
  return;
}

int SocketPartyCommunicationAgent::connectToHost(
    const std::string& serverAddress,
    int portNo) {
  auto portString = std::to_string(portNo);

  struct addrinfo hints = {};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  struct addrinfo* addrs;

  auto signal =
      getaddrinfo(serverAddress.data(), portString.data(), &hints, &addrs);
  int retryCount = 10;
  while (((signal != 0) || (addrs == nullptr) || (addrs->ai_addr == nullptr)) &&
         (retryCount > 0)) {
    XLOG(INFO) << "getaddrinfo() failed, retrying, remaining attempt "
               << retryCount;
    signal =
        getaddrinfo(serverAddress.data(), portString.data(), &hints, &addrs);
    retryCount--;
  }
  if ((signal != 0) || (addrs == nullptr) || (addrs->ai_addr == nullptr)) {
    throw std::runtime_error(
        "Can't get address info " + std::string(serverAddress.data()) + " " +
        portString.data());
  }
  auto sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    throw std::runtime_error("error opening socket");
  }

  while (connect(sockfd, addrs->ai_addr, addrs->ai_addrlen) < 0) {
    // wait a second and retry
    usleep(1000);
    close(sockfd);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
  }

  freeaddrinfo(addrs);

  return sockfd;
}

int SocketPartyCommunicationAgent::receiveFromClient(int portNo) {
  auto sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    throw std::runtime_error("error opening socket");
  }

  struct sockaddr_in servAddr;

  memset((char*)&servAddr, 0, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = INADDR_ANY;
  servAddr.sin_port = htons(portNo);

  // throw an exception if binding to socket failed
  if (::bind(sockfd, (struct sockaddr*)&servAddr, sizeof(struct sockaddr_in)) <
      0) {
    throw std::runtime_error("error on binding");
  }

  // only expect 1 client to connect
  listen(sockfd, 1);

  struct sockaddr_in cli_addr;
  socklen_t clilen = sizeof(struct sockaddr_in);
  auto acceptedConnection =
      accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);

  if (acceptedConnection < 0) {
    throw std::runtime_error("error on accepting");
  }
  close(sockfd);

  return acceptedConnection;
}

} // namespace fbpcf::mpc_framework::engine::communication
