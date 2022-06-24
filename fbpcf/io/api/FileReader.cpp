/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/io/api/FileReader.h"
#include <cstddef>
#include <memory>
#include <string>
#include <vector>
#include "fbpcf/io/api/CloudFileReader.h"
#include "fbpcf/io/api/LocalFileReader.h"

#include "IOUtils.h"

namespace fbpcf::io {
FileReader::FileReader(std::string filePath) {
  if (IOUtils::isCloudFile(filePath)) {
    childReader_ = std::make_unique<CloudFileReader>(filePath);
  } else {
    childReader_ = std::make_unique<LocalFileReader>(filePath);
  }
}

FileReader::~FileReader() {
  close();
}

size_t FileReader::read(std::vector<char>& buf) {
  return childReader_->read(buf);
}

bool FileReader::eof() {
  return childReader_->eof();
}

int FileReader::close() {
  return childReader_->close();
}
} // namespace fbpcf::io
