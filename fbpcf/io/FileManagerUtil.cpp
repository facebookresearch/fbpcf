/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "FileManagerUtil.h"

#include "LocalFileManager.h"
#include "S3FileManager.h"
#include "fbpcf/aws/S3Util.h"

namespace fbpcf::io {
std::unique_ptr<IInputStream> getInputStream(const std::string& fileName) {
  auto manager = getFileManager(fileName);
  return manager->getInputStream(fileName);
}

std::string read(const std::string& fileName) {
  auto manager = getFileManager(fileName);
  return manager->read(fileName);
}

void write(const std::string& fileName, const std::string& data) {
  auto manager = getFileManager(fileName);
  return manager->write(fileName, data);
}

FileType getFileType(const std::string& fileName) {
  // S3 file format: https://bucket-name.s3.Region.amazonaws.com/key-name
  return fileName.find("https://", 0) == 0 ? FileType::S3 : FileType::Local;
}

std::unique_ptr<fbpcf::IFileManager> getFileManager(
    const std::string& fileName) {
  auto type = getFileType(fileName);
  if (type == FileType ::S3) {
    const auto& ref = fbpcf::aws::uriToObjectReference(fileName);
    // Other options have to be set via environment variables
    return std::make_unique<S3FileManager>(fbpcf::aws::createS3Client(
        fbpcf::aws::S3ClientOption{.region = ref.region}));
  } else {
    return std::make_unique<LocalFileManager>();
  }
}
} // namespace fbpcf::io
