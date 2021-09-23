/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <istream>
#include <memory>
#include <string>

#include "IInputStream.h"

namespace fbpcf {
class IFileManager {
 public:
  virtual ~IFileManager() {}

  virtual std::unique_ptr<IInputStream> getInputStream(
      const std::string& fileName) = 0;

  virtual std::string read(const std::string& fileName) = 0;

  virtual void write(const std::string& fileName, const std::string& data) = 0;

  virtual std::string readBytes(
      const std::string& fileName,
      std::size_t start,
      std::size_t end) = 0;
};
} // namespace fbpcf
