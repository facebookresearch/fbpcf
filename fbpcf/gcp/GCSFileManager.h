/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include <google/cloud/storage/client.h>

#include "IFileManager.h"

namespace gcs = ::google::cloud::storage;

namespace fbpcf {
class GCSFileManager : public IFileManager {
 public:
  explicit GCSFileManager(std::unique_ptr<gcs::Client> client)
      : GCSClient_{std::move(client)} {}

  std::unique_ptr<IInputStream> getInputStream(
      const std::string& fileName) override;

  std::string
  readBytes(const std::string& fileName, std::size_t start, std::size_t end);
  std::string read(const std::string& fileName) override;

  void write(const std::string& fileName, const std::string& data) override;

 private:
  std::unique_ptr<gcs::Client> GCSClient_;
};
} // namespace fbpcf
