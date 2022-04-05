/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>

namespace fbpcf::cloudio {

class IFileReader {
 public:
  virtual ~IFileReader() {}
  virtual std::string readBytes(
      const std::string& fileName,
      std::size_t start,
      std::size_t end) = 0;
  virtual size_t getFileContentLength(const std::string& fileName) = 0;
};

} // namespace fbpcf::cloudio
