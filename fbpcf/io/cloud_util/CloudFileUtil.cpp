/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/io/cloud_util/CloudFileUtil.h"
#include <re2/re2.h>
#include "fbpcf/aws/S3Util.h"
#include "fbpcf/exception/PcfException.h"
#include "fbpcf/io/cloud_util/S3Client.h"
#include "fbpcf/io/cloud_util/S3FileReader.h"
#include "fbpcf/io/cloud_util/S3FileUploader.h"

namespace fbpcf::cloudio {

CloudFileType getCloudFileType(const std::string& filePath) {
  // S3 file format:
  // 1. https://bucket-name.s3.Region.amazonaws.com/key-name
  // 2. https://bucket-name.s3-Region.amazonaws.com/key-name
  // 3. s3://bucket-name/key-name
  // GCS file format:
  // 1. https://storage.cloud.google.com/bucket-name/key-name
  // 2. gs://bucket-name/key-name
  static const re2::RE2 s3Regex1(
      "https://[a-z0-9.-]+.s3.[a-z0-9-]+.amazonaws.com/.+");
  static const re2::RE2 s3Regex2(
      "https://[a-z0-9.-]+.s3-[a-z0-9-]+.amazonaws.com/.+");

  bool isS3File = re2::RE2::FullMatch(filePath, s3Regex1) ||
      re2::RE2::FullMatch(filePath, s3Regex2) || filePath.find("s3://", 0) == 0;
  if (isS3File) {
    return CloudFileType::S3;
  }

  static const re2::RE2 gcsRegex("https://storage.cloud.google.com/.*");
  bool isGCSFile =
      re2::RE2::FullMatch(filePath, gcsRegex) || filePath.find("gs://", 0) == 0;
  if (isGCSFile) {
    return CloudFileType::GCS;
  }
  return CloudFileType::UNKNOWN;
}

std::unique_ptr<IFileReader> getCloudFileReader(const std::string& filePath) {
  auto fileType = getCloudFileType(filePath);
  if (fileType == CloudFileType::S3) {
    const auto& ref = fbpcf::aws::uriToObjectReference(filePath);
    return std::make_unique<S3FileReader>(
        fbpcf::cloudio::S3Client::getInstance(
            fbpcf::aws::S3ClientOption{.region = ref.region})
            .getS3Client());
  } else {
    return nullptr;
  }
}

std::unique_ptr<IFileUploader> getCloudFileUploader(
    const std::string& filePath) {
  auto fileType = getCloudFileType(filePath);
  if (fileType == CloudFileType::S3) {
    const auto& ref = fbpcf::aws::uriToObjectReference(filePath);
    return std::make_unique<S3FileUploader>(
        fbpcf::cloudio::S3Client::getInstance(
            fbpcf::aws::S3ClientOption{.region = ref.region})
            .getS3Client(),
        filePath);
  } else {
    throw fbpcf::PcfException("Not supported yet.");
  }
}

} // namespace fbpcf::cloudio
