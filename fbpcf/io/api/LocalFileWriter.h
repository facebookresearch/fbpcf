/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include "fbpcf/io/api/ICloser.h"
#include "fbpcf/io/api/IWriter.h"

namespace fbpcf::io {

/*
This class is the API for writing a file to local
storage. It must be on disk and cannot be a file in
cloud storage.
*/
class LocalFileWriter : public IWriter, public ICloser {
 public:
  int close() override;
  int write(char buf[]) override;
  ~LocalFileWriter() override;
};

} // namespace fbpcf::io
