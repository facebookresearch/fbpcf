/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/io/api/LocalFileWriter.h"
#include <string>

namespace fbpcf::io {
LocalFileWriter::LocalFileWriter(std::string /* filePath */) {}

int LocalFileWriter::close() {
  return 0;
}
int LocalFileWriter::write(char buf[]) {
  return 0;
}

LocalFileWriter::~LocalFileWriter() {
  close();
}
} // namespace fbpcf::io
