/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <cstddef>
#include <string>
#include <vector>
#include "fbpcf/io/api/IReaderCloser.h"

namespace fbpcf::io {

/*
This class is the API for reading a file from cloud
storage. It can be in any supported cloud provider, but
cannot be a local file.
*/
class CloudFileReader : public IReaderCloser {
 public:
  explicit CloudFileReader(std::string filePath);

  int close() override;
  size_t read(std::vector<char>& buf) override;
  ~CloudFileReader() override;
};

} // namespace fbpcf::io
