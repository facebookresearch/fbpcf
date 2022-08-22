/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/io/api/LocalFileWriter.h"
#include <folly/logging/xlog.h>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

namespace fbpcf::io {

LocalFileWriter::LocalFileWriter(std::string filePathStr) {
  isClosed_ = false;
  std::filesystem::path filePath{filePathStr};
  if (filePath.has_parent_path()) {
    std::filesystem::create_directories(filePath.parent_path());
  }
  outputStream_ = std::make_unique<std::ofstream>(filePathStr);
  if (outputStream_->fail()) {
    XLOGF(ERR, "Error when opening file: {}", strerror(errno));
    throw std::runtime_error("Couldn't open local file.");
  }

  filepath_ = filePathStr;
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
    XLOGF(ERR, "Error when writing to file: {}", strerror(errno));
    throw std::runtime_error(
        "Internal error when writing to local file. Stream integrity may have been affected.");
  }
  return buf.size();
}

LocalFileWriter::~LocalFileWriter() {
  close();
}
} // namespace fbpcf::io
