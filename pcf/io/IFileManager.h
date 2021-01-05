/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/

#pragma once

#include <istream>
#include <string>
#include <memory>

#include "IInputStream.h"

namespace pcf {
class IFileManager {
 public:
  virtual ~IFileManager() {}

  virtual std::unique_ptr<IInputStream> getInputStream(const std::string& fileName) = 0;

  virtual std::string read(const std::string& fileName) = 0;

  virtual void write(const std::string& fileName, const std::string& data) = 0;
};
} // namespace pcf
