/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include <google/cloud/storage/client.h>

#include "fbpcf/io/IFileManager.h"

namespace gcs = ::google::cloud::storage;

namespace fbpcf {
template <class ClientCls>
class GCSFileManager : public IFileManager {
 public:
  explicit GCSFileManager(std::shared_ptr<ClientCls> client)
      : GCSClient_{std::move(client)} {}

  std::unique_ptr<IInputStream> getInputStream(
      const std::string& fileName) override;

  std::string readBytes(
      const std::string& fileName,
      std::size_t start,
      std::size_t end) override;
  std::string read(const std::string& fileName) override;

  void write(const std::string& fileName, const std::string& data) override;

 private:
  std::shared_ptr<ClientCls> GCSClient_;
};
} // namespace fbpcf
