/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <algorithm>
#include <cstddef>
#include <vector>

#include "fbpcf/io/api/BufferedWriter.h"

namespace fbpcf::io {

int BufferedWriter::close() {
  flush();
  return baseWriter_->close();
}

size_t BufferedWriter::write(std::vector<char>& buf) {
  size_t written = 0;
  size_t remaining = buf.size();

  while (written < buf.size()) {
    // write what we can in the current chunk
    size_t bytesThatWillFit = buffer_.size() - currentPosition_;
    size_t bytesToWrite = std::min(bytesThatWillFit, remaining);
    std::copy(
        buf.begin() + written,
        buf.begin() + written + bytesToWrite,
        buffer_.begin() + currentPosition_);
    currentPosition_ += bytesToWrite;
    written += bytesToWrite;
    remaining -= bytesToWrite;
    if (remaining == 0) {
      // if we were able to fit everything, break and return right here
      break;
    }

    // flush appropriately sets the currentPosition_
    flush();
  }

  return written;
}

size_t BufferedWriter::writeString(std::string& line) {
  auto vec = std::vector<char>(line.size());
  std::copy(line.begin(), line.end(), vec.begin());

  return write(vec);
}

BufferedWriter::~BufferedWriter() {}

void BufferedWriter::flush() {
  if (currentPosition_ == 0) {
    return;
  }

  std::vector<char> toWrite;
  if (currentPosition_ < buffer_.size()) {
    toWrite = std::vector<char>(currentPosition_);
    std::copy(
        buffer_.begin(), buffer_.begin() + currentPosition_, toWrite.begin());
  } else {
    toWrite = buffer_;
  }
  currentPosition_ = 0;
  if (baseWriter_->write(toWrite) != toWrite.size()) {
    throw std::runtime_error(
        "Failed to flush contents of buffer. Terminating.");
  }
}

} // namespace fbpcf::io
