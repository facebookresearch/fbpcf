/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/io/api/LocalFileReader.h"
#include <folly/logging/xlog.h>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <vector>

namespace fbpcf::io {

LocalFileReader::LocalFileReader(std::string filePath) {
  inputStream_ = std::make_unique<std::ifstream>(filePath);
  if (inputStream_->fail()) {
    XLOGF(ERR, "Error when opening file: {}", strerror(errno));
    throw std::runtime_error("Couldn't open local file.");
  }
  isClosed_ = false;

  filepath_ = filePath;
}

int LocalFileReader::close() {
  if (isClosed_) {
    return 0;
  }
  isClosed_ = true;
  inputStream_->close();

  return inputStream_->fail() ? -1 : 0;
}

size_t LocalFileReader::read(std::vector<char>& buf) {
  if (inputStream_->fail()) {
    throw std::runtime_error("Input stream is in a failed state.");
  }

  inputStream_->read(buf.data(), buf.size());

  if (inputStream_->fail() && !inputStream_->eof()) {
    throw std::runtime_error(
        "Internal error when reading from local file. Stream integrity may have been affected.");
  }

  return inputStream_->gcount();
}

bool LocalFileReader::eof() {
  // eof() returns true only if we have already attempted to read past the end
  // of the file
  return inputStream_->eof() || inputStream_->peek() == EOF;
}

LocalFileReader::~LocalFileReader() {
  close();
}
} // namespace fbpcf::io
