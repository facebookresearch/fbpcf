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

namespace fbpcf::io {

CloudFileReader::CloudFileReader(std::string /* filePath */) {}

int CloudFileReader::close() {
  return 0;
}
size_t CloudFileReader::read(std::vector<char>& /* buf */) {
  return 0;
}
bool CloudFileReader::eof() {
  return false;
}

CloudFileReader::~CloudFileReader() {
  close();
}

} // namespace fbpcf::io
