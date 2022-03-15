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

#include "fbpcf/io/api/IReaderCloser.h"

namespace fbpcf::io {

/*
This class is the API for reading data from network
socket. It is constructed with a socket file descriptor.
*/
class SocketReader : public IReaderCloser {
 public:
  /*
   * Creates a SocketReader to read from the given
   * file descriptor.
   */
  SocketReader(FILE* socket);

  /*
   * Creates a SocketReader to read from
   * the provided SSL/TLS connection object.
   */
  SocketReader(SSL* ssl);

  int close() override;
  size_t read(std::vector<char>& buf) override;
  ~SocketReader() override;
};

} // namespace fbpcf::io
