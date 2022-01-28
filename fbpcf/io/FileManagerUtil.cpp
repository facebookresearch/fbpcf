/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "FileManagerUtil.h"

#include <fbpcf/gcp/GCSUtil.h>
#include "LocalFileManager.h"
#include "S3FileManager.h"
#include "fbpcf/aws/S3Util.h"
#include "fbpcf/io/GCSFileManager.h"

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
  if (fileName.find("https://", 0) != 0) {
    return FileType::Local;
  } else if (fileName.find("https://storage.cloud.google.com") == 0) {
    return FileType::GCS;
  } else {
    return FileType::S3;
  }
}

std::unique_ptr<fbpcf::IFileManager> getFileManager(
    const std::string& fileName) {
  auto type = getFileType(fileName);
  if (type == FileType ::S3) {
    const auto& ref = fbpcf::aws::uriToObjectReference(fileName);
    // Other options have to be set via environment variables
    return std::make_unique<S3FileManager>(fbpcf::aws::createS3Client(
        fbpcf::aws::S3ClientOption{.region = ref.region}));
  } else if (type == FileType ::GCS) {
    return std::make_unique<GCSFileManager<google::cloud::storage::Client>>(
        fbpcf::gcp::createGCSClient());
  } else {
    return std::make_unique<LocalFileManager>();
  }
}
} // namespace fbpcf::io
