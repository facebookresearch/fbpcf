/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include "fbpcf/io/api/ICloser.h"
#include "fbpcf/io/api/IWriter.h"

namespace fbpcf::io {

/*
This class is the API for writing data to a network
socket. It is constructed with a socket file descriptor.
*/
class SocketWriter : public IWriter, public ICloser {
 public:
  int close() override;
  int write(char buf[]) override;
  ~SocketWriter() override;
};

} // namespace fbpcf::io
