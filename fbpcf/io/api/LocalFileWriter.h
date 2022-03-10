/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <cstddef>
#include <string>
#include "fbpcf/io/api/IWriterCloser.h"

namespace fbpcf::io {

/*
This class is the API for writing a file to local
storage. It must be on disk and cannot be a file in
cloud storage.
*/
class LocalFileWriter : public IWriterCloser {
 public:
  explicit LocalFileWriter(std::string filePath);

  int close() override;
  int write(char buf[], size_t nBytes) override;
  ~LocalFileWriter() override;
};

} // namespace fbpcf::io
