/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include <aws/s3/S3Client.h>
#include <sstream>

#include "IFileManager.h"
#include "IInputStream.h"

namespace fbpcf {
class S3FileManager : public IFileManager {
 public:
  explicit S3FileManager(std::unique_ptr<Aws::S3::S3Client> client)
      : s3Client_{std::move(client)} {}

  std::unique_ptr<IInputStream> getInputStream(
      const std::string& fileName) override;

  std::string
  readBytes(const std::string& fileName, std::size_t start, std::size_t end);
  std::string read(const std::string& fileName) override;

  void write(const std::string& fileName, const std::string& data) override;

  void copy(const std::string& sourceFile, const std::string& destination)
      override;

  void writeDataStream(
      const std::string& fileName,
      std::shared_ptr<std::basic_iostream<char, std::char_traits<char>>>
          dataStream);

 private:
  std::unique_ptr<Aws::S3::S3Client> s3Client_;
};
} // namespace fbpcf
