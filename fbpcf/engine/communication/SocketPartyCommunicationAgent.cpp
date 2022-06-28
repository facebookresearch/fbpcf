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
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <fstream>
#include <istream>

#include <folly/String.h>
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

/*
This function is only used temporarily since we only have self
signed certificates available. In the future, when we implement
a Private CA, this callback should not be used.
*/
static int callbackToSkipVerificationOfSelfSignedCert_UNSAFE(
    X509_STORE_CTX* /* ctx */,
    void* /* data */) {
  return 1; // always pass cert verification
}

SocketPartyCommunicationAgent::SocketPartyCommunicationAgent(
    int sockFd,
    int portNo,
    bool useTls,
    std::string tlsDir,
    std::shared_ptr<PartyCommunicationAgentTrafficRecorder> recorder)
    : recorder_(recorder), ssl_(nullptr) {
  if (useTls) {
    openServerPortWithTls(sockFd, portNo, tlsDir);
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
    : recorder_(recorder), ssl_(nullptr) {
  if (useTls) {
    openClientPortWithTls(serverAddress, portNo, tlsDir);
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

void SocketPartyCommunicationAgent::send(
    const std::vector<unsigned char>& data) {
  sendImpl(static_cast<const void*>(data.data()), data.size());
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

std::vector<unsigned char> SocketPartyCommunicationAgent::receive(size_t size) {
  std::vector<unsigned char> rst(size);
  recvImpl(static_cast<void*>(rst.data()), size);
  return rst;
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
  LOG(INFO) << "try to connect as server at port " << portNo << " with TLS";
  const SSL_METHOD* method;
  SSL_CTX* ctx;

  method = TLS_server_method();
  ctx = SSL_CTX_new(method);

  // Set passphrase for reading key.pem
  SSL_CTX_set_default_passwd_cb(ctx, passwordCallback);

  auto passphrase_file = tlsDir + "/" + PASSPHRASE_FILE;
  std::ifstream file_ptr(passphrase_file);
  std::string passphrase_string = "";
  file_ptr >> passphrase_string;
  file_ptr.close();
  SSL_CTX_set_default_passwd_cb_userdata(ctx, (void*)passphrase_string.c_str());

  if (ctx == nullptr) {
    LOG(INFO) << folly::errnoStr(errno);
    throw std::runtime_error("Could not create tls context");
  }

  // Load the certificate file
  if (SSL_CTX_use_certificate_file(
          ctx, (tlsDir + "/" + CERT_FILE).c_str(), SSL_FILETYPE_PEM) <= 0) {
    LOG(INFO) << folly::errnoStr(errno);
    throw std::runtime_error("Error using certificate file");
  }

  // Load the private key file
  if (SSL_CTX_use_PrivateKey_file(
          ctx, (tlsDir + "/" + PRIVATE_KEY_FILE).c_str(), SSL_FILETYPE_PEM) <=
      0) {
    LOG(INFO) << folly::errnoStr(errno);
    throw std::runtime_error("Error using private key file");
  }

  auto acceptedConnection = receiveFromClient(sockFd);

  const auto ssl = SSL_new(ctx);
  SSL_set_fd(ssl, acceptedConnection);

  // Accept handshake from client
  if (SSL_accept(ssl) <= 0) {
    LOG(INFO) << folly::errnoStr(errno);
    throw std::runtime_error("Error on accepting ssl");
  }

  LOG(INFO) << "connected as server at port " << portNo << " with TLS";

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

  // set cert verification callback for self signed certs
  // comment above has more information
  SSL_CTX_set_cert_verify_callback(
      ctx, callbackToSkipVerificationOfSelfSignedCert_UNSAFE, nullptr);

  if (ctx == nullptr) {
    LOG(INFO) << folly::errnoStr(errno);
    throw std::runtime_error("could not create tls context");
  }

  SSL* ssl = SSL_new(ctx);

  if (ssl == nullptr) {
    LOG(INFO) << folly::errnoStr(errno);
    throw std::runtime_error("could not create tls object");
  }

  const auto sockfd = connectToHost(serverAddress, portNo);

  SSL_set_fd(ssl, sockfd);

  // initiate handshake with server
  const int status = SSL_connect(ssl);
  if (status != 1) {
    LOG(INFO) << folly::errnoStr(errno);
    throw std::runtime_error("could not complete tls handshake");
  }

  XLOGF(INFO, "connected as client to {} at port {}", serverAddress, portNo);

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

  while (connect(sockfd, addrs->ai_addr, addrs->ai_addrlen) < 0) {
    // wait a second and retry
    usleep(1000);
    close(sockfd);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
  }

  freeaddrinfo(addrs);

  return sockfd;
}

int SocketPartyCommunicationAgent::receiveFromClient(int sockfd) {
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

} // namespace fbpcf::engine::communication
