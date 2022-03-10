/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include "fbpcf/io/api/IReaderCloser.h"

namespace fbpcf::io {

/*
This class is the API for reading a file from any
storage, local or cloud. It can be in any supported
cloud provider or a file on disk. Internally, this
class will create a LocalFileReader or CloudFileReader
depending on what file path is provided.
*/
class FileReader : public IReaderCloser {
 public:
  int close() override;
  int read(char buf[]) override;
  ~FileReader() override;
};

} // namespace fbpcf::io
