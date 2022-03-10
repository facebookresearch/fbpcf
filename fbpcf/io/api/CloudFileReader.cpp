/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/io/api/CloudFileReader.h"
#include <cstddef>
#include <string>

namespace fbpcf::io {

CloudFileReader::CloudFileReader(std::string /* filePath */) {}

int CloudFileReader::close() {
  return 0;
}
int CloudFileReader::read(char buf[], size_t nBytes) {
  return 0;
}

CloudFileReader::~CloudFileReader() {
  close();
}

} // namespace fbpcf::io
