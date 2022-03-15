/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/io/api/CloudFileWriter.h"
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace fbpcf::io {

CloudFileWriter::CloudFileWriter(std::string /* filePath */) {}

int CloudFileWriter::close() {
  return 0;
}
size_t CloudFileWriter::write(std::vector<char>& /* buf */) {
  return 0;
}

CloudFileWriter::~CloudFileWriter() {
  close();
}
} // namespace fbpcf::io
