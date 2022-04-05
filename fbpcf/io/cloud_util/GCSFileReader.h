/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <google/cloud/storage/client.h>

#include <memory>
#include <sstream>
#include "fbpcf/io/cloud_util/IFileReader.h"

namespace fbpcf::cloudio {
template <class ClientCls>
class GCSFileReader : public IFileReader {
 public:
  explicit GCSFileReader(std::shared_ptr<ClientCls> client)
      : GCSClient_{std::move(client)} {}

  std::string readBytes(
      const std::string& filePath,
      std::size_t start,
      std::size_t end) override;

  size_t getFileContentLength(const std::string& filePath) override;

 private:
  std::shared_ptr<ClientCls> GCSClient_;
};

} // namespace fbpcf::cloudio
