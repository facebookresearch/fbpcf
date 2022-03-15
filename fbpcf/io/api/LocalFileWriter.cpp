/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/io/api/LocalFileWriter.h"
#include <cstddef>
#include <string>
#include <vector>

namespace fbpcf::io {
LocalFileWriter::LocalFileWriter(std::string /* filePath */) {}

int LocalFileWriter::close() {
  return 0;
}
size_t LocalFileWriter::write(std::vector<char>& /* buf */) {
  return 0;
}

LocalFileWriter::~LocalFileWriter() {
  close();
}
} // namespace fbpcf::io
