/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

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
  /**
   * Create as socket server, optionally with TLS.
   */
  explicit SocketPartyCommunicationAgent(
      int sockFd,
      int portNo,
      bool useTls,
      std::string tlsDir);

  /**
   * Created as socket client, optionally with TLS.
   */
  SocketPartyCommunicationAgent(
      const std::string& serverAddress,
      int portNo,
      bool useTls,
      std::string tlsDir);

  ~SocketPartyCommunicationAgent() override;

  /**
   * @inherit doc
   */
  void send(const std::vector<unsigned char>& data) override;

  /**
   * @inherit doc
   */
  std::vector<unsigned char> receive(size_t size) override;

  /**
   * @inherit doc
   */
  std::pair<uint64_t, uint64_t> getTrafficStatistics() const override {
    return {sentData_, receivedData_};
  }

  void recvImpl(void* data, int nBytes) override;

  void sendImpl(const void* data, int nBytes) override;

 private:
  void openServerPort(int sockFd, int portNo);
  void openClientPort(const std::string& serverAddress, int portNo);

  void openServerPortWithTls(int sockFd, int portNo, std::string tlsDir);
  void openClientPortWithTls(
      const std::string& serverAddress,
      int portNo,
      std::string tlsDir);

  /*
   * helper functions for shared code between TLS and non-TLS implementations
   */
  int connectToHost(const std::string& serverAddress, int portNo);
  int receiveFromClient(int portNo);
  FILE* incomingPort_;
  FILE* outgoingPort_;

  uint64_t sentData_;
  uint64_t receivedData_;

  SSL* ssl_;
};

} // namespace fbpcf::engine::communication
