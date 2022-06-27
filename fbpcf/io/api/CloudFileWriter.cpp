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

int CloudFileWriter::close() {
  if (isClosed_) {
    return 0;
  }
  isClosed_ = true;
  return cloudFileUploader_->complete();
}

size_t CloudFileWriter::write(std::vector<char>& buf) {
  return cloudFileUploader_->upload(buf);
}

CloudFileWriter::~CloudFileWriter() {
  close();
}
} // namespace fbpcf::io
