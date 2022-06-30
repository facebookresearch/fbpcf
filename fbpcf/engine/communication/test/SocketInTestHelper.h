/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdexcept>
#include <string>

#include <fcntl.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

namespace fbpcf::engine::communication {

class SocketInTestHelper {
 public:
  static int findNextOpenPort(int portNo) {
    while (!isPortOpen(portNo)) {
      portNo++;
      if (portNo > 65535) {
        throw std::runtime_error("Can't find a open port!");
      }
    }
    return portNo;
  }

 private:
  static bool isPortOpen(int port) {
    auto sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
      throw std::runtime_error("error opening socket");
    }
    int enable = 1;

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) <
        0) {
      throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");
    }

    struct sockaddr_in servAddr;

    memset((char*)&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;
    servAddr.sin_port = htons(port);

    // if binding to the original port number fails, try to grab a free port
    // instead
    if (::bind(
            sockfd, (struct sockaddr*)&servAddr, sizeof(struct sockaddr_in)) <
        0) {
      return false;
    } else {
      close(sockfd);
      return true;
    }
  }
};

} // namespace fbpcf::engine::communication
