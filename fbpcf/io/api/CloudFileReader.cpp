/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/io/api/CloudFileReader.h"
#include <cstddef>
#include <string>
#include <vector>
#include "fbpcf/exception/AwsException.h"
#include "folly/Format.h"

namespace fbpcf::io {

int CloudFileReader::close() {
  if (isClosed_) {
    return 0;
  }
  isClosed_ = true;
  return 0;
}

size_t CloudFileReader::read(std::vector<char>& buf) {
  try {
    XLOG(INFO) << "Start reading bytes from S3.\n"
               << "Current position/file length: " << currentPosition_ << "/"
               << fileLength_ << "\n"
               << "The buf size: " << buf.size();
    if (eof()) {
      XLOG(ERR) << "Reached the end of the file.";
      return static_cast<size_t>(-1);
    }
    auto readStr = cloudFileReader_->readBytes(
        filePath_, currentPosition_, currentPosition_ + buf.size());
    auto readStrSize = readStr.size();
    XLOG(INFO) << "Read bytes size: " << readStr.length();
    buf.assign(readStr.begin(), readStr.end());
    currentPosition_ += readStrSize;
    return readStrSize;
  } catch (AwsException& e) {
    // If it fails to get object from S3,
    // we will throw AwsException (e.g. "InvalidRange").
    throw fbpcf::PcfException(
        folly::sformat("Exception from AWS: {}", e.what()));
  } catch (std::exception& e) {
    throw fbpcf::PcfException(folly::sformat("Exception: {}", e.what()));
  }
}
bool CloudFileReader::eof() {
  return currentPosition_ >= fileLength_;
}

CloudFileReader::~CloudFileReader() {
  close();
}

} // namespace fbpcf::io
