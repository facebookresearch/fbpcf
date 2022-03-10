/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/io/api/LocalFileReader.h"
#include <cstddef>

namespace fbpcf::io {

LocalFileReader::LocalFileReader(std::string /* filePath */) {}

int LocalFileReader::close() {
  return 0;
}
int LocalFileReader::read(char buf[], size_t nBytes) {
  return 0;
}

LocalFileReader::~LocalFileReader() {
  close();
}
} // namespace fbpcf::io
