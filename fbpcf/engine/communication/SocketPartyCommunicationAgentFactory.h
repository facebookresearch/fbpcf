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

namespace fbpcf::engine::communication {

SocketPartyCommunicationAgent::TlsInfo getTlsInfoFromArgs(
    bool useTls,
    std::string ca_cert_path,
    std::string server_cert_path,
    std::string private_key_path,
    std::string passphrase_path);

/**
 * An communication factory API
 */
class SocketPartyCommunicationAgentFactory
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
  [[deprecated("Use the constructor with TlsInfo instead.")]] SocketPartyCommunicationAgentFactory(
      int myId,
      std::map<int, PartyInfo> partyInfos,
      std::string myname)
      : IPartyCommunicationAgentFactory(myname),
        myId_(myId),
        useTls_(false),
        tlsDir_(""),
        partyInfos_(partyInfos),
        timeoutInSec_(1800 /* add a default value to avoid error*/) {
    SocketPartyCommunicationAgent::TlsInfo tlsInfo;
    tlsInfo.useTls = false;
    tlsInfo.certPath = "";
    tlsInfo.keyPath = "";
    tlsInfo.passphrasePath = "";
    tlsInfo_ = tlsInfo;
    setupInitialSockets(partyInfos);
    setupInitialConnection(partyInfos);
  }

  [[deprecated("Use the constructor with TlsInfo instead.")]] SocketPartyCommunicationAgentFactory(
      int myId,
      std::map<int, PartyInfo> partyInfos,
      std::shared_ptr<fbpcf::util::MetricCollector> metricCollector)
      : IPartyCommunicationAgentFactory(metricCollector),
        myId_(myId),
        useTls_(false),
        tlsDir_(""),
        partyInfos_(partyInfos),
        timeoutInSec_(1800 /* add a default value to avoid error*/) {
    SocketPartyCommunicationAgent::TlsInfo tlsInfo;
    tlsInfo.useTls = false;
    tlsInfo.certPath = "";
    tlsInfo.keyPath = "";
    tlsInfo.passphrasePath = "";
    tlsInfo_ = tlsInfo;
    setupInitialSockets(partyInfos);
    setupInitialConnection(partyInfos);
  }

  [[deprecated("Use the constructor with TlsInfo instead.")]] SocketPartyCommunicationAgentFactory(
      int myId,
      std::map<int, PartyInfo> partyInfos,
      bool useTls,
      std::string tlsDir,
      std::string myname)
      : IPartyCommunicationAgentFactory(myname),
        myId_(myId),
        useTls_(useTls),
        tlsDir_(tlsDir),
        partyInfos_(partyInfos),
        timeoutInSec_(1800 /* add a default value to avoid error*/) {
    SocketPartyCommunicationAgent::TlsInfo tlsInfo;
    tlsInfo.useTls = useTls;
    tlsInfo.certPath = tlsDir + "/cert.pem";
    tlsInfo.keyPath = tlsDir + "/key.pem";
    tlsInfo.passphrasePath = tlsDir + "/passphrase.pem";
    tlsInfo_ = tlsInfo;
    setupInitialSockets(partyInfos);
    setupInitialConnection(partyInfos);
  }

  [[deprecated("Use the constructor with metricCollector instead.")]] SocketPartyCommunicationAgentFactory(
      int myId,
      std::map<int, PartyInfo> partyInfos,
      SocketPartyCommunicationAgent::TlsInfo tlsInfo,
      std::string myname)
      : IPartyCommunicationAgentFactory(myname),
        myId_(myId),
        tlsInfo_(tlsInfo),
        partyInfos_(partyInfos),
        timeoutInSec_(1800 /* add a default value to avoid error*/) {
    setupInitialSockets(partyInfos);
    setupInitialConnection(partyInfos);
  }

  [[deprecated("Use the constructor with a timeout instead.")]] SocketPartyCommunicationAgentFactory(
      int myId,
      std::map<int, PartyInfo> partyInfos,
      SocketPartyCommunicationAgent::TlsInfo tlsInfo,
      std::shared_ptr<fbpcf::util::MetricCollector> metricCollector)
      : IPartyCommunicationAgentFactory(metricCollector),
        myId_(myId),
        tlsInfo_(tlsInfo),
        partyInfos_(partyInfos),
        timeoutInSec_(1800 /* add a default value to avoid error*/) {
    setupInitialSockets(partyInfos);
    setupInitialConnection(partyInfos);
  }

  SocketPartyCommunicationAgentFactory(
      int myId,
      std::map<int, PartyInfo> partyInfos,
      SocketPartyCommunicationAgent::TlsInfo tlsInfo,
      std::shared_ptr<fbpcf::util::MetricCollector> metricCollector,
      int timeoutInSec)
      : IPartyCommunicationAgentFactory(metricCollector),
        myId_(myId),
        tlsInfo_(tlsInfo),
        partyInfos_(partyInfos),
        timeoutInSec_(timeoutInSec) {
    XLOGF(
        INFO,
        "use_tls: {}, ca_cert_path: {}, server_cert_path: {}, key_path: {}, passphrase_path: {}",
        tlsInfo.useTls,
        tlsInfo.rootCaCertPath,
        tlsInfo.certPath,
        tlsInfo.keyPath,
        tlsInfo.passphrasePath);
    setupInitialSockets(partyInfos);
    setupInitialConnection(partyInfos);
  }

  /**
   * create an agent that talks to a certain party
   */
  std::unique_ptr<IPartyCommunicationAgent> create(int id, std::string name)
      override;

 protected:
  /**
   * This constructor sets member variables but does
   * not make any connections. This is important
   * for tests, and thus is protected (not public).
   */
  SocketPartyCommunicationAgentFactory(
      int myId,
      std::map<int, PartyInfo> partyInfos,
      SocketPartyCommunicationAgent::TlsInfo tlsInfo,
      std::shared_ptr<fbpcf::util::MetricCollector> metricCollector,
      bool /* skipConnection */)
      : IPartyCommunicationAgentFactory(metricCollector),
        myId_(myId),
        tlsInfo_(tlsInfo),
        partyInfos_(partyInfos),
        timeoutInSec_(300 /* default value for test only */) {
    setupInitialSockets(partyInfos);
  }

  /**
   * @param portNo intended port number for the socket. If portNo=0, a random
   * free port number will be used instead
   * @return a {socket, port number} pair.
   */
  std::pair<int, int> createSocketFromMaybeFreePort(int portNo);

  int getPortFromSocket(int sockfd);

  void setupInitialSockets(const std::map<int, PartyInfo>& partyInfos);
  void setupInitialConnection(const std::map<int, PartyInfo>& partyInfos);

  int myId_;
  std::map<int, int> sockets_;
  std::map<
      int,
      std::pair<PartyInfo, std::unique_ptr<SocketPartyCommunicationAgent>>>
      initialConnections_;

  /*
    useTls_ and tlsDir_ need to be different because
    for one-way TLS, the client does not need any certs
    but still needs to be part of the handshake
    */
  bool useTls_;
  std::string tlsDir_;

  SocketPartyCommunicationAgent::TlsInfo tlsInfo_;
  std::map<int, PartyInfo> partyInfos_;

  int timeoutInSec_;
};

} // namespace fbpcf::engine::communication
