/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include "fbpcf/io/api/IWriterCloser.h"

namespace fbpcf::io {

/*
This class is the API for writing a file to cloud
storage. It can be in any supported cloud provider, but
cannot be a local file.
*/
class CloudFileWriter : public IWriterCloser {
 public:
  int close() override;
  int write(char buf[]) override;
  ~CloudFileWriter() override;
};

} // namespace fbpcf::io
