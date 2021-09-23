/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include "IFileManager.h"

namespace fbpcf {
class LocalFileManager : public IFileManager {
 public:
  std::unique_ptr<IInputStream> getInputStream(
      const std::string& fileName) override;

  std::string read(const std::string& fileName) override;

  void write(const std::string& fileName, const std::string& data) override;

  std::string
  readBytes(const std::string& fileName, std::size_t start, std::size_t end);
};
} // namespace fbpcf
