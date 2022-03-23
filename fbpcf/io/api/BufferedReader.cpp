/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cstddef>
#include <exception>
#include <stdexcept>
#include <vector>

#include "fbpcf/io/api/BufferedReader.h"

namespace fbpcf::io {

int BufferedReader::close() {
  return baseReader_.close();
}

size_t BufferedReader::read(std::vector<char>& buf) {
  if (eof()) {
    throw std::runtime_error("There is no more data in this file.");
  }

  size_t filledUp = 0;
  size_t remaining = buf.size();
  while (filledUp < buf.size() && !eof()) {
    if ((lastPosition_ - currentPosition_) >= remaining) {
      // we already have enough data loaded
      std::copy(
          buffer_.begin() + currentPosition_,
          buffer_.begin() + currentPosition_ + remaining,
          buf.begin() + filledUp);
      currentPosition_ += remaining;
      filledUp += remaining;
      break;
    }

    // need to load more data

    // first, copy what we currently have
    std::copy(
        buffer_.begin() + currentPosition_,
        buffer_.begin() + lastPosition_,
        buf.begin() + filledUp);
    filledUp += (lastPosition_ - currentPosition_);
    remaining = buf.size() - filledUp;
    currentPosition_ = lastPosition_;

    if (baseReader_.eof()) {
      // that's all the data that exists in the file
      return filledUp;
    }

    // load next chunk and the repeat
    // loadNextChunk appropriately sets
    // currentPosition, lastPosition_, and eof_
    loadNextChunk();
  }

  return filledUp;
}

bool BufferedReader::eof() {
  // The base file is exhausted and the buffer is exhausted
  return baseReader_.eof() && (currentPosition_ == lastPosition_);
}

BufferedReader::~BufferedReader() {
  close();
}

std::string BufferedReader::readLine() {
  if (eof()) {
    throw std::runtime_error("There are no more lines in this file.");
  }

  if (currentPosition_ == lastPosition_) {
    loadNextChunk();
  }

  auto c = buffer_.at(currentPosition_);
  std::string output = "";
  while (c != '\n') {
    output += c;
    currentPosition_++;

    if (currentPosition_ == lastPosition_) {
      // we've run out of data, try to get the next chunk
      if (baseReader_.eof()) {
        // no more data in the file, return what we have
        break;
      }

      loadNextChunk();
    }

    c = buffer_.at(currentPosition_);
  }
  currentPosition_++;

  return output;
}

void BufferedReader::loadNextChunk() {
  auto bytesRead = baseReader_.read(buffer_);

  lastPosition_ = bytesRead;
  currentPosition_ = 0;
}

} // namespace fbpcf::io
