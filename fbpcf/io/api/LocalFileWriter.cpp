/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/io/api/LocalFileWriter.h"
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace fbpcf::io {

LocalFileWriter::LocalFileWriter(std::string filePath) {
  isClosed_ = false;
  outputStream_ = std::make_unique<std::ofstream>(filePath);
}

int LocalFileWriter::close() {
  if (isClosed_) {
    return 0;
  }
  isClosed_ = true;
  outputStream_->close();

  return outputStream_->fail() ? -1 : 0;
}

size_t LocalFileWriter::write(std::vector<char>& buf) {
  outputStream_->write(buf.data(), buf.size());

  if (outputStream_->fail()) {
    throw std::runtime_error(
        "Internal error when writing to local file. Stream integrity may have been affected.");
  }

  return buf.size();
}

LocalFileWriter::~LocalFileWriter() {}
} // namespace fbpcf::io
