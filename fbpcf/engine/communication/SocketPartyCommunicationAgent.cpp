/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/engine/communication/SocketPartyCommunicationAgent.h"

#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <chrono>
#include <fstream>
#include <istream>
#include <stdexcept>
#include <thread>

#include "folly/logging/xlog.h"

namespace fbpcf::engine::communication {

const std::string CERT_FILE = "cert.pem";
const std::string PRIVATE_KEY_FILE = "key.pem";
const std::string PASSPHRASE_FILE = "passphrase.pem";

/*
Per openSSL documentation, this callback is used to provide
the passphrase to open the private key file. See
https://www.openssl.org/docs/manmaster/man3/SSL_CTX_set_default_passwd_cb.html.
*/
static int
passwordCallback(char* buf, int size, int /* rwflag */, void* userdata) {
  strncpy(buf, (char*)userdata, size);
  buf[size - 1] = '\0';
  return strlen((char*)userdata);
}

SocketPartyCommunicationAgent::SocketPartyCommunicationAgent(
    int sockFd,
    int portNo,
    bool useTls,
    std::string tlsDir,
    std::shared_ptr<PartyCommunicationAgentTrafficRecorder> recorder)
    : recorder_(recorder),
      ssl_(nullptr),
      timeoutInSec_(1800 /* use a defaule value to avoid error*/) {
  if (useTls) {
    openServerPortWithTls(sockFd, portNo, tlsDir);
  } else {
    openServerPort(sockFd, portNo);
  }
}

SocketPartyCommunicationAgent::SocketPartyCommunicationAgent(
    int sockFd,
    int portNo,
    TlsInfo tlsInfo,
    std::shared_ptr<PartyCommunicationAgentTrafficRecorder> recorder,
    int timeoutInSec)
    : recorder_(recorder),
      ssl_(nullptr),
      tlsInfo_(tlsInfo),
      timeoutInSec_(timeoutInSec) {
  if (tlsInfo.useTls) {
    openServerPortWithTls(sockFd, portNo, tlsInfo);
  } else {
    openServerPort(sockFd, portNo);
  }
}

SocketPartyCommunicationAgent::SocketPartyCommunicationAgent(
    const std::string& serverAddress,
    int portNo,
    bool useTls,
    std::string tlsDir,
    std::shared_ptr<PartyCommunicationAgentTrafficRecorder> recorder)
    : recorder_(recorder),
      ssl_(nullptr),
      timeoutInSec_(1800 /* use a defaule value to avoid error*/) {
  if (useTls) {
    openClientPortWithTls(serverAddress, portNo, tlsDir);
  } else {
    openClientPort(serverAddress, portNo);
  }
}

SocketPartyCommunicationAgent::SocketPartyCommunicationAgent(
    const std::string& serverAddress,
    int portNo,
    TlsInfo tlsInfo,
    std::shared_ptr<PartyCommunicationAgentTrafficRecorder> recorder,
    int timeoutInSec)
    : recorder_(recorder),
      ssl_(nullptr),
      tlsInfo_(tlsInfo),
      timeoutInSec_(timeoutInSec) {
  if (tlsInfo.useTls) {
    openClientPortWithTls(serverAddress, portNo, tlsInfo);
  } else {
    openClientPort(serverAddress, portNo);
  }
}

SocketPartyCommunicationAgent::~SocketPartyCommunicationAgent() {
  if (!ssl_) {
    fclose(outgoingPort_);
    fclose(incomingPort_);
  } else {
    SSL_shutdown(ssl_);
    SSL_free(ssl_);
  }
}

void SocketPartyCommunicationAgent::sendImpl(const void* data, int nBytes) {
  size_t bytesWritten;
  if (!ssl_) {
    bytesWritten = fwrite(data, sizeof(unsigned char), nBytes, outgoingPort_);
  } else {
    bytesWritten = SSL_write(ssl_, data, nBytes);
  }
  assert(bytesWritten == nBytes);
  recorder_->addSentData(bytesWritten);
  if (!ssl_) {
    fflush(outgoingPort_);
  }
}

void SocketPartyCommunicationAgent::recvImpl(void* data, int nBytes) {
  size_t bytesRead = 0;

  if (!ssl_) {
    bytesRead = fread(data, sizeof(unsigned char), nBytes, incomingPort_);
  } else {
    // fread is blocking, but SSL_read is nonblocking. This discrepancy
    // can cause issues at the application level. We need to make sure that
    // both APIs behave consistently, so here we add a loop to ensure we
    // mimick blocking behavior.
    while (bytesRead < nBytes) {
      bytesRead += SSL_read(
          ssl_,
          (unsigned char*)data + (bytesRead * sizeof(unsigned char)),
          nBytes - bytesRead);
    }
  }
  assert(bytesRead == nBytes);
  recorder_->addReceivedData(bytesRead);
}

void SocketPartyCommunicationAgent::openServerPort(int sockFd, int portNo) {
  XLOG(INFO) << "try to connect as server at port " << portNo;

  auto acceptedConnection = receiveFromClient(sockFd);

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

void SocketPartyCommunicationAgent::openServerPortWithTls(
    int sockFd,
    int portNo,
    std::string tlsDir) {
  XLOG(INFO) << "try to connect as server at port " << portNo << " with TLS";
  const SSL_METHOD* method;
  SSL_CTX* ctx;

  method = TLS_server_method();
  ctx = SSL_CTX_new(method);

  // ensure we use TLS 1.3
  SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION);

  // Set passphrase for reading key.pem
  SSL_CTX_set_default_passwd_cb(ctx, passwordCallback);

  auto passphrase_file = tlsDir + "/" + PASSPHRASE_FILE;
  std::ifstream file_ptr(passphrase_file);
  std::string passphrase_string = "";
  file_ptr >> passphrase_string;
  file_ptr.close();
  SSL_CTX_set_default_passwd_cb_userdata(ctx, (void*)passphrase_string.c_str());

  if (ctx == nullptr) {
    auto errorMsg = getErrorInfo();
    XLOGF(INFO, "error message: {}", errorMsg);
    throw std::runtime_error("Could not create tls context " + errorMsg);
  }

  // Load the certificate file
  if (SSL_CTX_use_certificate_file(
          ctx, (tlsDir + "/" + CERT_FILE).c_str(), SSL_FILETYPE_PEM) <= 0) {
    auto errorMsg = getErrorInfo();
    XLOGF(INFO, "error message: {}", errorMsg);
    throw std::runtime_error("Error using certificate file " + errorMsg);
  }

  // Load the private key file
  if (SSL_CTX_use_PrivateKey_file(
          ctx, (tlsDir + "/" + PRIVATE_KEY_FILE).c_str(), SSL_FILETYPE_PEM) <=
      0) {
    auto errorMsg = getErrorInfo();
    XLOGF(INFO, "error message: {}", errorMsg);
    throw std::runtime_error("Error using private key file " + errorMsg);
  }

  auto acceptedConnection = receiveFromClient(sockFd);

  const auto ssl = SSL_new(ctx);
  SSL_CTX_free(ctx);
  SSL_set_fd(ssl, acceptedConnection);

  // Accept handshake from client
  if (SSL_accept(ssl) <= 0) {
    auto errorMsg = getErrorInfo();
    XLOGF(INFO, "error message: {}", errorMsg);
    throw std::runtime_error("Error on accepting ssl " + errorMsg);
  }

  XLOG(INFO) << "connected as server at port " << portNo << " with TLS";

  ssl_ = ssl;
}

void SocketPartyCommunicationAgent::openServerPortWithTls(
    int sockFd,
    int portNo,
    TlsInfo tlsInfo) {
  XLOG(INFO) << "try to connect as server at port " << portNo << " with TLS";
  const SSL_METHOD* method;
  SSL_CTX* ctx;

  method = TLS_server_method();
  ctx = SSL_CTX_new(method);

  // ensure we use TLS 1.3
  SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION);

  // Set passphrase for reading key.pem
  SSL_CTX_set_default_passwd_cb(ctx, passwordCallback);

  auto passphrase_file = tlsInfo.passphrasePath;
  std::ifstream file_ptr(passphrase_file);
  std::string passphrase_string = "";
  file_ptr >> passphrase_string;
  file_ptr.close();
  SSL_CTX_set_default_passwd_cb_userdata(ctx, (void*)passphrase_string.c_str());

  if (ctx == nullptr) {
    auto errorMsg = getErrorInfo();
    XLOGF(INFO, "error message: {}", errorMsg);
    throw std::runtime_error("Could not create tls context " + errorMsg);
  }

  // Load the certificate file
  XLOGF(INFO, "Using certificate file at: {}", tlsInfo.certPath);
  if (SSL_CTX_use_certificate_file(
          ctx, (tlsInfo.certPath).c_str(), SSL_FILETYPE_PEM) <= 0) {
    auto errorMsg = getErrorInfo();
    XLOGF(INFO, "error message: {}", errorMsg);
    throw std::runtime_error("Error using certificate file " + errorMsg);
  }

  // Load the private key file
  XLOGF(INFO, "Using private key file at: {}", tlsInfo.keyPath);
  if (SSL_CTX_use_PrivateKey_file(
          ctx, (tlsInfo.keyPath).c_str(), SSL_FILETYPE_PEM) <= 0) {
    auto errorMsg = getErrorInfo();
    XLOGF(INFO, "error message: {}", errorMsg);
    throw std::runtime_error("Error using private key file " + errorMsg);
  }

  auto acceptedConnection = receiveFromClient(sockFd);

  const auto ssl = SSL_new(ctx);
  SSL_CTX_free(ctx);
  SSL_set_fd(ssl, acceptedConnection);

  // Accept handshake from client
  if (SSL_accept(ssl) <= 0) {
    auto errorMsg = getErrorInfo();
    XLOGF(INFO, "error message: {}", errorMsg);
    throw std::runtime_error("Error on accepting ssl " + errorMsg);
  }

  XLOG(INFO) << "connected as server at port " << portNo << " with TLS";

  ssl_ = ssl;
}

void SocketPartyCommunicationAgent::openClientPortWithTls(
    const std::string& serverAddress,
    int portNo,
    std::string /* tls_dir */) {
  XLOGF(
      INFO,
      "try to connect as client to {} at port {} with TLS",
      serverAddress,
      portNo);
  const SSL_METHOD* method = TLS_client_method();
  SSL_CTX* ctx = SSL_CTX_new(method);

  // ensure we use TLS 1.3
  SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION);

  if (ctx == nullptr) {
    auto errorMsg = getErrorInfo();
    XLOGF(INFO, "error message: {}", errorMsg);
    throw std::runtime_error("could not create tls context " + errorMsg);
  }

  SSL* ssl = SSL_new(ctx);
  SSL_CTX_free(ctx);
  if (ssl == nullptr) {
    auto errorMsg = getErrorInfo();
    XLOGF(INFO, "error message: {}", errorMsg);
    throw std::runtime_error("could not create tls object " + errorMsg);
  }

  const auto sockfd = connectToHost(serverAddress, portNo);

  SSL_set_fd(ssl, sockfd);

  // initiate handshake with server
  const int status = SSL_connect(ssl);
  if (status != 1) {
    auto errorMsg = getErrorInfo();
    XLOGF(INFO, "error message: {}", errorMsg);
    throw std::runtime_error("could not complete tls handshake " + errorMsg);
  }

  XLOGF(INFO, "connected as client to {} at port {}", serverAddress, portNo);

  ssl_ = ssl;
}

void SocketPartyCommunicationAgent::openClientPortWithTls(
    const std::string& serverAddress,
    int portNo,
    TlsInfo tlsInfo) {
  XLOGF(
      INFO,
      "try to connect as client to {} at port {} with TLS",
      serverAddress,
      portNo);
  const SSL_METHOD* method = TLS_client_method();
  SSL_CTX* ctx = SSL_CTX_new(method);

  // ensure we use TLS 1.3
  SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION);

  // specify the trusted root CA
  XLOGF(INFO, "Using root CA cert file at: {}", tlsInfo.rootCaCertPath);
  if (!tlsInfo.rootCaCertPath.empty()) {
    if (SSL_CTX_load_verify_locations(
            ctx, tlsInfo.rootCaCertPath.c_str(), nullptr) == 0) {
      throw std::runtime_error("failed to set root CA");
    }
  }

  if (ctx == nullptr) {
    auto errorMsg = getErrorInfo();
    XLOGF(INFO, "error message: {}", errorMsg);
    throw std::runtime_error("could not create tls context " + errorMsg);
  }

  // make sure to verify server cert
  SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);

  // and verify the domain name
  auto params = SSL_CTX_get0_param(ctx);
  X509_VERIFY_PARAM_set1_host(
      params, serverAddress.c_str(), serverAddress.size());

  SSL* ssl = SSL_new(ctx);
  SSL_CTX_free(ctx);
  if (ssl == nullptr) {
    auto errorMsg = getErrorInfo();
    XLOGF(INFO, "error message: {}", errorMsg);
    throw std::runtime_error("could not create tls object " + errorMsg);
  }

  const auto sockfd = connectToHost(serverAddress, portNo);

  SSL_set_fd(ssl, sockfd);

  // initiate handshake with server
  const int status = SSL_connect(ssl);
  X509* cert = SSL_get_peer_certificate(ssl);
  auto result = X509_verify_cert_error_string(SSL_get_verify_result(ssl));
  XLOGF(INFO, "verify result: {}", result);

  if (status != 1) {
    auto errorMsg = getErrorInfo();
    XLOGF(INFO, "error message: {}", errorMsg);
    throw std::runtime_error("could not complete tls handshake " + errorMsg);
  }

  auto line = X509_NAME_oneline(X509_get_subject_name(cert), nullptr, 0);
  XLOGF(INFO, "Server certificate: {}", line);
  line = X509_NAME_oneline(X509_get_issuer_name(cert), nullptr, 0);
  XLOGF(INFO, "Server certificate issuer: {}", line);

  XLOGF(
      INFO,
      "connected as client to {} at port {} with TLS",
      serverAddress,
      portNo);

  ssl_ = ssl;
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

  int enable = 1;

  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
    XLOG(INFO) << "setsockopt(SO_REUSEADDR) failed";
  }

  retryCount = timeoutInSec_ * 10;
  while (connect(sockfd, addrs->ai_addr, addrs->ai_addrlen) < 0) {
    // wait a second and retry
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    close(sockfd);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (retryCount-- < 0) {
      throw std::runtime_error(
          "Timeout when trying to establish a connection as client.");
    }
  }

  freeaddrinfo(addrs);

  return sockfd;
}

int SocketPartyCommunicationAgent::receiveFromClient(int sockfd) {
  struct timeval tv;
  fd_set rfds;
  FD_ZERO(&rfds);
  FD_SET(sockfd, &rfds);
  tv.tv_sec = (long)timeoutInSec_;
  tv.tv_usec = 0;

  if (select(sockfd + 1, &rfds, (fd_set*)0, (fd_set*)0, &tv) > 0) {
    struct sockaddr_in cli_addr;
    unsigned int clilen = sizeof(cli_addr);

    auto acceptedConnection =
        accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
    if (acceptedConnection < 0) {
      throw std::runtime_error("error on accepting");
    }

    // Log the client IP address
    char client_ip[INET_ADDRSTRLEN];
    if (!inet_ntop(AF_INET, &cli_addr.sin_addr, client_ip, INET_ADDRSTRLEN)) {
      XLOG(WARN, "Issue converting client ip address");
    } else {
      XLOGF(INFO, "Client connected from {}", client_ip);
    }
    close(sockfd);
    return acceptedConnection;
  } else {
    throw std::runtime_error(
        "Timeout when trying to establish a connection as server.");
  }
}

} // namespace fbpcf::engine::communication
