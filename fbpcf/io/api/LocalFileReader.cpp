/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/io/api/LocalFileReader.h"
#include <cstddef>
#include <vector>

namespace fbpcf::io {

LocalFileReader::LocalFileReader(std::string /* filePath */) {}

int LocalFileReader::close() {
  return 0;
}
size_t LocalFileReader::read(std::vector<char>& /* buf */) {
  return 0;
}

LocalFileReader::~LocalFileReader() {
  close();
}
} // namespace fbpcf::io
