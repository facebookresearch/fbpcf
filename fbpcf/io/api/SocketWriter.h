/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <openssl/ssl.h>
#include <cstddef>
#include <vector>

#include "fbpcf/io/api/IWriterCloser.h"

namespace fbpcf::io {

/*
This class is the API for writing data to a network
socket. It is constructed with a socket file descriptor.
*/
class SocketWriter : public IWriterCloser {
 public:
  /*
   * Creates a SocketWriter to write to the given
   * file descriptor.
   */
  SocketWriter(FILE* socket);

  /*
   * Creates a SocketReader to write to
   * the provided SSL/TLS connection object.
   */
  SocketWriter(SSL* ssl);

  int close() override;
  size_t write(std::vector<char>& buf) override;
  ~SocketWriter() override;
};

} // namespace fbpcf::io
