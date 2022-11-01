/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include <aws/s3/S3Client.h> // @manual
#include "fbpcf/io/cloud_util/IFileReader.h"

namespace fbpcf::cloudio {

class S3FileReader : public IFileReader {
 public:
  explicit S3FileReader(std::unique_ptr<Aws::S3::S3Client> client)
      : s3Client_{std::move(client)} {}

  std::string readBytes(
      const std::string& filePath,
      std::size_t start,
      std::size_t end) override;

  size_t getFileContentLength(const std::string& filePath) override;

 private:
  std::unique_ptr<Aws::S3::S3Client> s3Client_;
};

} // namespace fbpcf::cloudio
