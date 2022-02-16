/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <vector>
#include "fbpcf/io/api/ICloser.h"
#include "fbpcf/io/api/IReader.h"

namespace fbpcf::io {

/*
This class is the API for reading a file from local
storage. It must be on disk and cannot be a file in
cloud storage.
*/
class LocalFileReader : public IReader, public ICloser {
 public:
  int close() override;
  int read(char buf[]) override;
  ~LocalFileReader() override;
};

} // namespace fbpcf::io
