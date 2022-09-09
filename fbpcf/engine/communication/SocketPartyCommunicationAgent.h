/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <openssl/opensslv.h>
#include <openssl/ssl.h>
#include <string>

#include "fbpcf/engine/communication/IPartyCommunicationAgent.h"

namespace fbpcf::engine::communication {

/**
 * This object connect two parties on different machines via socket. This object
 * is merely connecting two ports. It assumes the security/privacy of the
 * underlying infra (e.g. TLS).
 */
class SocketPartyCommunicationAgent final : public IPartyCommunicationAgent {
 public:
  struct TlsInfo {
    bool useTls;
    std::string certPath;
    std::string keyPath;
    std::string passphrasePath;
  };
  /**
   * Create as socket server, optionally with TLS.
   */
  [[deprecated("Use the constructor with TlsInfo instead.")]] explicit SocketPartyCommunicationAgent(
      int sockFd,
      int portNo,
      bool useTls,
      std::string tlsDir,
      std::shared_ptr<PartyCommunicationAgentTrafficRecorder> recorder);

  explicit SocketPartyCommunicationAgent(
      int sockFd,
      int portNo,
      TlsInfo tlsInfo,
      std::shared_ptr<PartyCommunicationAgentTrafficRecorder> recorder);

  /**
   * Created as socket client, optionally with TLS.
   */
  [[deprecated("Use the constructor with TlsInfo instead.")]] SocketPartyCommunicationAgent(
      const std::string& serverAddress,
      int portNo,
      bool useTls,
      std::string tlsDir,
      std::shared_ptr<PartyCommunicationAgentTrafficRecorder> recorder);

  SocketPartyCommunicationAgent(
      const std::string& serverAddress,
      int portNo,
      TlsInfo tlsInfo,
      std::shared_ptr<PartyCommunicationAgentTrafficRecorder> recorder);

  ~SocketPartyCommunicationAgent() override;

  /**
   * @inherit doc
   */
  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    return recorder_->getTrafficStatistics();
  }

  void recvImpl(void* data, int nBytes) override;

  void sendImpl(const void* data, int nBytes) override;

 private:
  void openServerPort(int sockFd, int portNo);
  void openClientPort(const std::string& serverAddress, int portNo);

  void openServerPortWithTls(int sockFd, int portNo, std::string tlsDir);
  void openServerPortWithTls(int sockFd, int portNo, TlsInfo tlsInfo);
  void openClientPortWithTls(
      const std::string& serverAddress,
      int portNo,
      std::string tlsDir);
  void openClientPortWithTls(
      const std::string& serverAddress,
      int portNo,
      TlsInfo tlsInfo);

  /*
   * helper functions for shared code between TLS and non-TLS implementations
   */
  int connectToHost(const std::string& serverAddress, int portNo);
  int receiveFromClient(int portNo);
  FILE* incomingPort_;
  FILE* outgoingPort_;

  std::shared_ptr<PartyCommunicationAgentTrafficRecorder> recorder_;

  SSL* ssl_;
  TlsInfo tlsInfo_;
};

} // namespace fbpcf::engine::communication
